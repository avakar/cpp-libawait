#include "command.h"
#include "task_traits.h"

template <typename T, typename F>
auto aw::detail::then_traits<T, F>::invoke(result<T> && t, F && f) noexcept
	-> return_type
{
	return invoke_and_taskify(std::move(f), std::move(t.value()));
}

template <typename F>
auto aw::detail::then_traits<void, F>::invoke(result<void> && t, F && f) noexcept
	-> return_type
{
	return invoke_and_taskify(std::move(f));
}

template <typename T>
template <typename F>
auto aw::task<T>::then(F && f)
	-> typename aw::detail::then_traits<T, F>::return_type
{
	assert(!this->empty());

	struct cont
	{
		explicit cont(F && f)
			: m_f(std::move(f))
		{
		}

		typename aw::detail::then_traits<T, F>::return_type operator()(result<T> && r)
		{
			if (r.has_exception())
				return r.exception();
			return detail::then_traits<T, F>::invoke(std::move(r), std::move(m_f));
		}

		F m_f;
	};

	return this->continue_with(cont(std::move(f)));
}

template <typename T>
template <typename F>
auto aw::task<T>::continue_with(F && f) -> typename detail::continue_with_traits<T, F>::return_type
{
	typedef typename detail::invoke_and_taskify_traits<F, result<T>>::value_type U;

	assert(m_kind != detail::task_kind::empty);
	if (m_kind != detail::task_kind::command)
		return detail::invoke_and_taskify(std::move(f), detail::dismiss_task(*this));

	struct impl
		: private detail::task_completion<T>
	{
		typedef U value_type;

		impl(task<T> && t, F && f)
			: m_task(std::move(t)), m_f(std::move(f))
		{
		}

		result<U> dismiss()
		{
			task<U> r = detail::invoke_and_taskify(std::move(m_f), detail::dismiss_task(m_task));
			return detail::dismiss_task(r);
		}

		task<U> start(detail::scheduler & sch, detail::task_completion<U> & sink)
		{
			if (detail::start_command(m_task, sch, *this))
			{
				m_sink = &sink;
				return nullptr;
			}

			return detail::invoke_and_taskify(std::move(m_f), detail::dismiss_task(m_task));
		}

	private:
		void on_completion(detail::scheduler & sch, task<T> && t) override
		{
			detail::mark_complete(m_task);
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
