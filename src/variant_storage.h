#ifndef AVAKAR_LIBAWAIT_VARIANT_STORAGE_H
#define AVAKAR_LIBAWAIT_VARIANT_STORAGE_H

#include "../../../src/meta.h"
#include <type_traits>
#include <utility>

namespace avakar {
namespace libawait {
namespace detail {

template <typename... Types>
struct variant_storage_impl;

template <typename... Types>
struct variant_storage_impl<_meta::list<Types...>>
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
struct variant_storage_type_filter<_meta::list<>>
{
	using type = _meta::list<>;
};

template <typename T0, typename... Tn>
struct variant_storage_type_filter<_meta::list<T0, Tn...>>
{
	using _filtered_tail = typename variant_storage_type_filter<_meta::list<Tn...>>::type;

	using type = std::conditional_t<
		std::is_void<T0>::value,
		_filtered_tail,
		_meta::concat_t<variant_storage_member_t<T0>, _filtered_tail>>;
};

template <typename L>
using variant_storage_t = typename variant_storage_impl<typename variant_storage_type_filter<L>::type>::type;

template <typename T, size_t I = 0>
struct variant_member;

template <typename L, typename I, typename Visitor, typename... Args>
struct visit_impl;

template <typename... Tn, size_t... In, typename Visitor, typename... Args>
struct visit_impl<_meta::list<Tn...>, std::index_sequence<In...>, Visitor, Args...>
{
	using return_type = std::common_type_t<
		decltype(std::declval<Visitor>()(std::declval<variant_member<Tn, In>>(), std::declval<Args>()...))...>;

	template <typename T, size_t I>
	static return_type visit_one(Visitor && visitor, Args &&... args)
	{
		return std::forward<Visitor>(visitor)(variant_member<T, I>(), std::forward<Args>(args)...);
	}

	static return_type visit(Visitor && visitor, size_t index, Args &&... args)
	{
		using visit_fn = return_type(Visitor && visitor, Args &&... args);
		static visit_fn * const fns[] = { &visit_one<Tn, In>... };

		return fns[index](std::forward<Visitor>(visitor), std::forward<Args>(args)...);
	}
};

template <typename L, typename Visitor, typename... Args>
auto variant_visit(size_t index, Visitor && visitor, Args &&... args)
{
	return visit_impl<L, std::make_index_sequence<_meta::length<L>::value>, Visitor, Args...>
		::visit(std::forward<Visitor>(visitor), index, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto construct_member(void * storage, Args &&... args) noexcept
	-> std::enable_if_t<std::is_nothrow_constructible<T, Args...>::value, bool>
{
	new(storage) T(std::forward<Args>(args)...);
	return true;
}

template <typename T, typename... Args>
auto construct_member(void * storage, Args &&... args) noexcept
	-> std::enable_if_t<std::is_constructible<T, Args...>::value && !std::is_nothrow_constructible<T, Args...>::value, bool>
{
	try
	{
		new(storage) T(std::forward<Args>(args)...);
		return true;
	}
	catch (...)
	{
		new(storage) std::exception_ptr(std::current_exception());
		return false;
	}
}

template <typename T, size_t I>
struct variant_member
{
	using type = T;
	static constexpr size_t index = I;

	template <typename... Args>
	static bool construct(void * storage, Args &&... args) noexcept
	{
		return construct_member<T>(storage, std::forward<Args>(args)...);
	}

	static void destruct(void * storage) noexcept
	{
		static_cast<T *>(storage)->~T();
	}

	template <typename U = T>
	static bool copy(void * dst, void const * src) noexcept
	{
		return construct_member<T>(dst, *static_cast<U const *>(src));
	}

	template <typename U = T>
	static bool move(void * dst, void * src) noexcept
	{
		return construct_member<T>(dst, std::move(*static_cast<U *>(src)));
	}

	static bool equal(void const * lhs, void const * rhs) noexcept
	{
		return *static_cast<T const *>(lhs) == *static_cast<T const *>(rhs);
	}
};

template <size_t I>
struct variant_member<void, I>
{
	using type = void;
	static constexpr size_t index = I;

	static bool construct(void * storage) noexcept
	{
		return true;
	}

	static void destruct(void * storage) noexcept
	{
	}

	static bool copy(void * dst, void const * src) noexcept
	{
		return true;
	}

	static bool move(void * dst, void * src) noexcept
	{
		return true;
	}

	static bool equal(void const * lhs, void const * rhs) noexcept
	{
		return true;
	}
};

template <typename T, size_t I>
struct variant_member<T &, I>
{
	using type = T &;
	static constexpr size_t index = I;

	static bool construct(void * storage, T & arg) noexcept
	{
		new(storage) T *(std::addressof(arg));
		return true;
	}

	static void destruct(void * storage) noexcept
	{
	}

	static bool copy(void * dst, void const * src) noexcept
	{
		new(dst) T *(*static_cast<T**>(src));
		return true;
	}

	static bool move(void * dst, void * src) noexcept
	{
		return copy(dst, src);
	}

	static bool equal(void const * lhs, void const * rhs) noexcept
	{
		return **static_cast<T const **>(lhs) == **static_cast<T const **>(rhs);
	}
};

}
}
}

#endif // AVAKAR_LIBAWAIT_VARIANT_STORAGE_H
