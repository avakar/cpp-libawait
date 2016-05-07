#include "../run.h"

aw::result<void> aw::detail::to_result(task<void> && t)
{
	detail::task_kind kind = detail::task_access::get_kind(t);
	if (kind == detail::task_kind::exception)
		return aw::result<void>(std::move(detail::task_access::as_exception(t)));

	assert(kind == detail::task_kind::value);
	return aw::result<void>::from_value();
}
