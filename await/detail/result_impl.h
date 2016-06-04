#include <assert.h>
#include <new>
#include <utility>

namespace aw {
namespace detail {

template <typename T, typename... U>
void construct_result(result_kind & kind, void * storage, U &&... u) noexcept;

}
}

template <typename T, typename... U>
void aw::detail::construct_result(result_kind & kind, void * storage, U &&... u) noexcept
{
	try
	{
		new(storage) T(std::forward<U>(u)...);
		kind = result_kind::value;
	}
	catch (...)
	{
		new(storage) std::exception_ptr(std::current_exception());
		kind = result_kind::exception;
	}
}

template <typename T>
aw::result<T>::result() noexcept
	: m_kind(detail::result_kind::exception)
{
	new(&m_storage) std::exception_ptr(nullptr);
}

template <typename T>
aw::result<T>::result(result const & o) noexcept
{
	switch (o.m_kind)
	{
	case detail::result_kind::value:
		detail::construct_result<T>(m_kind, &m_storage, reinterpret_cast<T const &>(o.m_storage));
		break;
	case detail::result_kind::exception:
		new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr const &>(o.m_storage));
		m_kind = detail::result_kind::exception;
		break;
	}
}

template <typename T>
aw::result<T>::result(result && o) noexcept
	: m_kind(o.m_kind)
{
	switch (o.m_kind)
	{
	case detail::result_kind::value:
		detail::construct_result<T>(m_kind, &m_storage, reinterpret_cast<T &&>(o.m_storage));
		break;
	case detail::result_kind::exception:
		new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr &&>(o.m_storage));
		break;
	}
}

template <typename T>
template <typename... U>
aw::result<T>::result(in_place_t, U &&... u) noexcept
{
	detail::construct_result<T>(m_kind, &m_storage, std::forward<U>(u)...);
}


template <typename T>
aw::result<T>::result(std::exception_ptr e) noexcept
	: m_kind(detail::result_kind::exception)
{
	assert(e != nullptr);
	new(&m_storage) std::exception_ptr(std::move(e));
}

template <typename T>
aw::result<T>::~result()
{
	switch (m_kind)
	{
	case detail::result_kind::value:
		reinterpret_cast<T &>(m_storage).~T();
		break;
	case detail::result_kind::exception:
		reinterpret_cast<std::exception_ptr &>(m_storage).~exception_ptr();
		break;
	}
}

template <typename T>
template <typename U>
aw::result<T>::result(result<U> const & o) noexcept
{
	if (o.has_value())
	{
		m_kind = detail::result_kind::value;
		detail::construct_result<T>(m_kind, &m_storage, o.value());
	}
	else
	{
		m_kind = detail::result_kind::exception;
		new(&m_storage) std::exception_ptr(o.exception());
	}
}

template <typename T>
template <typename U>
aw::result<T>::result(result<U> && o) noexcept
{
	if (o.has_value())
	{
		m_kind = detail::result_kind::value;
		detail::construct_result<T>(m_kind, &m_storage, std::move(o.value()));
	}
	else
	{
		m_kind = detail::result_kind::exception;
		new(&m_storage) std::exception_ptr(std::move(o.exception()));
	}
}

template <typename T>
aw::result<T> & aw::result<T>::operator=(result o) noexcept
{
	switch (m_kind)
	{
	case detail::result_kind::value:
		reinterpret_cast<T &>(m_storage).~T();
		break;
	case detail::result_kind::exception:
		reinterpret_cast<std::exception_ptr &>(m_storage).~exception_ptr();
		break;
	}

	switch (o.m_kind)
	{
	case detail::result_kind::value:
		m_kind = detail::result_kind::value;
		detail::construct_result<T>(m_kind, &m_storage, reinterpret_cast<T &&>(o.m_storage));
		break;
	case detail::result_kind::exception:
		m_kind = detail::result_kind::exception;
		new(&m_storage) std::exception_ptr(reinterpret_cast<std::exception_ptr &&>(o.m_storage));
		break;
	}

	return *this;
}

template <typename T>
bool aw::result<T>::has_value() const noexcept
{
	return m_kind == detail::result_kind::value;
}

template <typename T>
bool aw::result<T>::has_exception() const noexcept
{
	return m_kind == detail::result_kind::exception;
}

template <typename T>
T & aw::result<T>::value() noexcept
{
	assert(m_kind == detail::result_kind::value);
	return reinterpret_cast<T &>(m_storage);
}

template <typename T>
T const & aw::result<T>::value() const noexcept
{
	assert(m_kind == detail::result_kind::value);
	return reinterpret_cast<T const &>(m_storage);
}

template <typename T>
std::exception_ptr const & aw::result<T>::exception() const noexcept
{
	assert(m_kind == detail::result_kind::exception);
	return reinterpret_cast<std::exception_ptr const &>(m_storage);
}

template <typename T>
std::exception_ptr & aw::result<T>::exception() noexcept
{
	assert(m_kind == detail::result_kind::exception);
	return reinterpret_cast<std::exception_ptr &>(m_storage);
}

template <typename T>
T && aw::result<T>::get()
{
	this->rethrow();
	return std::move(this->value());
}

template <typename T>
void aw::result<T>::rethrow() const
{
	if (m_kind == detail::result_kind::exception)
	{
		auto & ep = reinterpret_cast<std::exception_ptr const &>(m_storage);
		assert(ep != nullptr);
		std::rethrow_exception(ep);
	}
}

template <typename T>
aw::result<typename std::remove_const<typename std::remove_reference<T>::type>::type> aw::value(T && v) noexcept
{
	return result<typename std::remove_const<typename std::remove_reference<T>::type>::type>(in_place, std::forward<T>(v));
}
