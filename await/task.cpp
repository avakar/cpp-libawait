#include "task.h"
#include <assert.h>

aw::task<void>::task()
{
}

aw::task<void>::task(std::exception_ptr e)
	: task<detail::unit_t>(std::move(e))
{
}

aw::task<void>::task(result<void> && v)
	: task<detail::unit_t>(std::move(v))
{
}

aw::task<void>::task(result<void> const & v)
	: task<detail::unit_t>(v)
{
}
