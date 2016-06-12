#ifndef AWAIT_DETAIL_TASK_ACCESS_H
#define AWAIT_DETAIL_TASK_ACCESS_H

#include "task_vtable.h"
#include <stddef.h>

namespace aw {

template <typename T>
struct task;

namespace detail {

enum class task_kind { empty, value, exception, command };

struct task_access
{
	template <typename T>
	static task_kind get_kind(task<T> const & t);

	template <typename T>
	static void set_kind(task<T> & t, task_kind kind);

	template <typename T>
	static void * storage(task<T> & t);
};

template <typename T>
void move_task(task<T> & dst, task<T> & src);

template <typename T>
bool has_exception(task<T> const & t);

template <typename T>
std::exception_ptr fetch_exception(task<T> & t);

template <typename T>
bool has_command(task<T> const & t);

template <typename T>
bool start_command(task<T> & t, scheduler & sch, task_completion<T> & sink);

template <typename T>
result<T> dismiss_task(task<T> & t);

template <>
result<void> dismiss_task<void>(task<void> & t);

template <typename T>
void mark_complete(task<T> & t);

}
}

#endif // AWAIT_DETAIL_TASK_ACCESS_H
