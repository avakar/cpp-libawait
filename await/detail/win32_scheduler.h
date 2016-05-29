#ifndef AWAIT_DETAIL_WIN32_SCHEDULER_H
#define AWAIT_DETAIL_WIN32_SCHEDULER_H

#include "task_vtable.h"
#include <windows.h>

struct aw::detail::scheduler
{
	struct completion_sink
	{
		enum class completion_result { finish, reschedule };
		virtual completion_result on_completion(scheduler & sch) = 0;
	};

	virtual void add_handle(HANDLE h, completion_sink & sink) = 0;

	struct sleeper_node
	{
		virtual void wakeup() = 0;
	};

	virtual sleeper_node * register_sleeper(completion_sink & sink) = 0;
};

#endif // AWAIT_DETAIL_WIN32_SCHEDULER_H
