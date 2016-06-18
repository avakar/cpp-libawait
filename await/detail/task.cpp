#include "../task.h"
#include "../cancel.h"

aw::task<void> aw::postpone()
{
	struct cmd
	{
		typedef void value_type;

		result<void> dismiss()
		{
			return std::make_exception_ptr(aw::task_aborted());
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			(void)sch;
			(void)sink;
			return aw::value();
		}

		task<void> cancel(detail::scheduler & sch)
		{
			(void)sch;
			assert(false);
			return nullptr;
		}
	};

	return detail::make_command<cmd>();
}
