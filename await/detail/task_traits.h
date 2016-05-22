#ifndef AWAIT_DETAIL_TASK_TRAITS_H
#define AWAIT_DETAIL_TASK_TRAITS_H

#include "../task.h"

namespace aw {
namespace detail {

template <typename T>
struct task_traits;

template <typename T>
struct task_traits<task<T>>
{
	typedef T value_type;
};

template <typename T>
struct task_traits<result<T>>
{
	typedef T value_type;
};

}
}

#endif // AWAIT_DETAIL_TASK_TRAITS_H
