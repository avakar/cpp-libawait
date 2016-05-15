#ifndef AWAIT_DETAIL_TASK_ACCESS_H
#define AWAIT_DETAIL_TASK_ACCESS_H

#include "task_vtable.h"

namespace aw {

template <typename T>
struct task;

namespace detail {

struct task_access
{
	template <typename T>
	static task_vtable<T> const * pull_vtable(task<T> & t);

	template <typename T>
	static void * storage(task<T> & t);
};

}
}

#endif // AWAIT_DETAIL_TASK_ACCESS_H
