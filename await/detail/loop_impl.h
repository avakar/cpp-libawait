#include <assert.h>

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

template <typename T>
struct schroedinger
{
	template <typename... P>
	void construct(P &&... p)
	{
		new(&m_buf) T(std::forward<P>(p)...);
	}

	void destroy()
	{
		this->get()->~T();
	}

	T * get()
	{
		return reinterpret_cast<T *>(&m_buf);
	}

	T & operator*()
	{
		return *this->get();
	}

	T * operator->()
	{
		return this->get();
	}

private:
	typename std::aligned_union<0, T>::type m_buf;
};

}
}

template <typename Ctx, typename StartF, typename UpdateF>
aw::task<void> aw::loop(Ctx c, StartF && start, UpdateF && update)
{
	typedef typename detail::task_traits<decltype(start(c))>::value_type T;

	struct coroutine
	{
	public:
		coroutine()
			: state(init)
		{
		}

		coroutine(coroutine && o)
			: state(o.state)
		{
			o.state = init;

			switch (state)
			{
			case awaiting_cmd:
				cmd = std::move(o.cmd);
				break;
			case done:
				res.construct(std::move(*o.res));
				o.res.destroy();
				break;
			}
		}

		~coroutine()
		{
			assert(state == init || state == done);
			if (state == done)
				res.destroy();
		}

		void process(Ctx & c, StartF & start, UpdateF & update)
		{
			assert(state != done);

			if (state == awaiting_cmd)
				goto resume_point;

			for (;;)
			{
				{
					task<T> task = detail::invoke(start, c);
					if (task.empty())
					{
						res.construct(aw::value());
						state = done;
						return;
					}

					cmd = detail::fetch_command(task);
					if (cmd)
					{
						state = awaiting_cmd;
						return;
					}

					r.construct(task.dismiss());
				}

			resume_point:
				if (r->has_exception())
				{
					res.construct(std::move(r->exception()));
					r.destroy();
					state = done;
					return;
				}

				detail::invoke_loop_update<T>::invoke(c, update, *r); // XXX exception
				r.destroy();
			}
		}

		enum state_t { init, awaiting_cmd, done };
		state_t state;
		detail::command_ptr<T> cmd;
		detail::schroedinger<result<T>> r;
		detail::schroedinger<result<void>> res;
	};

	struct cmd2
		: detail::command<void>, detail::task_completion<T>
	{
		cmd2(Ctx & c, StartF & start, UpdateF & update, coroutine & coro)
			: m_ctx(std::move(c)), m_start(std::move(start)), m_update(std::move(update)), m_coro(std::move(coro))
		{
		}

		result<void> dismiss(cancel_info ci) noexcept override
		{
			while (m_coro.state == coroutine::awaiting_cmd)
			{
				m_coro.r.construct(m_coro.cmd.dismiss(ci));
				m_coro.process(m_ctx, m_start, m_update);
			}

			assert(m_coro.state == coroutine::done);
			return std::move(*m_coro.res);
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink) noexcept override
		{
			m_sink = &sink;
			for (;;)
			{
				task<T> t = m_coro.cmd.start(sch, *this);
				if (t.empty())
					return nullptr;

				m_coro.r.construct(t.dismiss());
				m_coro.process(m_ctx, m_start, m_update);

				if (m_coro.state == coroutine::done)
					return std::move(*m_coro.res);
			}
		}

		task<void> cancel(detail::scheduler & sch) noexcept override
		{
			for (;;)
			{
				task<T> t = m_coro.cmd.cancel(sch);
				if (t.empty())
					return nullptr;

				m_coro.r.construct(t.dismiss());
				m_coro.process(m_ctx, m_start, m_update);

				if (m_coro.state == coroutine::done)
					return std::move(*m_coro.res);
			}
		}

	private:
		void on_completion(detail::scheduler & sch, task<T> && t) override
		{
			m_coro.cmd.complete();

			m_coro.cmd = detail::fetch_command(t);
			if (!m_coro.cmd)
			{
				m_coro.r.construct(t.dismiss());
				m_coro.process(m_ctx, m_start, m_update);
				if (m_coro.state == coroutine::done)
					return m_sink->on_completion(sch, *m_coro.res);
			}

			task<void> tt = this->start(sch, *m_sink);
			if (!tt.empty())
				m_sink->on_completion(sch, std::move(tt));
		}

		detail::task_completion<void> * m_sink;
		Ctx m_ctx;
		StartF m_start;
		UpdateF m_update;
		coroutine m_coro;
	};

	coroutine coro;
	coro.process(c, start, update);
	assert(coro.state != coroutine::init);

	if (coro.state == coroutine::done)
		return std::move(*coro.res);

	try
	{
		detail::command_ptr<void> p(new cmd2(c, start, update, coro));
		return detail::from_command(p);
	}
	catch (...)
	{
		for (;;)
		{
			coro.r.construct(coro.cmd.dismiss(std::current_exception()));
			coro.process(c, start, update);
			assert(coro.state != coroutine::init);
			if (coro.state == coroutine::done)
				return std::move(*coro.res);
		}
	}

}
