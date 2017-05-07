namespace avakar {
namespace meta {

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

template <typename L, typename I, typename Visitor, typename... Args>
struct _visit_impl;

template <typename... Tn, size_t... In, typename Visitor, typename... Args>
struct _visit_impl<list<Tn...>, std::index_sequence<In...>, Visitor, Args...>
{
	using return_type = std::common_type_t<
		decltype(std::declval<Visitor>()(std::declval<list_item<Tn, In>>(), std::declval<Args>()...))...>;

	template <typename T, size_t I>
	static return_type visit_one(Visitor && visitor, Args &&... args)
	{
		return std::forward<Visitor>(visitor)(list_item<T, I>(), std::forward<Args>(args)...);
	}

	static return_type visit(Visitor && visitor, size_t index, Args &&... args)
	{
		using visit_fn = return_type(Visitor && visitor, Args &&... args);
		static visit_fn * const fns[] = { &visit_one<Tn, In>... };

		return fns[index](std::forward<Visitor>(visitor), std::forward<Args>(args)...);
	}
};

template <typename L, typename Visitor, typename... Args>
auto visit(size_t index, Visitor && visitor, Args &&... args)
{
	return _visit_impl<L, std::make_index_sequence<length<L>::value>, Visitor, Args...>
		::visit(std::forward<Visitor>(visitor), index, std::forward<Args>(args)...);
}

}
}
