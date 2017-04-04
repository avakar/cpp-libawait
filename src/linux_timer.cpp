#include <avakar/await/timer.h>
#include <avakar/await/cancel.h>
#include "command.h"
#include "linux_scheduler.h"
#include <unistd.h>
#include <poll.h>
#include <sys/timerfd.h>

aw::task<void> aw::wait_ms(int64_t ms)
{
	struct impl
		: detail::scheduler::fd_completion_sink
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

		result<void> dismiss(cancel_info ci)
		{
			return ci;
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			sch.add_fd(m_fd, POLLIN, *this);
			m_sink = &sink;
			return nullptr;
		}

		result<void> cancel(detail::scheduler & sch, cancel_info ci)
		{
			sch.remove_fd(m_fd, *this);
			return ci;
		}

		void on_completion(detail::scheduler & sch, short) override
		{
			m_sink->on_completion(sch, aw::value());
		}

		int m_fd;
		detail::task_completion<void> * m_sink;
	};

	return detail::make_command<impl>(ms);
}
