#ifndef AWAIT_DETAIL_TASK_STORAGE_H
#define AWAIT_DETAIL_TASK_STORAGE_H

#include <type_traits>
#include <exception>
#include <cstddef>

namespace aw {
namespace detail {

template <typename T>
struct task_storage
	: std::aligned_union<sizeof(intptr_t[4]), T, std::exception_ptr, std::max_align_t>
{
};

template <>
struct task_storage<void>
	: std::aligned_union<sizeof(intptr_t[4]), std::exception_ptr, std::max_align_t>
{
};

}
}

#endif // AWAIT_DETAIL_TASK_STORAGE_H
