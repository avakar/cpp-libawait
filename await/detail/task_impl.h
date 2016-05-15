#include <assert.h>
#include <new>

template <typename T>
aw::detail::task_vtable<T> const * aw::detail::task_access::pull_vtable(task<T> & t)
{
	task_vtable<T> const * vtable = t.m_vtable;
	t.m_vtable = nullptr;
	return vtable;
}

template <typename T>
void * aw::detail::task_access::storage(task<T> & t)
{
	return &t.m_storage;
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
