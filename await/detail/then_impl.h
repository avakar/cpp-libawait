#include "command.h"
#include "task_traits.h"

namespace aw {
namespace detail {

template <typename T, typename F>
auto aw::detail::continue_with(result<T> && r, F && f) -> decltype(std::declval<F>()(std::declval<result<T>>()));

}
}

template <typename T, typename F>
typename aw::detail::then_traits<T, F>::task_type aw::detail::then_traits<T, F>::invoke(result<T> && t, F && f)
{
	return f(std::move(t.value()));
}

template <typename F>
typename aw::detail::then_traits<void, F>::task_type aw::detail::then_traits<void, F>::invoke(result<void> && t, F && f)
{
	return f();
}

template <typename T>
template <typename F>
typename aw::detail::then_traits<T, F>::task_type aw::task<T>::then(F && f)
{
	assert(m_vtable);

	struct cont
	{
		explicit cont(F && f)
			: m_f(std::move(f))
		{
		}

		typename aw::detail::then_traits<T, F>::task_type operator()(result<T> && r)
		{
			if (r.has_exception())
				return std::move(r.exception());

			try
			{
				return detail::then_traits<T, F>::invoke(std::move(r), std::move(m_f));
			}
			catch (...)
			{
				return std::current_exception();
			}
		}

		F m_f;
	};

	return this->continue_with(cont(std::move(f)));
}

template <typename T, typename F>
auto aw::detail::continue_with(result<T> && r, F && f) -> decltype(std::declval<F>()(std::declval<result<T>>()))
{
	try
	{
		return f(std::move(r));
	}
	catch (...)
	{
		return std::current_exception();
	}
}

template <typename T>
template <typename F>
decltype(std::declval<F>()(std::declval<aw::result<T>>())) aw::task<T>::continue_with(F && f)
{
	assert(m_vtable != nullptr);
	typedef decltype(std::declval<F>()(std::declval<result<T>>())) task_U;
	typedef typename detail::task_traits<task_U>::value_type U;

	if (m_vtable->get_result)
	{
		result<T> r = m_vtable->get_result(&m_storage);
		m_vtable = nullptr;
		return detail::continue_with(std::move(r), std::move(f));
	}

	struct impl
		: private detail::task_completion<T>
	{
		typedef U value_type;

		impl(task<T> && t, F && f)
			: m_task(std::move(t)), m_f(std::move(f))
		{
		}

		task<U> start(detail::scheduler & sch, detail::task_completion<U> & sink)
		{
			void * storage = detail::task_access::storage(m_task);
			detail::task_vtable<T> const * vtable = detail::task_access::get_vtable(m_task);

			while (vtable->start != nullptr)
			{
				task<T> t = vtable->start(storage, sch, *this);
				if (t.empty())
				{
					m_sink = &sink;
					return nullptr;
				}

				m_task = std::move(t);
				vtable = detail::task_access::get_vtable(m_task);
			}

			result<T> r = vtable->get_result(storage);
			detail::task_access::set_vtable(m_task, nullptr);
			return detail::continue_with(std::move(r), std::move(m_f));
		}

	private:
		void on_completion(detail::scheduler & sch, task<T> && t) override
		{
			m_task = std::move(t);

			task<U> u = this->start(sch, *m_sink);
			if (!u.empty())
				m_sink->on_completion(sch, std::move(u));
		}

		task<T> m_task;
		F m_f;
		detail::task_completion<U> * m_sink;
	};

	return detail::make_command<impl>(std::move(*this), std::move(f));
}
