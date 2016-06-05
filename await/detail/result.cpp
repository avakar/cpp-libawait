#include "../result.h"

aw::result<void>::result() noexcept
	: m_exception(nullptr)
{
}

aw::result<void>::result(std::exception_ptr e) noexcept
	: m_exception(std::move(e))
{
	assert(m_exception != nullptr);
}

aw::result<void>::result(in_place_t) noexcept
	: m_exception(nullptr)
{
}

bool aw::result<void>::has_value() const noexcept
{
	return m_exception == nullptr;
}

bool aw::result<void>::has_exception() const noexcept
{
	return m_exception != nullptr;
}

std::exception_ptr & aw::result<void>::exception() noexcept
{
	return m_exception;
}

std::exception_ptr const & aw::result<void>::exception() const noexcept
{
	return m_exception;
}

void aw::result<void>::get()
{
	this->rethrow();
}

void aw::result<void>::rethrow() const
{
	if (m_exception != nullptr)
		std::rethrow_exception(m_exception);
}

aw::result<void> aw::value() noexcept
{
	return aw::result<void>(in_place);
}
