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
	static task_vtable<T> const * get_vtable(task<T> const & t);

	template <typename T>
	static void set_vtable(task<T> & t, typename identity<task_vtable<T> const *>::type vtable);

	template <typename T>
	static void * storage(task<T> & t);

	template <typename T>
	static constexpr size_t storage_size();
};

template <typename T>
bool has_exception(task<T> const & t);

template <typename T>
std::exception_ptr fetch_exception(task<T> & t);

template <typename T>
bool has_result(task<T> const & t);

template <typename T>
result<T> const & get_result(task<T> const & t);

template <typename T>
result<T> & get_result(task<T> & t);

template <typename T>
result<T> fetch_result(task<T> & t);

template <typename T>
bool has_command(task<T> const & t);

template <typename T>
bool start_command(task<T> & t, scheduler & sch, task_completion<T> & sink);

template <typename T>
result<T> dismiss_task(task<T> & t);

template <typename T>
void mark_complete(task<T> & t);

}
}

#endif // AWAIT_DETAIL_TASK_ACCESS_H
