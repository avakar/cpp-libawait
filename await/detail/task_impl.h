#include <assert.h>
#include <new>
#include <tuple>

template <typename T>
aw::detail::task_vtable<T> const * aw::detail::task_access::get_vtable(task<T> & t)
{
	return t.m_vtable;
}

template <typename T>
void aw::detail::task_access::set_vtable(task<T> & t, typename identity<task_vtable<T> const *>::type vtable)
{
	t.m_vtable = vtable;
}

template <typename T>
void * aw::detail::task_access::storage(task<T> & t)
{
	return &t.m_storage;
}

template <typename T>
size_t aw::detail::task_access::storage_size()
{
	return sizeof(task<T>::m_storage);
}

template <typename T>
aw::task<T>::task()
	: m_vtable(nullptr)
{
}

template <typename T>
aw::task<T>::task(task && o)
	: m_vtable(o.m_vtable)
{
	if (m_vtable)
	{
		m_vtable->move_to(&o.m_storage, &m_storage);
		o.m_vtable = nullptr;
	}
}

template <typename T>
aw::task<T> & aw::task<T>::operator=(task && o)
{
	this->clear();

	m_vtable = o.m_vtable;
	if (m_vtable)
	{
		m_vtable->move_to(&o.m_storage, &m_storage);
		o.m_vtable = nullptr;
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
	: m_vtable(nullptr)
{
}

template <typename T>
aw::task<T>::task(std::exception_ptr e)
{
	assert(e != nullptr);
	m_vtable = detail::construct_exception<T>(&m_storage, std::move(e));
}

template <typename T>
template <typename U>
aw::task<T>::task(result<U> && v)
{
	m_vtable = detail::construct_result<T>(&m_storage, std::move(v));
}

template <typename T>
template <typename U>
aw::task<T>::task(result<U> const & v)
{
	m_vtable = detail::construct_result<T>(&m_storage, v);
}

template <typename T>
void aw::task<T>::clear()
{
	if (m_vtable)
	{
		m_vtable->destroy(&m_storage);
		m_vtable = nullptr;
	}
}

template <typename T>
bool aw::task<T>::empty() const
{
	return m_vtable == nullptr;
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
			return std::move(r);
		}

		std::tuple<std::remove_const_t<std::remove_reference_t<P>>...> m_v;
	};

	return this->continue_with(X(std::forward<P>(p)...));
}
