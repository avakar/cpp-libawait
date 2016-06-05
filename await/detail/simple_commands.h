#ifndef AWAIT_DETAIL_SIMPLE_COMMANDS_H
#define AWAIT_DETAIL_SIMPLE_COMMANDS_H

#include "task_vtable.h"

namespace aw {
namespace detail {

template <typename T>
task_vtable<T> const * result_vtable()
{
	struct impl
	{
		static void move_to(void * self, void * dst)
		{
			result<T> * o = reinterpret_cast<result<T> *>(self);
			new(dst) result<T>(std::move(*o));
			o->~result();
		}

		static void destroy(void * self)
		{
			result<T> * o = reinterpret_cast<result<T> *>(self);
			o->~result();
		}
	};

	static task_vtable<T> const vtable = {
		nullptr,
		&impl::move_to,
		&impl::destroy,
	};

	return &vtable;
}

template <typename T>
task_vtable<T> const * construct_exception(void * self, std::exception_ptr e)
{
	new(self) result<T>(std::move(e));
	return result_vtable<T>();
}

template <typename T, typename U>
task_vtable<T> const * construct_result(void * self, result<U> const & v)
{
	new(self) result<T>(v);
	return result_vtable<T>();
}

template <typename T, typename U>
task_vtable<T> const * construct_result(void * self, result<U> && v)
{
	new(self) result<T>(std::move(v));
	return result_vtable<T>();
}

}
}

#endif // AWAIT_DETAIL_SIMPLE_COMMANDS_H
