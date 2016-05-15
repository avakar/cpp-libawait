#ifndef AWAIT_DETAIL_TASK_STORAGE_H
#define AWAIT_DETAIL_TASK_STORAGE_H

#include <type_traits>
#include <exception>

namespace aw {
namespace detail {

template <typename T>
struct task_storage
	: std::aligned_union<0, T, std::exception_ptr>
{
};

template <>
struct task_storage<void>
	: std::aligned_union<0, std::exception_ptr>
{
};

}
}

#endif // AWAIT_DETAIL_TASK_STORAGE_H
