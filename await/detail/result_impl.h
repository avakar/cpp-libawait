#include <assert.h>
#include <new>
#include <utility>

template <typename T>
aw::result<T>::result(result const & o)
	: m_kind(o.m_kind)
{
	switch (o.m_kind)
	{
	case kind_t::value:
		new(&m_storage) T(reinterpret_cast<T const &>(o.m_storage));
		break;
	case kind_t::exception:
		new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr const &>(o.m_storage));
		break;
	}
}

template <typename T>
aw::result<T>::result(result && o)
	: m_kind(o.m_kind)
{
	switch (o.m_kind)
	{
	case kind_t::value:
		new(&m_storage) T(reinterpret_cast<T &&>(o.m_storage));
		break;
	case kind_t::exception:
		new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr &&>(o.m_storage));
		break;
	}
}

template <typename T>
aw::result<T>::result(std::exception_ptr e)
	: m_kind(kind_t::exception)
{
	assert(e != nullptr);
	new(&m_storage) std::exception_ptr(std::move(e));
}

template <typename T>
aw::result<T>::~result()
{
	switch (m_kind)
	{
	case kind_t::value:
		reinterpret_cast<T &>(m_storage).~T();
		break;
	case kind_t::exception:
		reinterpret_cast<std::exception_ptr &>(m_storage).~exception_ptr();
		break;
	}
}

template <typename T>
template <typename U>
aw::result<T>::result(result<U> const & o)
{
	try
	{
		switch (o.m_kind)
		{
		case result<U>::kind_t::value:
			m_kind = kind_t::value;
			new(&m_storage) T(reinterpret_cast<U const &>(o.m_storage));
			break;
		case result<U>::kind_t::exception:
			m_kind = kind_t::exception;
			new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr const &>(o.m_storage));
			break;
		}
	}
	catch (...)
	{
		m_kind = kind_t::exception;
		new(&m_storage) std::exception_ptr(std::current_exception());
	}
}

template <typename T>
template <typename U>
aw::result<T>::result(result<U> && o)
{
	try
	{
		switch (o.m_kind)
		{
		case result<U>::kind_t::value:
			m_kind = kind_t::value;
			new(&m_storage) T(reinterpret_cast<U &&>(o.m_storage));
			break;
		case result<U>::kind_t::exception:
			m_kind = kind_t::exception;
			new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr &&>(o.m_storage));
			break;
		}
	}
	catch (...)
	{
		m_kind = kind_t::exception;
		new(&m_storage) std::exception_ptr(std::current_exception());
	}
}

template <typename T>
aw::result<T> & aw::result<T>::operator=(result o)
{
	switch (m_kind)
	{
	case kind_t::value:
		reinterpret_cast<T &>(m_storage).~T();
		break;
	case kind_t::exception:
		reinterpret_cast<std::exception_ptr &>(m_storage).~exception_ptr();
		break;
	}

	try
	{
		switch (o.m_kind)
		{
		case result<T>::kind_t::value:
			m_kind = kind_t::value;
			new(&m_storage) T(reinterpret_cast<T &&>(o.m_storage));
			break;
		case result<T>::kind_t::exception:
			m_kind = kind_t::exception;
			new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr &&>(o.m_storage));
			break;
		}
	}
	catch (...)
	{
		m_kind = kind_t::exception;
		new(&m_storage) std::exception_ptr(std::current_exception());
	}

	return *this;
}

template <typename T>
bool aw::result<T>::has_value() const
{
	return m_kind == kind_t::value;
}

template <typename T>
bool aw::result<T>::has_exception() const
{
	return m_kind == kind_t::exception;
}

template <typename T>
T aw::result<T>::get()
{
	this->rethrow();
	return std::move(this->value());
}

template <typename T>
T & aw::result<T>::value()
{
	assert(m_kind == kind_t::value);
	return reinterpret_cast<T &>(m_storage);
}

template <typename T>
T const & aw::result<T>::value() const
{
	assert(m_kind == kind_t::value);
	return reinterpret_cast<T const &>(m_storage);
}

template <typename T>
std::exception_ptr aw::result<T>::exception() const
{
	if (m_kind != kind_t::exception)
		return nullptr;
	return reinterpret_cast<std::exception_ptr const &>(m_storage);
}

template <typename T>
void aw::result<T>::rethrow() const
{
	if (m_kind == kind_t::exception)
	{
		auto & ep = reinterpret_cast<std::exception_ptr const &>(m_storage);
		assert(ep != nullptr);
		std::rethrow_exception(ep);
	}
}

template <typename T>
template <typename U>
aw::result<T> aw::result<T>::from_value(U && v)
{
	aw::result<T> r((empty_t()));
	try
	{
		r.m_kind = kind_t::value;
		new (&r.m_storage) T(std::forward<U>(v));
	}
	catch (...)
	{
		r.m_kind = kind_t::exception;
		new (&r.m_storage) std::exception_ptr(std::current_exception());
	}
	return std::move(r);
}

template <typename T>
aw::result<T>::result()
	: m_kind(kind_t::exception)
{
	new(&m_storage) std::exception_ptr(std::make_exception_ptr(no_result_error()));
}

template <typename T>
aw::result<T>::result(empty_t)
{
}

template <typename T>
aw::result<typename std::remove_const<typename std::remove_reference<T>::type>::type> aw::value(T && v)
{
	return result<typename std::remove_const<typename std::remove_reference<T>::type>::type>::from_value(std::forward<T>(v));
}
