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
aw::result<T> aw::detail::command_ptr<T>::dismiss() noexcept
{
	assert(m_ptr != nullptr);

	result<T> r = m_ptr->dismiss();
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
aw::task<T> aw::detail::command_ptr<T>::cancel(scheduler & sch) noexcept
{
	assert(m_ptr != nullptr);

	task<T> t = m_ptr->cancel(sch);
	if (t.empty())
		return nullptr;

	this->complete();
	return t.dismiss();
}

template <typename T>
void aw::detail::command_ptr<T>::complete() noexcept
{
	delete m_ptr;
	m_ptr = nullptr;
}
