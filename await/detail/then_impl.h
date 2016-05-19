template <typename T, typename F>
typename aw::detail::then_traits<T, F>::type aw::detail::then_traits<T, F>::invoke(result<T> && t, F && f)
{
	return f(std::move(t.value()));
}

template <typename F>
typename aw::detail::then_traits<void, F>::type aw::detail::then_traits<void, F>::invoke(result<void> && t, F && f)
{
	return f();
}

template <typename T>
template <typename F>
typename aw::detail::then_traits<T, F>::type aw::task<T>::then(F && f)
{
	assert(m_vtable);

	aw::result<T> r = m_vtable->get_result(&m_storage);
	m_vtable = nullptr;
	if (r.has_exception())
		return r.exception();
	return detail::then_traits<T, F>::invoke(std::move(r), std::move(f));
}

template <typename T>
template <typename F>
decltype(std::declval<F>()(std::declval<aw::result<T>>())) aw::task<T>::continue_with(F && f)
{
	assert(m_vtable);

	aw::result<T> r = m_vtable->get_result(&m_storage);
	m_vtable = nullptr;
	return f(std::move(r));
}
