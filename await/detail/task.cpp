#include "../task.h"

aw::task<void> aw::postpone()
{
	struct cmd
	{
		typedef void value_type;

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			(void)sch;
			(void)sink;
			return aw::value();
		}
	};

	return detail::make_command<cmd>();
}
