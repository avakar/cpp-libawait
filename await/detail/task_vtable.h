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
	// Moves the command from `self` to `dst` and destroys `self`. Shall only be called in `ready` state.
	// 
	void (*move_to)(void * self, void * dst);

	// Destroys `self`. May only be called in `complete` state.
	void (*destroy)(void * self);

	// Completes the command synchronously. Shall only be called in `ready` state.
	result<T> (*dismiss)(void * self);

	// Starts the command. The command is started if the returned task is empty.
	// Otherwise, the command is destroyed and should be replaced by the returned task.
	task<T> (*start)(void * self, scheduler & sch, task_completion<T> & sink);
};

}
}

#endif // AWAIT_DETAIL_TASK_VTABLE_H
