#include "task.h"
#include <assert.h>

aw::task<void>::task()
	: m_kind(kind::empty)
{
}

aw::task<void>::~task()
{
	this->clear();
}

aw::task<void>::task(task && o)
	: m_kind(kind::empty)
{
	*this = std::move(o);
}

aw::task<void> & aw::task<void>::operator=(task && o)
{
	this->clear();

	m_kind = o.m_kind;
	o.m_kind = kind::empty;
	if (m_kind == kind::exception)
	{
		std::exception_ptr & oo = reinterpret_cast<std::exception_ptr &>(o.m_storage);
		new(&m_storage) std::exception_ptr(std::move(oo));
		oo.~exception_ptr();
	}

	return *this;
}

aw::task<void>::task(std::nullptr_t)
	: m_kind(kind::empty)
{
}

aw::task<void>::task(std::exception_ptr e)
	: m_kind(kind::exception)
{
	new(&m_storage) std::exception_ptr(std::move(e));
}

aw::task<void>::task(result<void> const & v)
{
	if (v.has_value())
	{
		m_kind = kind::value;
	}
	else
	{
		m_kind = kind::exception;
		new(&m_storage) std::exception_ptr(v.exception());
	}
}

void aw::task<void>::clear()
{
	switch (m_kind)
	{
	case kind::empty:
		break;
	case kind::value:
		break;
	case kind::exception:
		reinterpret_cast<std::exception_ptr &>(m_storage).~exception_ptr();
		break;
	}

	m_kind = kind::empty;
}

bool aw::task<void>::empty() const
{
	return m_kind == kind::empty;
}
