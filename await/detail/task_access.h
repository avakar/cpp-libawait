#ifndef AWAIT_DETAIL_TASK_ACCESS_H
#define AWAIT_DETAIL_TASK_ACCESS_H

#include "task_vtable.h"
#include <stddef.h>

namespace aw {

template <typename T>
struct task;

namespace detail {

template <typename T>
struct identity
{
	typedef T type;
};

struct task_access
{
	template <typename T>
	static task_vtable<T> const * get_vtable(task<T> & t);

	template <typename T>
	static void set_vtable(task<T> & t, typename identity<task_vtable<T> const *>::type vtable);

	template <typename T>
	static void * storage(task<T> & t);

	template <typename T>
	static constexpr size_t storage_size();
};

}
}

#endif // AWAIT_DETAIL_TASK_ACCESS_H
