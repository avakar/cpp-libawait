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
aw::task<T>::~task()
{
}

template <typename T>
aw::task<T>::task(result<T> && v)
{
	if (v.has_exception())
	{
		m_kind = kind::exception;
		new(&m_storage) std::exception_ptr(v.exception());
	}
	else
	{
		m_kind = kind::value;
		new(&m_storage) T(std::move(v.get()));
	}
}

template <typename T>
aw::task<T>::task(result<T> const & v)
{
	if (v.has_exception())
	{
		m_kind = kind::exception;
		new(&m_storage) std::exception_ptr(v.exception());
	}
	else
	{
		m_kind = kind::value;
		new(&m_storage) T(v.get());
	}
}

template <typename T>
bool aw::task<T>::empty() const
{
	return m_kind == kind::empty;
}

template <typename T>
T aw::run(task<T> && t)
{
	assert(!t.empty());
	return try_run(std::move(t)).get();
}

template <typename T>
aw::result<T> aw::try_run(task<T> && t)
{
	detail::task_kind kind = detail::task_access::get_kind(t);
	assert(kind != detail::task_kind::empty);

	if (kind == detail::task_kind::exception)
		return aw::result<T>(std::move(detail::task_access::as_exception(t)));

	assert(kind == detail::task_kind::value);
	return aw::result<T>::from_value(std::move(detail::task_access::as_value(t)));
}
