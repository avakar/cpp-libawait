template <typename T>
aw::detail::command_ptr<T>::command_ptr() noexcept
	: m_ptr(nullptr)
{
}

template <typename T>
aw::detail::command_ptr<T>::command_ptr(std::nullptr_t) noexcept
	: m_ptr(nullptr)
{
}

template <typename T>
aw::detail::command_ptr<T>::command_ptr(command<T> * p) noexcept
	: m_ptr(p)
{
}

template <typename T>
aw::detail::command_ptr<T>::command_ptr(command_ptr && o) noexcept
	: m_ptr(o.m_ptr)
{
	o.m_ptr = nullptr;
}

template <typename T>
aw::detail::command_ptr<T>::~command_ptr()
{
	if (!this->empty())
		this->dismiss();
}

template <typename T>
aw::detail::command_ptr<T> & aw::detail::command_ptr<T>::operator=(command_ptr o) noexcept
{
	std::swap(m_ptr, o.m_ptr);
	return *this;
}

template <typename T>
bool aw::detail::command_ptr<T>::empty() const noexcept
{
	return m_ptr == nullptr;
}

template <typename T>
aw::detail::command_ptr<T>::operator bool() const noexcept
{
	return m_ptr != nullptr;
}

template <typename T>
bool aw::detail::command_ptr<T>::operator!() const noexcept
{
	return m_ptr == nullptr;
}

template <typename T>
aw::detail::command<T> & aw::detail::command_ptr<T>::operator*() const noexcept
{
	return *m_ptr;
}

template <typename T>
aw::detail::command<T> * aw::detail::command_ptr<T>::operator->() const noexcept
{
	return m_ptr;
}

template <typename T>
aw::detail::command<T> * aw::detail::command_ptr<T>::get() noexcept
{
	return m_ptr;
}

template <typename T>
aw::detail::command<T> * aw::detail::command_ptr<T>::release() noexcept
{
	command<T> * res = m_ptr;
	m_ptr = nullptr;
	return res;
}

template <typename T>
aw::result<T> aw::detail::command_ptr<T>::dismiss() noexcept
{
	return this->dismiss(std::make_exception_ptr(task_aborted()));
}

template <typename T>
aw::result<T> aw::detail::command_ptr<T>::dismiss(cancel_info ci) noexcept
{
	assert(m_ptr != nullptr);

	result<T> r = m_ptr->dismiss(ci);
	this->complete();
	return r;
}

template <typename T>
aw::task<T> aw::detail::command_ptr<T>::start(scheduler & sch, task_completion<T> & sink) noexcept
{
	assert(m_ptr != nullptr);

	for (;;)
	{
		task<T> t = m_ptr->start(sch, sink);
		if (t.empty())
			return nullptr;

		this->complete();
		*this = detail::fetch_command(t);

		if (m_ptr == nullptr)
			return std::move(t);
	}
}

template <typename T>
aw::result<T> aw::detail::command_ptr<T>::cancel(scheduler & sch, cancel_info ci) noexcept
{
	assert(m_ptr != nullptr);

	result<T> r = m_ptr->cancel(sch, ci);
	this->complete();
	return r;
}

template <typename T>
void aw::detail::command_ptr<T>::complete() noexcept
{
	delete m_ptr;
	m_ptr = nullptr;
}

template <typename T>
void aw::detail::command_ptr<T>::complete(task<T> & t) noexcept
{
	delete m_ptr;
	m_ptr = nullptr;
	*this = detail::fetch_command(t);
}
