#ifndef AWAIT_DETAIL_WIN32_SCHEDULER_H
#define AWAIT_DETAIL_WIN32_SCHEDULER_H

#include "command_intf.h"

struct aw::detail::scheduler
{
	struct fd_completion_sink
	{
		virtual void on_completion(scheduler & sch, short revents) = 0;
	};

	virtual void add_fd(int fd, short events, fd_completion_sink & sink) = 0;
	virtual void remove_fd(int fd, fd_completion_sink & sink) = 0;
};

#endif // AWAIT_DETAIL_WIN32_SCHEDULER_H
