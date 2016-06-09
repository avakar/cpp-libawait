#include <assert.h>
#include <new>
#include <tuple>

namespace aw {
namespace detail {

template <typename R>
struct invoke_and_taskify_impl;

template <>
struct invoke_and_taskify_impl<void>;

}
}

template <typename T>
auto aw::detail::task_traits<T>::taskify(T && v) noexcept
	-> task_type
{
	return aw::value(std::move(v));
}

template <typename T>
auto aw::detail::task_traits<aw::result<T>>::taskify(result<T> && v) noexcept
	-> task_type
{
	return std::move(v);
}

template <typename T>
auto aw::detail::task_traits<aw::task<T>>::taskify(task<T> && v) noexcept
	-> task_type &&
{
	return std::move(v);
}

template <typename R>
struct aw::detail::invoke_and_taskify_impl
{
	typedef typename task_traits<R>::task_type task_type;

	template <typename F, typename... P>
	static auto invoke_and_taskify(F && f, P &&... p) -> task_type
	{
		auto && r = f(std::forward<P>(p)...);
		return task_traits<R>::taskify(std::move(r));
	}
};

template <>
struct aw::detail::invoke_and_taskify_impl<void>
{
	typedef task<void> task_type;

	template <typename F, typename... P>
	static auto invoke_and_taskify(F && f, P &&... p) -> task_type
	{
		f(std::forward<P>(p)...);
		return aw::value();
	}
};

template <typename F, typename... P>
auto aw::detail::invoke_and_taskify(F && f, P &&... p) noexcept -> typename invoke_and_taskify_traits<F, P...>::task_type
{
	typedef typename std::result_of<F(P &&...)>::type R;
	try
	{
		return invoke_and_taskify_impl<R>::invoke_and_taskify(std::move(f), std::forward<P>(p)...);
	}
	catch (...)
	{
		return std::current_exception();
	}
}

template <typename T>
aw::detail::task_vtable<T> const * aw::detail::task_access::get_vtable(task<T> const & t)
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
void const * aw::detail::task_access::storage(task<T> const & t)
{
	return &t.m_storage;
}

template <typename T>
constexpr size_t aw::detail::task_access::storage_size()
{
	return sizeof(task<T>::m_storage);
}

template <typename T>
bool aw::detail::has_exception(task<T> const & t)
{
	return has_result(t) && get_result(t).has_exception();
}

template <typename T>
std::exception_ptr aw::detail::fetch_exception(task<T> & t)
{
	assert(has_exception(t));
	return fetch_result(t).exception();
}

template <typename T>
bool aw::detail::has_result(task<T> const & t)
{
	auto vtable = task_access::get_vtable(t);
	return vtable != nullptr && vtable->start == nullptr;
}

template <typename T>
aw::result<T> const & aw::detail::get_result(task<T> const & t)
{
	assert(has_result(t));
	return *static_cast<result<T> const *>(task_access::storage(t));
}

template <typename T>
aw::result<T> & aw::detail::get_result(task<T> & t)
{
	assert(has_result(t));
	return *static_cast<result<T> *>(task_access::storage(t));
}

template <typename T>
aw::result<T> aw::detail::fetch_result(task<T> & t)
{
	aw::result<T> r = std::move(get_result(t));
	t.clear();
	return r;
}

template <typename T>
bool aw::detail::has_command(task<T> const & t)
{
	auto vtable = task_access::get_vtable(t);
	return vtable != nullptr && vtable->start != nullptr;
}

template <typename T>
bool aw::detail::start_command(task<T> & t, scheduler & sch, task_completion<T> & sink)
{
	assert(!t.empty());

	task_vtable<T> const * vtable = task_access::get_vtable(t);
	void * storage = task_access::storage(t);

	for (;;)
	{
		if (vtable->start == nullptr)
			return false;

		task<T> u = vtable->start(storage, sch, sink);
		if (u.empty())
			return true;

		t = std::move(u);
		vtable = detail::task_access::get_vtable(t);
	}
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
			return std::move(r);
		}

		std::tuple<typename std::remove_const<typename std::remove_reference<P>::type>::type...> m_v;
	};

	return this->continue_with(X(std::forward<P>(p)...));
}
