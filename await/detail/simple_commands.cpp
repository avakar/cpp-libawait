#include "simple_commands.h"

template <>
aw::detail::task_vtable<void> const * aw::detail::value_vtable()
{
	struct impl
	{
		static result<void> get_result(void * self)
		{
			(void)self;
			return aw::value();
		}

		static void move_to(void * self, void * dst)
		{
			(void)self;
			(void)dst;
		}

		static void destroy(void * self)
		{
			(void)self;
		}
	};

	static task_vtable<void> const vtable = {
		&impl::get_result,
		&impl::move_to,
		&impl::destroy,
	};

	return &vtable;
}

template <>
aw::detail::task_vtable<void> const * aw::detail::construct_result(void * self, result<void> const & v)
{
	if (v.has_exception())
		return construct_exception<void>(self, v.exception());
	return value_vtable<void>();
}

template <>
aw::detail::task_vtable<void> const * aw::detail::construct_result(void * self, result<void> && v)
{
	if (v.has_exception())
		return construct_exception<void>(self, std::move(v.exception()));
	return value_vtable<void>();
}
