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
auto aw::detail::invoke(F && f, P &&... p) noexcept -> typename invoke_and_taskify_traits<F, P...>::task_type
{
	typedef typename std::result_of<F(P &&...)>::type R;
	try
	{
		return invoke_and_taskify_impl<R>::invoke_and_taskify(std::forward<F>(f), std::forward<P>(p)...);
	}
	catch (...)
	{
		return std::current_exception();
	}
}
