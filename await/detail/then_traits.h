#ifndef AWAIT_DETAIL_THEN_TRAITS_H
#define AWAIT_DETAIL_THEN_TRAITS_H

namespace aw {

namespace detail {

template <typename T, typename F>
struct then_traits
{
	typedef decltype(std::declval<F>()(std::declval<T>())) type;
	static type invoke(result<T> && t, F && f);
};

template <typename F>
struct then_traits<void, F>
{
	typedef decltype(std::declval<F>()()) type;
	static type invoke(result<void> && t, F && f);
};

}
}

#endif // AWAIT_DETAIL_THEN_TRAITS_H
