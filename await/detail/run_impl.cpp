#include "../run.h"

aw::result<void> aw::detail::try_run_impl(task<void> && t)
{
	assert(!t.empty());

	detail::task_vtable<void> const * vtable = detail::task_access::pull_vtable(t);
	void * storage = detail::task_access::storage(t);
	return vtable->get_result(storage);
}
