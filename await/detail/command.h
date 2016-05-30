#ifndef AWAIT_DETAIL_COMMAND_H
#define AWAIT_DETAIL_COMMAND_H

#include "../task.h"
#include <exception>

namespace aw {
namespace detail {

template <typename I, typename... P>
task<typename I::value_type> make_command(P &&... p);

template <bool fits>
struct make_command_impl;

template <>
struct make_command_impl<true>
{
	template <typename I, typename... P>
	static void make(task<typename I::value_type> & task, P &&... p)
	{
		typedef typename I::value_type T;
		void * storage = detail::task_access::storage(task);

		struct impl
		{
			static aw::task<T> start(void * self, detail::scheduler & sch, detail::task_completion<T> & sink)
			{
				return static_cast<I *>(self)->start(sch, sink);
			}

			static void move_to(void * self, void * dst)
			{
				I * ss = static_cast<I *>(self);
				new(dst) I(std::move(*ss));
				ss->~I();
			}

			static void destroy(void * self)
			{
				static_cast<I *>(self)->~I();
			}
		};

		static detail::task_vtable<T> const vtable = {
			&impl::start,
			nullptr,
			&impl::move_to,
			&impl::destroy,
		};

		new(storage) I(std::forward<P>(p)...);
		detail::task_access::set_vtable(task, &vtable);
	}
};

template <>
struct make_command_impl<false>
{
	template <typename I, typename... P>
	static void make(task<typename I::value_type> & task, P &&... p)
	{
		typedef typename I::value_type T;
		void * storage = detail::task_access::storage(task);

		struct impl
		{
			static aw::task<T> start(void * self, detail::scheduler & sch, detail::task_completion<T> & sink)
			{
				return (*static_cast<I **>(self))->start(sch, sink);
			}

			static void move_to(void * self, void * dst)
			{
				I ** ss = static_cast<I **>(self);
				new(dst) I *(*ss);
			}

			static void destroy(void * self)
			{
				I ** ss = static_cast<I **>(self);
				delete *ss;
			}
		};

		static detail::task_vtable<T> const vtable = {
			&impl::start,
			nullptr,
			&impl::move_to,
			&impl::destroy,
		};

		I * ss = new I(std::forward<P>(p)...);
		new(storage) I *(ss);
		detail::task_access::set_vtable(task, &vtable);
	}
};

}
}

template <typename I, typename... P>
aw::task<typename I::value_type> aw::detail::make_command(P &&... p)
{
	typedef typename I::value_type T;

	task<T> t;

	try
	{
		typedef aw::detail::make_command_impl<sizeof(I) <= detail::task_access::storage_size<T>()> impl;
		impl::template make<I>(t, std::forward<P>(p)...);
	}
	catch (...)
	{
		t = std::current_exception();
	}

	return std::move(t);
}

#endif // AWAIT_DETAIL_COMMAND_H
