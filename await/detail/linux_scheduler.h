#ifndef AWAIT_DETAIL_WIN32_SCHEDULER_H
#define AWAIT_DETAIL_WIN32_SCHEDULER_H

#include "task_vtable.h"

struct fd_completion_sink
{
	virtual void on_completion(short revents) = 0;
};

struct aw::detail::scheduler
{
	virtual void add_fd(int fd, short events, fd_completion_sink & sink) = 0;
};

#endif // AWAIT_DETAIL_WIN32_SCHEDULER_H
