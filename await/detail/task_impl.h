#include <assert.h>
#include <new>

template <typename T>
aw::detail::task_kind aw::detail::task_access::get_kind(task<T> const & t)
{
	return t.m_kind;
}

template <typename T>
T & aw::detail::task_access::as_value(task<T> & t)
{
	assert(t.m_kind == task_kind::value);
	return reinterpret_cast<T &>(t.m_storage);
}

template <typename T>
T const & aw::detail::task_access::as_value(task<T> const & t)
{
	assert(t.m_kind == task_kind::value);
	return reinterpret_cast<T const &>(t.m_storage);
}

template <typename T>
std::exception_ptr & aw::detail::task_access::as_exception(task<T> & t)
{
	assert(t.m_kind == task_kind::exception);
	return reinterpret_cast<std::exception_ptr &>(t.m_storage);
}

template <typename T>
std::exception_ptr const & aw::detail::task_access::as_exception(task<T> const & t)
{
	assert(t.m_kind == task_kind::exception);
	return reinterpret_cast<std::exception_ptr const &>(t.m_storage);
}

template <typename T>
aw::task<T>::task()
	: m_kind(kind::empty)
{
}

template <typename T>
aw::task<T>::task(task && o)
	: m_kind(kind::empty)
{
	*this = std::move(o);
}

template <typename T>
aw::task<T> & aw::task<T>::operator=(task && o)
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
	else if (m_kind == kind::value)
	{
		T & oo = reinterpret_cast<T &>(o.m_storage);
		new(&m_storage) T(std::move(oo));
		oo.~T();
	}

	return *this;
}

template <typename T>
aw::task<T>::~task()
{
	this->clear();
}

template <typename T>
aw::task<T>::task(std::nullptr_t)
	: m_kind(kind::empty)
{
}

template <typename T>
aw::task<T>::task(std::exception_ptr e)
	: m_kind(kind::exception)
{
	assert(e != nullptr);
	new(&m_storage) std::exception_ptr(std::move(e));
}

template <typename T>
template <typename U>
aw::task<T>::task(result<U> && v)
{
	if (v.has_exception())
	{
		m_kind = kind::exception;
		new(&m_storage) std::exception_ptr(v.exception());
	}
	else
	{
		m_kind = kind::value;
		new(&m_storage) T(std::move(v.value()));
	}
}

template <typename T>
template <typename U>
aw::task<T>::task(result<U> const & v)
{
	if (v.has_exception())
	{
		m_kind = kind::exception;
		new(&m_storage) std::exception_ptr(v.exception());
	}
	else
	{
		m_kind = kind::value;
		new(&m_storage) T(v.value());
	}
}

template <typename T>
void aw::task<T>::clear()
{
	switch (m_kind)
	{
	case kind::empty:
		break;
	case kind::exception:
		reinterpret_cast<std::exception_ptr &>(m_storage).~exception_ptr();
		break;
	case kind::value:
		reinterpret_cast<T &>(m_storage).~T();
		break;
	}

	m_kind = kind::empty;
}

template <typename T>
bool aw::task<T>::empty() const
{
	return m_kind == kind::empty;
}
