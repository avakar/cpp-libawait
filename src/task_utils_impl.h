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

template <typename T, typename U>
aw::detail::task_kind aw::detail::construct_result(void * storage, result<U> && v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(std::move(v.exception()));
		return detail::task_kind::exception;
	}

	try
	{
		new(storage) T(std::move(v.value()));
		return detail::task_kind::value;
	}
	catch (...)
	{
		new(storage) std::exception_ptr(std::current_exception());
		return detail::task_kind::exception;
	}
}

template <typename T, typename U>
aw::detail::task_kind aw::detail::construct_result(void * storage, result<U> const & v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(v.exception());
		return detail::task_kind::exception;
	}

	try
	{
		new(storage) T(v.value());
		return detail::task_kind::value;
	}
	catch (...)
	{
		new(storage) std::exception_ptr(std::current_exception());
		return detail::task_kind::exception;
	}
}

template <>
inline aw::detail::task_kind aw::detail::construct_result<void, void>(void * storage, result<void> && v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(std::move(v.exception()));
		return detail::task_kind::exception;
	}

	return detail::task_kind::value;
}

template <>
inline aw::detail::task_kind aw::detail::construct_result<void, void>(void * storage, result<void> const & v) noexcept
{
	if (v.has_exception())
	{
		new(storage) std::exception_ptr(v.exception());
		return detail::task_kind::exception;
	}

	return detail::task_kind::value;
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
		dst.dismiss();

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
aw::task<T> aw::detail::from_command(command<T> * p)
{
	aw::task<T> task;
	new(task_access::storage(task)) command<T> *(p);
	task_access::set_kind(task, task_kind::command);
	return task;
}

template <typename T>
aw::task<T> aw::detail::from_command(command_ptr<T> & t)
{
	aw::task<T> task;
	new(task_access::storage(task)) command<T> *(t.release());
	task_access::set_kind(task, task_kind::command);
	return task;
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
	assert(cmd);
	aw::task<T> u = cmd.start(sch, sink);
	if (u.empty())
		return nullptr;

	return f(u.dismiss());
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
