#ifndef AWAIT_DETAIL_TASK_STORAGE_H
#define AWAIT_DETAIL_TASK_STORAGE_H

#include "task_vtable.h"
#include <type_traits>
#include <exception>

namespace aw {
namespace detail {

template <typename T>
struct task_storage
	: std::aligned_union<0, command<T> *, T, std::exception_ptr>
{
};

template <>
struct task_storage<void>
	: std::aligned_union<0, command<void> *, std::exception_ptr>
{
};

}
}

#endif // AWAIT_DETAIL_TASK_STORAGE_H
