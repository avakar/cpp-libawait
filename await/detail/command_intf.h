#ifndef AWAIT_DETAIL_COMMAND_INTF_H
#define AWAIT_DETAIL_COMMAND_INTF_H

#include "../result.h"
#include "task_fwd.h"
#include "../cancel.h"

namespace aw {
namespace detail {

struct scheduler;

cancel_info get_cancel_info(scheduler & sch);

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
	virtual task<T> cancel(scheduler & sch) noexcept = 0;
};

}
}

#endif // AWAIT_DETAIL_COMMAND_INTF_H
