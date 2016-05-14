#include "result.h"

aw::result<void>::result(std::exception_ptr e)
	: m_exception(std::move(e))
{
	assert(m_exception != nullptr);
}

bool aw::result<void>::has_value() const
{
	return m_exception == nullptr;
}

bool aw::result<void>::has_exception() const
{
	return m_exception != nullptr;
}

void aw::result<void>::get()
{
	this->rethrow();
}

std::exception_ptr aw::result<void>::exception() const
{
	return m_exception;
}

void aw::result<void>::rethrow() const
{
	if (m_exception != nullptr)
		std::rethrow_exception(m_exception);
}

aw::result<void> aw::result<void>::from_value()
{
	return aw::result<void>();
}

aw::result<void>::result()
	: m_exception(nullptr)
{
}

aw::result<void> aw::value()
{
	return aw::result<void>::from_value();
}
