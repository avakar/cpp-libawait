#include "command.h"
#include "task_traits.h"

template <typename T, typename F>
auto aw::detail::then_traits<T, F>::invoke(result<T> && t, F && f) noexcept
	-> return_type
{
	assert(t.has_value());
	return detail::invoke(std::move(f), std::move(t.value()));
}

template <typename F>
auto aw::detail::then_traits<void, F>::invoke(result<void> && t, F && f) noexcept
	-> return_type
{
	(void)t;

	assert(t.has_value());
	return detail::invoke(std::move(f));
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
		return detail::invoke(std::move(f), this->dismiss());

	struct impl
		: private detail::task_completion<T>
	{
		typedef U value_type;

		impl(task<T> && t, F && f)
			: m_cmd(detail::fetch_command(t)), m_f(std::move(f))
		{
		}

		result<U> dismiss(cancel_info ci)
		{
			task<U> r = detail::invoke(std::move(m_f), m_cmd.dismiss(ci));
			return r.dismiss(ci);
		}

		task<U> start(detail::scheduler & sch, detail::task_completion<U> & sink)
		{
			m_sink = &sink;
			return start_command(m_cmd, sch, *this, [this](result<T> && r) {
				return detail::invoke(std::move(m_f), std::move(r));
			});
		}

		result<U> cancel(detail::scheduler & sch)
		{
			result<T> r = m_cmd.cancel(sch);
			return detail::invoke(std::move(m_f), std::move(r)).dismiss(detail::get_cancel_info(sch));
		}

	private:
		void on_completion(detail::scheduler & sch, task<T> && t) override
		{
			m_cmd.complete(t);

			task<U> u;
			if (m_cmd)
			{
				u = start_command(m_cmd, sch, *this, [this](result<T> && r) {
					return detail::invoke(std::move(m_f), std::move(r));
				});
			}
			else
			{
				u = detail::invoke(std::move(m_f), t.dismiss());
			}

			if (!u.empty())
				m_sink->on_completion(sch, std::move(u));
		}

		detail::command_ptr<T> m_cmd;
		F m_f;
		detail::task_completion<U> * m_sink;
	};

	return detail::make_command<impl>(std::move(*this), std::move(f));
}
