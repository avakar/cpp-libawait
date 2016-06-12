#include <assert.h>

template <typename F>
aw::task<void> aw::loop(F && f)
{
	struct cmd
		: private detail::task_completion<void>
	{
		typedef void value_type;

		cmd(task<void> && t, F && f)
			: m_task(std::move(t)), m_f(std::move(f)), m_sink(nullptr)
		{
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			if (!detail::start_command(m_task, sch, *this))
				return m_task;

			m_sink = &sink;
			return nullptr;
		}

	private:
		void on_completion(detail::scheduler & sch, task<void> && t) override
		{
			m_task = std::move(t);
			while (!detail::start_command(m_task, sch, *this))
			{
				if (detail::has_exception(m_task))
				{
					m_sink->on_completion(sch, std::move(m_task));
					return;
				}

				m_task = m_f();
				if (m_task.empty())
				{
					m_sink->on_completion(sch, aw::value());
					return;
				}
			}
		}
		
		task<void> m_task;
		F m_f;
		detail::task_completion<void> * m_sink;
	};

	for (;;)
	{
		task<void> t = f();
		if (t.empty())
			return t;
		if (detail::has_command(t))
			return detail::make_command<cmd>(std::move(t), std::move(f));
		if (detail::has_exception(t))
			return t;
	}
}

namespace aw {
namespace detail {

template <typename T>
struct invoke_loop_update
{
	template <typename Ctx, typename Update>
	static void invoke(Ctx & ctx, Update & update, result<T> & r)
	{
		update(ctx, std::move(r.value()));
	}
};

template <>
struct invoke_loop_update<void>
{
	template <typename Ctx, typename Update>
	static void invoke(Ctx & ctx, Update & update, result<void> & r)
	{
		(void)r;
		update(ctx);
	}
};

}
}

template <typename Ctx, typename StartF, typename UpdateF>
aw::task<void> aw::loop(Ctx c, StartF && start, UpdateF && update)
{
	typedef typename detail::task_traits<decltype(start(c))>::value_type T;

	struct cmd
		: private detail::task_completion<T>
	{
		typedef void value_type;

		cmd(task<T> & task, Ctx && ctx, StartF && start, UpdateF && update)
			: m_task(std::move(task)), m_ctx(std::move(ctx)), m_start(std::move(start)), m_update(std::move(update)), m_sink(nullptr)
		{
		}

		result<void> dismiss()
		{
			for (;;)
			{
				result<T> r = detail::dismiss_task(m_task);
				if (r.has_exception())
					return r.exception();
				detail::invoke_loop_update<T>::invoke(m_ctx, m_update, r);
				m_task = m_start(m_ctx);
				if (m_task.empty())
					return aw::value();
			}
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			for (;;)
			{
				if (detail::start_command(m_task, sch, *this))
				{
					m_sink = &sink;
					return nullptr;
				}

				result<T> & r = detail::dismiss_task(m_task);
				if (r.has_exception())
					return r.exception();
				detail::invoke_loop_update<T>::invoke(m_ctx, m_update, r);
				m_task = m_start(m_ctx);
				if (m_task.empty())
					return aw::value();
			}
		}

	private:
		void on_completion(detail::scheduler & sch, task<T> && t) override
		{
			detail::mark_complete(m_task);
			m_task = std::move(t);
			task<void> u = this->start(sch, *m_sink);
			if (!u.empty())
				m_sink->on_completion(sch, std::move(u));
		}

		task<T> m_task;
		Ctx m_ctx;
		StartF m_start;
		UpdateF m_update;
		detail::task_completion<void> * m_sink;
	};

	for (;;)
	{
		task<T> task = start(c);
		if (task.empty())
			return aw::value();
		if (detail::has_command(task))
			return detail::make_command<cmd>(task, std::move(c), std::move(start), std::move(update));

		result<T> & r = detail::dismiss_task(task);
		if (r.has_exception())
			return r.exception();

		detail::invoke_loop_update<T>::invoke(c, update, r);
	}
}
