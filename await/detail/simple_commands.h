#ifndef AWAIT_DETAIL_SIMPLE_COMMANDS_H
#define AWAIT_DETAIL_SIMPLE_COMMANDS_H

#include "task_vtable.h"

namespace aw {
namespace detail {

template <typename T, typename U>
task_kind construct_result(void * storage, result<U> && v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(std::move(v.exception()));
		return detail::task_kind::exception;
	}

	try
	{
		new(storage) T(std::move(v.value()));
		return detail::task_kind::value;
	}
	catch (...)
	{
		new(storage) std::exception_ptr(std::current_exception());
		return detail::task_kind::exception;
	}
}

template <typename T, typename U>
task_kind construct_result(void * storage, result<U> const & v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(v.exception());
		return detail::task_kind::exception;
	}

	try
	{
		new(storage) T(v.value());
		return detail::task_kind::value;
	}
	catch (...)
	{
		new(storage) std::exception_ptr(std::current_exception());
		return detail::task_kind::exception;
	}
}

template <>
inline task_kind construct_result<void, void>(void * storage, result<void> && v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(std::move(v.exception()));
		return detail::task_kind::exception;
	}

	return detail::task_kind::value;
}

template <>
inline task_kind construct_result<void, void>(void * storage, result<void> const & v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(v.exception());
		return detail::task_kind::exception;
	}

	return detail::task_kind::value;
}

}
}

#endif // AWAIT_DETAIL_SIMPLE_COMMANDS_H
