#ifndef AWAIT_DETAIL_WIN32_SCHEDULER_H
#define AWAIT_DETAIL_WIN32_SCHEDULER_H

#include "task_fwd.h"
#include <windows.h>

struct releaser
{
	template <typename T>
	void operator()(T * p)
	{
		p->release();
	}
};

#include <memory>
#include <system_error>

struct aw::detail::scheduler
{
	struct completion_sink
	{
		enum class completion_result { finish, reschedule };
		virtual completion_result on_completion(scheduler & sch) = 0;
	};

	struct sleeper_node
	{
		virtual void wakeup() = 0;
	};

	virtual sleeper_node * register_sleeper(completion_sink & sink) = 0;
	virtual void wait_for_sleeper(sleeper_node & sleeper) = 0;

	struct token_base
	{
		virtual void release() = 0;
	};

	using token = std::unique_ptr<token_base, releaser>;

	struct alloc_event_sink
	{
		virtual void on_event_ready(scheduler & sch, HANDLE h) noexcept = 0;
	};

	virtual token alloc_event(alloc_event_sink & sink, std::error_code & ec) noexcept = 0;
	virtual void add_handle(token const & tok, completion_sink & sink) noexcept = 0;
	virtual void remove_handle(token const & tok) noexcept = 0;
};

#endif // AWAIT_DETAIL_WIN32_SCHEDULER_H
