#ifndef AVAKAR_LIBAWAIT_VARIANT_STORAGE_H
#define AVAKAR_LIBAWAIT_VARIANT_STORAGE_H

#include <type_traits>

namespace avakar {
namespace libawait {
namespace detail {

template <typename... Tn>
struct list;

template <typename L>
struct length;

template <typename... Tn>
struct length<list<Tn...>>
	: std::integral_constant<size_t, sizeof...(Tn)>
{
};

template <typename T, typename L>
struct index_of;

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

template <typename L, size_t I>
struct sub;

template <typename L, size_t I>
using sub_t = typename sub<L, I>::type;

template <typename T0, typename... Tn>
struct sub<list<T0, Tn...>, 0>
{
	using type = T0;
};

template <typename T0, typename... Tn, size_t I>
struct sub<list<T0, Tn...>, I>
{
	using type = sub_t<list<Tn...>, I-1>;
};

template <typename... Ln>
struct concat;

template <typename... Ln>
using concat_t = typename concat<Ln...>::type;

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

template <typename... Types>
struct variant_storage_impl;

template <typename... Types>
struct variant_storage_impl<list<Types...>>
{
	using type = std::aligned_union_t<0, Types...>;
};

template <typename T>
struct variant_storage_member
{
	using type = T;
};

template <typename T>
using variant_storage_member_t = typename variant_storage_member<T>::type;

template <typename T>
struct variant_storage_member<T &>
{
	using type = T *;
};

template <typename L>
struct variant_storage_type_filter;

template <>
struct variant_storage_type_filter<list<>>
{
	using type = list<>;
};

template <typename T0, typename... Tn>
struct variant_storage_type_filter<list<T0, Tn...>>
{
	using _filtered_tail = typename variant_storage_type_filter<list<Tn...>>::type;

	using type = std::conditional_t<
		std::is_void<T0>::value,
		_filtered_tail,
		concat_t<variant_storage_member_t<T0>, _filtered_tail>>;
};

template <typename L>
using variant_storage_t = typename variant_storage_impl<typename variant_storage_type_filter<L>::type>::type;

template <typename T, size_t I = 0>
struct variant_member;

template <typename L, typename I, typename Visitor, typename... Args>
struct visit_impl;

template <typename... Tn, size_t... In, typename Visitor, typename... Args>
struct visit_impl<list<Tn...>, std::index_sequence<In...>, Visitor, Args...>
{
	template <typename T, size_t I>
	static void visit_one(Visitor && visitor, Args &&... args)
	{
		std::forward<Visitor>(visitor)(variant_member<T, I>(), std::forward<Args>(args)...);
	}

	static void visit(Visitor && visitor, size_t index, Args &&... args)
	{
		using visit_fn = void(Visitor && visitor, Args &&... args);
		static visit_fn * const fns[] = { &visit_one<Tn, In>... };

		fns[index](std::forward<Visitor>(visitor), std::forward<Args>(args)...);
	}
};

template <typename L, typename Visitor, typename... Args>
void variant_visit(size_t index, Visitor && visitor, Args &&... args)
{
	visit_impl<L, std::make_index_sequence<length<L>::value>, Visitor, Args...>
		::visit(std::forward<Visitor>(visitor), index, std::forward<Args>(args)...);
}

template <typename T, size_t I>
struct variant_member
{
	using type = T;
	static constexpr size_t index = I;

	template <typename... Args>
	static void construct(void * storage, Args &&... args)
	{
		new(storage) T(std::forward<Args>(args)...);
	}

	static void destruct(void * storage)
	{
		static_cast<T *>(storage)->~T();
	}

	template <typename U = T>
	static void copy(void * dst, void const * src)
	{
		new(dst) T(*static_cast<U const *>(src));
	}

	template <typename U = T>
	static void move(void * dst, void * src)
	{
		new(dst) T(std::move(*static_cast<U *>(src)));
	}
};

template <size_t I>
struct variant_member<void, I>
{
	using type = void;
	static constexpr size_t index = I;

	static void construct(void * storage)
	{
}

	static void destruct(void * storage)
	{
	}

	static void copy(void * dst, void const * src)
	{
	}

	static void move(void * dst, void * src)
	{
	}
};

template <typename T, size_t I>
struct variant_member<T &, I>
{
	using type = T &;
	static constexpr size_t index = I;

	static void construct(void * storage, T & arg)
	{
		new(storage) T *(std::addressof(arg));
	}

	static void destruct(void * storage)
	{
	}

	static void copy(void * dst, void const * src)
	{
		new(dst) T *(*static_cast<T**>(src));
	}

	static void move(void * dst, void * src)
	{
		copy(dst, src);
	}
};

}
}
}

#endif // AVAKAR_LIBAWAIT_VARIANT_STORAGE_H
