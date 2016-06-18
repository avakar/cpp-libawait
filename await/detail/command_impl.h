#include "command_intf.h"

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

			aw::task<T> start(scheduler & sch, aw::detail::task_completion<T> & sink) noexcept override
			{
				try
				{
					return this->I::start(sch, sink);
				}
				catch (...)
				{
					return std::current_exception();
				}
			}

			aw::result<T> dismiss() noexcept override
			{
				try
				{
					return this->I::dismiss();
				}
				catch (...)
				{
					return std::current_exception();
				}
			}

			aw::task<T> cancel(scheduler & sch) noexcept override
			{
				try
				{
					return this->I::cancel(sch);
				}
				catch (...)
				{
					return std::current_exception();
				}
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
