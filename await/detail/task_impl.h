#include "command_ptr_fwd.h"
#include "task_utils_fwd.h"

#include "command_ptr_impl.h"
#include "task_utils_impl.h"
#include "task_traits_impl.h"

#include <assert.h>
#include <new>
#include <tuple>


template <typename T>
aw::task<T>::task()
	: m_kind(detail::task_kind::empty)
{
}

template <typename T>
aw::task<T>::task(task && o)
	: m_kind(detail::task_kind::empty)
{
	detail::move_task(*this, o);
}

template <typename T>
aw::task<T> & aw::task<T>::operator=(task && o)
{
	detail::move_task(*this, o);
	return *this;
}

template <typename T>
aw::task<T>::~task()
{
	this->clear();
}

template <typename T>
aw::task<T>::task(std::nullptr_t)
	: m_kind(detail::task_kind::empty)
{
}

template <typename T>
aw::task<T>::task(std::exception_ptr e)
	: m_kind(detail::task_kind::exception)
{
	assert(e != nullptr);
	new(&m_storage) std::exception_ptr(std::move(e));
}

template <typename T>
template <typename U>
aw::task<T>::task(result<U> && v)
{
	m_kind = detail::construct_result<T>(&m_storage, std::move(v));
}

template <typename T>
template <typename U>
aw::task<T>::task(result<U> const & v)
{
	m_kind = detail::construct_result<T>(&m_storage, v);
}

template <typename T>
void aw::task<T>::clear()
{
	if (m_kind != detail::task_kind::empty)
		detail::dismiss_task(*this);
}

template <typename T>
bool aw::task<T>::empty() const
{
	return m_kind == detail::task_kind::empty;
}

template <typename T>
aw::task<T>::operator bool() const
{
	return !this->empty();
}

template <typename T>
aw::task<void> aw::task<T>::ignore_result()
{
	return this->then([](T &&) { return aw::value(); });
}

template <typename T>
template <typename... P>
aw::task<T> aw::task<T>::hold(P &&... p)
{
	struct X
	{
		X(P &&... v)
			: m_v(std::forward<P>(v)...)
		{
		}

		task<T> operator()(result<T> && r)
		{
			return r;
		}

		std::tuple<typename std::remove_const<typename std::remove_reference<P>::type>::type...> m_v;
	};

	return this->continue_with(X(std::forward<P>(p)...));
}
