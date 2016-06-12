#ifndef AWAIT_DETAIL_COMMAND_H
#define AWAIT_DETAIL_COMMAND_H

#include "../task.h"
#include "task_vtable.h"
#include <exception>

namespace aw {
namespace detail {

template <typename I, typename... P>
task<typename I::value_type> make_command(P &&... p);

}
}

template <typename I, typename... P>
aw::task<typename I::value_type> aw::detail::make_command(P &&... p)
{
	typedef typename I::value_type T;

	task<T> task;
	try
	{
		struct impl final
			: command<T>, private I
		{
			explicit impl(P &&... p)
				: I(std::forward<P>(p)...)
			{
			}

			aw::task<T> start(scheduler & sch, aw::detail::task_completion<T> & sink) override
			{
				return this->I::start(sch, sink);
			}

			result<T> dismiss() override
			{
				return this->I::dismiss();
			}
		};

		impl * ss = new impl(std::forward<P>(p)...);

		new(task_access::storage(task)) command<T> *(ss);
		task_access::set_kind(task, task_kind::command);
	}
	catch (...)
	{
		task = std::current_exception();
	}

	return task;
}

#endif // AWAIT_DETAIL_COMMAND_H
