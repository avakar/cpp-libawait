#include "result.h"

aw::result<void>::result(result const & o)
	: result<detail::unit_t>(static_cast<result<detail::unit_t> const &>(o))
{
}

aw::result<void>::result(result && o)
	: result<detail::unit_t>(static_cast<result<detail::unit_t> &&>(o))
{
}

aw::result<void>::result(std::exception_ptr e)
	: result<detail::unit_t>(e)
{
}

bool aw::result<void>::has_value() const
{
	return this->result<detail::unit_t>::has_value();
}

bool aw::result<void>::has_exception() const
{
	return this->result<detail::unit_t>::has_exception();
}

void aw::result<void>::get()
{
	(void)this->result<detail::unit_t>::get();
}

std::exception_ptr aw::result<void>::exception() const
{
	return this->result<detail::unit_t>::exception();
}

void aw::result<void>::rethrow() const
{
	this->result<detail::unit_t>::rethrow();
}

aw::result<void> aw::result<void>::from_value()
{
	return aw::result<void>();
}

aw::result<void>::result()
	: result<detail::unit_t>(result<detail::unit_t>::from_value(detail::unit_t()))
{
}

aw::result<void> aw::value()
{
	return aw::result<void>::from_value();
}
