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
	return v;
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
aw::detail::task_kind aw::detail::task_access::get_kind(task<T> const & t)
{
	return t.m_kind;
}

template <typename T>
void aw::detail::task_access::set_kind(task<T> & t, task_kind kind)
{
	t.m_kind = kind;
}

template <typename T>
void * aw::detail::task_access::storage(task<T> & t)
{
	return &t.m_storage;
}

template <typename T>
void aw::detail::move_value(void * dst, void * src)
{
	T * p = static_cast<T *>(src);
	new(dst) T(std::move(*p));
	p->~T();
}

template <>
inline void aw::detail::move_value<void>(void * dst, void * src)
{
	(void)src;
	(void)dst;
}

template <typename T>
void aw::detail::destroy_value(void * p)
{
	static_cast<T *>(p)->~T();
}

template <>
inline void aw::detail::destroy_value<void>(void * p)
{
	(void)p;
}

template <typename T>
aw::result<T> aw::detail::dismiss_value(void * p)
{
	T * pp = static_cast<T *>(p);
	result<T> r(in_place, std::move(*pp));
	pp->~T();
	return r;
}

template <>
inline aw::result<void> aw::detail::dismiss_value<void>(void * p)
{
	(void)p;
	return result<void>(in_place);
}

template <typename T>
void aw::detail::move_task(task<T> & dst, task<T> & src)
{
	if (!dst.empty())
		dismiss_task(dst);

	task_kind kind = task_access::get_kind(src);
	task_access::set_kind(dst, kind);
	task_access::set_kind(src, task_kind::empty);
	void * dst_storage = task_access::storage(dst);
	void * src_storage = task_access::storage(src);

	if (kind == task_kind::value)
	{
		move_value<T>(dst_storage, src_storage);
	}
	else if (kind == task_kind::exception)
	{
		auto p = static_cast<std::exception_ptr *>(src_storage);
		new(dst_storage) std::exception_ptr(std::move(*p));
		p->~exception_ptr();
	}
	else if (kind == task_kind::command)
	{
		auto p = static_cast<command<T> **>(src_storage);
		new(dst_storage) command<T> *(std::move(*p));
	}
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
bool aw::detail::has_command(task<T> const & t)
{
	return task_access::get_kind(t) == task_kind::command;
}

template <typename T>
aw::detail::command_ptr<T> aw::detail::fetch_command(task<T> & t)
{
	if (task_access::get_kind(t) == task_kind::command)
	{
		auto * cmd = *static_cast<command<T> **>(task_access::storage(t));
		task_access::set_kind(t, task_kind::empty);
		return aw::detail::command_ptr<T>(cmd);
	}

	return nullptr;
}

template <typename T>
bool aw::detail::start_command(task<T> & t, scheduler & sch, task_completion<T> & sink)
{
	assert(!t.empty());

	void * storage = task_access::storage(t);

	while (task_access::get_kind(t) == task_kind::command)
	{
		auto cmd = *static_cast<command<T> **>(storage);

		task<T> u = cmd->start(sch, sink);
		if (u.empty())
			return true;

		mark_complete(t);
		t = std::move(u);
	}

	return false;
}

template <typename T, typename F>
auto aw::detail::start_command(command_ptr<T> & cmd, scheduler & sch, task_completion<T> & sink, F && f)
	-> typename task_traits<decltype(f(std::declval<result<T>>()))>::task_type
{
	typedef typename aw::detail::task_traits<decltype(f(std::declval<aw::result<T>>()))>::value_type U;

	assert(cmd);
	for (;;)
	{
		aw::task<T> u = cmd->start(sch, sink);
		if (u.empty())
			return nullptr;

		delete cmd.release();
		cmd = aw::detail::fetch_command(u);

		if (!cmd)
			return f(aw::detail::dismiss_task(u));
	}
}

template <typename T>
aw::result<T> aw::detail::dismiss(command_ptr<T> & t)
{
	result<T> r = t->dismiss();
	t.reset();
	return r;
}

template <typename T>
aw::result<T> aw::detail::dismiss_task(task<T> & t)
{
	assert(!t.empty());

	task_kind kind = task_access::get_kind(t);
	task_access::set_kind(t, task_kind::empty);
	void * storage = task_access::storage(t);

	if (kind == task_kind::value)
		return dismiss_value<T>(storage);

	if (kind == task_kind::exception)
	{
		auto p = static_cast<std::exception_ptr *>(storage);
		result<T> r(std::move(*p));
		p->~exception_ptr();
		return r;
	}

	auto cmd = static_cast<command<T> **>(storage);
	result<T> r = (*cmd)->dismiss();
	delete *cmd;
	return r;
}

template <typename T>
void aw::detail::mark_complete(task<T> & t)
{
	assert(!t.empty());

	task_kind kind = task_access::get_kind(t);
	task_access::set_kind(t, task_kind::empty);
	void * storage = task_access::storage(t);

	if (kind == task_kind::value)
	{
		destroy_value<T>(storage);
	}
	else if (kind == task_kind::exception)
	{
		auto p = static_cast<std::exception_ptr *>(storage);
		p->~exception_ptr();
	}
	else
	{
		auto cmd = static_cast<command<T> **>(storage);
		delete *cmd;
	}
}

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
