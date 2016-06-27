#include "../task.h"
#include "../cancel.h"

aw::task<void> aw::postpone()
{
	struct cmd
	{
		typedef void value_type;

		result<void> dismiss(cancel_info ci)
		{
			return ci;
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			(void)sch;
			(void)sink;
			return aw::value();
		}

		result<void> cancel(detail::scheduler & sch, cancel_info ci)
		{
			assert(false);
			return ci;
		}
	};

	return detail::make_command<cmd>();
}
