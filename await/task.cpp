#include "task.h"
#include <assert.h>

aw::task<void>::task(task && o)
	: task<detail::unit_t>(static_cast<task<detail::unit_t> &&>(o))
{
}

aw::task<void> & aw::task<void>::operator=(task && o)
{
	static_cast<task<detail::unit_t> &>(*this) = std::move(o);
	return *this;
}

aw::task<void>::task(nullptr_t)
	: task<detail::unit_t>(nullptr)
{
}

aw::task<void>::task(std::exception_ptr e)
	: task<detail::unit_t>(std::move(e))
{
}

aw::task<void>::task(result<void> && v)
	: task<detail::unit_t>(static_cast<result<detail::unit_t> &&>(v))
{
}

aw::task<void>::task(result<void> const & v)
	: task<detail::unit_t>(static_cast<result<detail::unit_t> const &>(v))
{
}
