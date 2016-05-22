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
struct task_vtable
{
	task<T> (*start)(void * self, scheduler & sch, task_completion<T> & sink);
	result<T> (*get_result)(void * self);
	void (*move_to)(void * self, void * dst);
	void (*destroy)(void * self);
};

}
}

#endif // AWAIT_DETAIL_TASK_VTABLE_H
