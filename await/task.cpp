#include "task.h"
#include <assert.h>

aw::task<void>::task()
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

/*void aw::run(task<void> && t)
{
	assert(!t.empty());
	aw::try_run(std::move(t)).get();
}
*/
