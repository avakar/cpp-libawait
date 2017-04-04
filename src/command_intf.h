#ifndef AWAIT_DETAIL_COMMAND_INTF_H
#define AWAIT_DETAIL_COMMAND_INTF_H

#include <avakar/await/result.h>
#include "task_fwd.h"
#include <avakar/await/cancel.h>

namespace aw {
namespace detail {

struct scheduler;

template <typename T>
struct task_completion
{
	virtual void on_completion(scheduler & sch, task<T> && t) = 0;
};

template <typename T>
struct command
{
	virtual ~command() {}
	virtual result<T> dismiss(cancel_info ci) noexcept = 0;
	virtual task<T> start(scheduler & sch, task_completion<T> & sink) noexcept = 0;
	virtual result<T> cancel(scheduler & sch, cancel_info ci) noexcept = 0;
};

}
}

#endif // AWAIT_DETAIL_COMMAND_INTF_H
