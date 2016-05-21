#ifndef AWAIT_DETAIL_WIN32_SCHEDULER_H
#define AWAIT_DETAIL_WIN32_SCHEDULER_H

#include "task_vtable.h"
#include <windows.h>

struct aw::detail::scheduler
{
	struct handle_completion_sink
	{
		virtual void on_completion() = 0;
	};

	virtual void add_handle(HANDLE h, handle_completion_sink & sink) = 0;
};

#endif // AWAIT_DETAIL_WIN32_SCHEDULER_H
