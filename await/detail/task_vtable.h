#ifndef AWAIT_DETAIL_TASK_VTABLE_H
#define AWAIT_DETAIL_TASK_VTABLE_H

#include "../result.h"

namespace aw {

template <typename T>
struct task;

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
	virtual result<T> dismiss() = 0;
	virtual task<T> start(scheduler & sch, task_completion<T> & sink) = 0;
};

}
}

#endif // AWAIT_DETAIL_TASK_VTABLE_H
