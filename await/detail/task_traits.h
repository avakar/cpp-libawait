#ifndef AWAIT_DETAIL_TASK_TRAITS_H
#define AWAIT_DETAIL_TASK_TRAITS_H

#include "../result.h"
#include "task_fwd.h"

namespace aw {

template <typename T>
struct task;

namespace detail {

template <typename T>
struct task_traits
{
	typedef T value_type;
	typedef task<T> task_type;
	static task_type taskify(T && v) noexcept;
};

template <typename T>
struct task_traits<result<T>>
{
	typedef T value_type;
	typedef task<T> task_type;
	static task_type taskify(result<T> && v) noexcept;
};

template <typename T>
struct task_traits<task<T>>
{
	typedef T value_type;
	typedef task<T> task_type;
	static task_type && taskify(task<T> && v) noexcept;
};

template <>
struct task_traits<void>
{
	typedef void value_type;
	typedef task<void> task_type;
};

template <typename F, typename... P>
struct invoke_and_taskify_traits
{
	typedef typename std::result_of<F(P &&...)>::type return_type;
	typedef typename task_traits<return_type>::value_type value_type;
	typedef typename task_traits<return_type>::task_type task_type;
};

template <typename F, typename... P>
auto invoke_and_taskify(F && f, P &&... p) noexcept -> typename invoke_and_taskify_traits<F, P...>::task_type;

}
}

#endif // AWAIT_DETAIL_TASK_TRAITS_H
