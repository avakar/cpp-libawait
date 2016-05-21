#ifndef AWAIT_DETAIL_SIMPLE_COMMANDS_H
#define AWAIT_DETAIL_SIMPLE_COMMANDS_H

#include "task_vtable.h"

namespace aw {
namespace detail {

template <typename T>
task_vtable<T> const * value_vtable()
{
	struct impl
	{
		static result<T> get_result(void * self)
		{
			T * o = reinterpret_cast<T *>(self);
			result<T> r = result<T>::from_value(std::move(*o));
			o->~T();
			return std::move(r);
		}

		static void move_to(void * self, void * dst)
		{
			T * o = reinterpret_cast<T *>(self);
			new(dst) T(std::move(*o));
			o->~T();
		}

		static void destroy(void * self)
		{
			T * o = reinterpret_cast<T *>(self);
			o->~T();
		}
	};

	static task_vtable<T> const vtable = {
		nullptr,
		&impl::get_result,
		&impl::move_to,
		&impl::destroy,
	};

	return &vtable;
}

template <>
task_vtable<void> const * value_vtable();

template <typename T>
task_vtable<T> const * construct_exception(void * self, std::exception_ptr && e)
{
	struct impl
	{
		static result<T> get_result(void * self)
		{
			std::exception_ptr * o = reinterpret_cast<std::exception_ptr *>(self);
			result<T> r(std::move(*o));
			o->~exception_ptr();
			return std::move(r);
		}

		static void move_to(void * self, void * dst)
		{
			std::exception_ptr * o = reinterpret_cast<std::exception_ptr *>(self);
			new(dst) std::exception_ptr(std::move(*o));
			o->~exception_ptr();
		}

		static void destroy(void * self)
		{
			std::exception_ptr * o = reinterpret_cast<std::exception_ptr *>(self);
			o->~exception_ptr();
		}
	};

	static task_vtable<T> const vtable = {
		nullptr,
		&impl::get_result,
		&impl::move_to,
		&impl::destroy,
	};

	new(self) std::exception_ptr(std::move(e));
	return &vtable;
}

template <typename T, typename U>
task_vtable<T> const * construct_result(void * self, result<U> const & v)
{
	if (v.has_exception())
		return construct_exception<T>(self, v.exception());

	new(self) T(v.value());
	return value_vtable<T>();
}

template <typename T, typename U>
task_vtable<T> const * construct_result(void * self, result<U> && v)
{
	if (v.has_exception())
		return construct_exception<T>(self, std::move(v.exception()));

	new(self) T(std::move(v.value()));
	return value_vtable<T>();
}

template <>
task_vtable<void> const * construct_result(void * self, result<void> const & v);

template <>
task_vtable<void> const * construct_result(void * self, result<void> && v);

}
}

#endif // AWAIT_DETAIL_SIMPLE_COMMANDS_H
