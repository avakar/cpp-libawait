#include "../timer.h"
#include "command.h"
#include "linux_scheduler.h"
#include <unistd.h>
#include <poll.h>
#include <sys/timerfd.h>

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
