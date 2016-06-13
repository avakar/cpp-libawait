#ifndef AWAIT_DETAIL_COMMAND_INTF_H
#define AWAIT_DETAIL_COMMAND_INTF_H

#include "../result.h"
#include "task_fwd.h"

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
	virtual result<T> dismiss() noexcept = 0;
	virtual task<T> start(scheduler & sch, task_completion<T> & sink) noexcept = 0;
	virtual void cancel(scheduler & sch) noexcept = 0;
};

}
}

#endif // AWAIT_DETAIL_COMMAND_INTF_H