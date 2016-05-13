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

void aw::result<void>::get()
{
	(void)this->result<detail::unit_t>::get();
}

void aw::result<void>::value()
{
	(void)this->result<detail::unit_t>::value();
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
