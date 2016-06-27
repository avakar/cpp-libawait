#include "linux_scheduler.h"
#include "linux_fd_task.h"
#include "linux_error.h"
#include "../cancel.h"
#include <poll.h>

aw::task<short> aw::detail::linux_fd_task(int fd, short events)
{
	struct impl
		: private scheduler::fd_completion_sink
	{
		typedef short value_type;

		impl(int fd, short events)
			: m_fd(fd), m_events(events), m_sink(nullptr)
		{
		}

		aw::result<short> dismiss(aw::cancel_info ci)
		{
			return ci;
		}

		aw::task<short> start(scheduler & sch, task_completion<short> & sink)
		{
			m_sink = &sink;
			sch.add_fd(m_fd, m_events, *this);
			return nullptr;
		}

		aw::result<short> cancel(scheduler & sch, cancel_info ci)
		{
			// XXX
			sch.remove_fd(m_fd, *this);
			return ci;
		}

	private:
		void on_completion(scheduler & sch, short revents) override
		{
			m_sink->on_completion(sch, aw::value(revents));
		}

		int m_fd;
		short m_events;
		task_completion<short> * m_sink;
	};

	struct pollfd pf;
	pf.fd = fd;
	pf.events = events;

	for (;;)
	{
		int r = poll(&pf, 1, 0);
		if (r == 1)
			return aw::value(pf.revents);

		if (r == 0)
			return aw::detail::make_command<impl>(fd, events);

		r = errno;
		if (r != EINTR)
			return std::make_exception_ptr(linux_error(r));
	}
}
