#ifndef AWAIT_DETAIL_WIN32_SCHEDULER_H
#define AWAIT_DETAIL_WIN32_SCHEDULER_H

#include "task_fwd.h"
#include <windows.h>

struct aw::detail::scheduler
{
	struct completion_sink
	{
		enum class completion_result { finish, reschedule };
		virtual completion_result on_completion(scheduler & sch) = 0;
	};

	virtual cancel_info get_cancel_info() const noexcept = 0;

	virtual void add_handle(HANDLE h, completion_sink & sink) = 0;
	virtual void remove_handle(HANDLE h, completion_sink & sink) noexcept = 0;

	struct sleeper_node
	{
		virtual void wakeup() = 0;
	};

	virtual sleeper_node * register_sleeper(completion_sink & sink) = 0;
	virtual void wait_for_sleeper(sleeper_node & sleeper) = 0;
};

#endif // AWAIT_DETAIL_WIN32_SCHEDULER_H
