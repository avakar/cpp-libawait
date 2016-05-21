#include "../timer.h"
#include "task_vtable.h"
#include "linux_scheduler.h"
#include <unistd.h>
#include <poll.h>
#include <sys/timerfd.h>

namespace aw {

template <typename I, typename... P>
task<typename I::value_type> make_command(P &&... p)
{
	typedef typename I::value_type T;

	task<T> t;

	void * storage = detail::task_access::storage(t);
	size_t size = detail::task_access::storage_size<T>();

	if (sizeof(I) <= size)
	{
		struct impl
		{
			static task<T> start(void * self, detail::scheduler & sch, detail::task_completion<T> & sink)
			{
				return static_cast<I *>(self)->start(sch, sink);
			}

			static void move_to(void * self, void * dst)
			{
				I * ss = static_cast<I *>(self);
				new(dst) I(std::move(*ss));
				ss->~I();
			}

			static void destroy(void * self)
			{
				static_cast<I *>(self)->~I();
			}
		};

		static detail::task_vtable<T> const vtable = {
			&impl::start,
			nullptr,
			&impl::move_to,
			&impl::destroy,
		};

		new(storage) I(std::forward<P>(p)...);
		detail::task_access::set_vtable(t, &vtable);
	}
	else
	{
		struct impl
		{
			static task<T> start(void * self, detail::scheduler & sch, detail::task_completion<T> & sink)
			{
				return (*static_cast<I **>(self))->start(sch, sink);
			}

			static void move_to(void * self, void * dst)
			{
				I ** ss = static_cast<I **>(self);
				new(dst) I *(*ss);
			}

			static void destroy(void * self)
			{
				I ** ss = static_cast<I **>(self);
				delete ss;
			}
		};

		static detail::task_vtable<T> const vtable = {
			&impl::start,
			nullptr,
			&impl::move_to,
			&impl::destroy,
		};

		I * ss = new(std::nothrow) I(std::forward<P>(p)...);
		if (ss == nullptr)
		{
			t = std::make_exception_ptr(std::bad_alloc());
		}
		else
		{
			new(storage) I *(ss);
			detail::task_access::set_vtable(t, &vtable);
		}
	}

	return std::move(t);
}

}

aw::task<void> aw::wait_ms(int64_t ms)
{
	struct impl
		: fd_completion_sink
	{
		typedef void value_type;

		explicit impl(int64_t ms)
			: m_sink(nullptr)
		{
			m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

			itimerspec ts = {};
			ts.it_value.tv_sec = ms / 1000;
			ts.it_value.tv_nsec = (ms % 1000) * 1000000;
			timerfd_settime(m_fd, 0, &ts, nullptr);
		}

		~impl()
		{
			if (m_fd != -1)
				close(m_fd);
		}

		impl(impl && o)
			: m_fd(o.m_fd)
		{
			o.m_fd = -1;
		}

		impl & operator=(impl && o)
		{
			m_fd = o.m_fd;
			o.m_fd = -1;
			return *this;
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			sch.add_fd(m_fd, POLLIN, *this);
			m_sink = &sink;
			return nullptr;
		}

		void on_completion(short) override
		{
			m_sink->on_completion(aw::value());
		}

		int m_fd;
		detail::task_completion<void> * m_sink;
	};

	return make_command<impl>(ms);
}
