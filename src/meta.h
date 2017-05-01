#ifndef AVAKAR_LIBAWAIT_META_H
#define AVAKAR_LIBAWAIT_META_H

#include <stdlib.h>
#include <type_traits>

namespace avakar {
namespace libawait {
namespace _meta {

template <typename... Tn>
struct list;

template <typename L>
struct length;

template <typename T, typename L>
struct index_of;

template <typename L, size_t I>
struct sub;

template <typename L, size_t I>
using sub_t = typename sub<L, I>::type;

template <typename... Ln>
struct concat;

template <typename... Ln>
using concat_t = typename concat<Ln...>::type;

template <typename... L>
struct overload_sandbox;

template <typename T, typename L>
using choose_overload_t = decltype(overload_sandbox<L>::f(std::declval<T>()));



template <typename... Tn>
struct length<list<Tn...>>
	: std::integral_constant<size_t, sizeof...(Tn)>
{
};

constexpr size_t npos = -1;

template <typename T>
struct index_of<T, list<>>
	: std::integral_constant<size_t, npos>
{
};

template <typename T, typename... Tn>
struct index_of<T, list<T, Tn...>>
	: std::integral_constant<size_t, 0>
{
};

template <typename T, typename T0, typename... Tn>
struct index_of<T, list<T0, Tn...>>
	: std::integral_constant<size_t, 1 + index_of<T, list<Tn...>>::value>
{
};

template <typename T0, typename... Tn>
struct sub<list<T0, Tn...>, 0>
{
	using type = T0;
};

template <typename T0, typename... Tn, size_t I>
struct sub<list<T0, Tn...>, I>
{
	using type = sub_t<list<Tn...>, I - 1>;
};

template <>
struct concat<>
{
	using type = list<>;
};

template <typename T>
struct concat<T>
{
	using type = list<T>;
};

template <typename... Tn>
struct concat<list<Tn...>>
{
	using type = list<Tn...>;
};

template <typename... T1n, typename... T2n>
struct concat<list<T1n...>, list<T2n...>>
{
	using type = list<T1n..., T2n...>;
};

template <typename T1, typename... T2n>
struct concat<T1, list<T2n...>>
{
	using type = list<T1, T2n...>;
};

template <typename... T1n, typename T2>
struct concat<list<T1n...>, T2>
{
	using type = list<T1n..., T2>;
};

template <typename T1, typename T2>
struct concat<T1, T2>
{
	using type = list<T1, T2>;
};

template <typename L1, typename L2, typename... Ln>
struct concat<L1, L2, Ln...>
{
	using type = concat_t<concat_t<L1, L2>, Ln...>;
};

template <typename T0, typename... Tn>
struct overload_sandbox<list<T0, Tn...>>
	: overload_sandbox<list<Tn...>>
{
	using overload_sandbox<list<Tn...>>::f;
	static T0 f(T0);
};

template <typename T0>
struct overload_sandbox<list<T0>>
{
	static T0 f(T0);
};

template <typename... Tn>
struct overload_sandbox<list<void, Tn...>>
	: overload_sandbox<list<Tn...>>
{
};

}
}
}

#endif // AVAKAR_LIBAWAIT_META_H
