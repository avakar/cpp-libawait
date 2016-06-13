#ifndef AWAIT_DETAIL_TASK_ACCESS_H
#define AWAIT_DETAIL_TASK_ACCESS_H

#include "command_intf.h"
#include "task_traits.h"
#include "task_fwd.h"
#include "command_ptr.h"
#include <stddef.h>

namespace aw {
namespace detail {

template <typename T>
void move_value(void * dst, void * src);

template <>
void move_value<void>(void * dst, void * src);

template <typename T>
void destroy_value(void * p);

template <>
void destroy_value<void>(void * p);

template <typename T>
result<T> dismiss_value(void * p);

template <>
result<void> dismiss_value<void>(void * p);

template <typename T>
void move_task(task<T> & dst, task<T> & src);

template <typename T>
bool has_command(task<T> const & t);

template <typename T>
command_ptr<T> fetch_command(task<T> & t);

template <typename T>
bool start_command(task<T> & t, scheduler & sch, task_completion<T> & sink);

template <typename T, typename F>
auto start_command(command_ptr<T> & cmd, scheduler & sch, task_completion<T> & sink, F && f)
	-> typename task_traits<decltype(f(std::declval<result<T>>()))>::task_type;

template <typename T>
result<T> dismiss_task(task<T> & t);

template <typename T>
void mark_complete(task<T> & t);

}
}

#endif // AWAIT_DETAIL_TASK_ACCESS_H
