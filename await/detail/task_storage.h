#ifndef AWAIT_DETAIL_TASK_STORAGE_H
#define AWAIT_DETAIL_TASK_STORAGE_H

#include <type_traits>
#include <exception>
#include <stdint.h>
#include <stddef.h>

namespace aw {
namespace detail {

template <typename... P>
struct union_size;

template <typename T>
struct union_size<T>
{
	static size_t const value = sizeof(T);
};

template <typename T, typename... P>
struct union_size<T, P...>
{
	static size_t const value = union_size<P...>::value > sizeof(T)? union_size<P...>::value: sizeof(T);
};

template <typename T>
struct task_storage
	: std::aligned_storage<union_size<intptr_t[4], result<T>>::value>
{
};

}
}

#endif // AWAIT_DETAIL_TASK_STORAGE_H
