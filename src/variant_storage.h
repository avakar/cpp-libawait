#ifndef AVAKAR_LIBAWAIT_VARIANT_STORAGE_H
#define AVAKAR_LIBAWAIT_VARIANT_STORAGE_H

#include <avakar/meta.h>
#include <type_traits>
#include <utility>

namespace avakar {
namespace libawait {
namespace detail {

template <typename... Types>
struct variant_storage_impl;

template <typename... Types>
struct variant_storage_impl<meta::list<Types...>>
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
struct variant_storage_type_filter<meta::list<>>
{
	using type = meta::list<>;
};

template <typename T0, typename... Tn>
struct variant_storage_type_filter<meta::list<T0, Tn...>>
{
	using _filtered_tail = typename variant_storage_type_filter<meta::list<Tn...>>::type;

	using type = std::conditional_t<
		std::is_void<T0>::value,
		_filtered_tail,
		meta::concat_t<variant_storage_member_t<T0>, _filtered_tail>>;
};

template <typename L>
using variant_storage_t = typename variant_storage_impl<typename variant_storage_type_filter<L>::type>::type;

template <typename... Args>
auto construct_member(meta::item<void>, void * st, Args &&... args) noexcept
{
	return true;
}

template <typename T, typename... Args>
auto construct_member(meta::item<T>, void * st, Args &&... args) noexcept
	-> std::enable_if_t<std::is_nothrow_constructible<T, Args...>::value, bool>
{
	new(st) T(std::forward<Args>(args)...);
	return true;
}

template <typename T, typename... Args>
auto construct_member(meta::item<T>, void * st, Args &&... args) noexcept
	-> std::enable_if_t<std::is_constructible<T, Args...>::value && !std::is_nothrow_constructible<T, Args...>::value, bool>
{
	try
	{
		new(st) T(std::forward<Args>(args)...);
		return true;
	}
	catch (...)
	{
		new(st) std::exception_ptr(std::current_exception());
		return false;
	}
}

template <typename T>
void destroy_member(meta::item<T>, void * st) noexcept
{
	static_cast<T *>(st)->~T();
}

inline void destroy_member(meta::item<void>, void * st) noexcept
{
}

template <typename T, typename U>
auto move_construct_member(meta::item<T>, void * lhs, meta::item<U>, void * rhs) noexcept
	-> std::enable_if_t<std::is_nothrow_constructible<T, U &&>::value, bool>
{
	new(lhs) T(std::move(*static_cast<U *>(rhs)));
	return true;
}

template <typename T, typename U>
auto move_construct_member(meta::item<T>, void * lhs, meta::item<U>, void * rhs) noexcept
	-> std::enable_if_t<!std::is_nothrow_constructible<T, U &&>::value, bool>
{
	try
	{
		new(lhs) T(std::move(*static_cast<U *>(rhs)));
		return true;
	}
	catch (...)
	{
		new(lhs) std::exception_ptr(std::current_exception());
		return false;
	}
}

inline bool move_construct_member(meta::item<void>, void *, meta::item<void>, void *) noexcept
{
	return true;
}

template <typename T, typename U>
auto copy_construct_member(meta::item<T>, void * lhs, meta::item<U>, void const * rhs) noexcept
	-> std::enable_if_t<std::is_nothrow_constructible<T, U &>::value, bool>
{
	new(lhs) T(*static_cast<U const *>(rhs));
	return true;
}

template <typename T, typename U>
auto copy_construct_member(meta::item<T>, void * lhs, meta::item<U>, void const * rhs) noexcept
	-> std::enable_if_t<!std::is_nothrow_constructible<T, U &>::value, bool>
{
	try
	{
		new(lhs) T(*static_cast<U const *>(rhs));
		return true;
	}
	catch (...)
	{
		new(lhs) std::exception_ptr(std::current_exception());
		return false;
	}
}

inline bool copy_construct_member(meta::item<void>, void *, meta::item<void>, void const *) noexcept
{
	return true;
}

template <typename T, typename U>
bool equal_member(meta::item<T>, void const * lhs, meta::item<U>, void const * rhs)
	noexcept(noexcept(std::declval<T>() == std::declval<T>()))
{
	return *static_cast<T const *>(lhs) == *static_cast<U const *>(rhs);
}

inline bool equal_member(meta::item<void>, void const *, meta::item<void>, void const *) noexcept
{
	return true;
}

template <typename T>
T & get(meta::item<T>, void * storage)
{
	return *static_cast<T *>(storage);
}

template <typename T>
T const & get(meta::item<T>, void const * storage)
{
	return *static_cast<T const *>(storage);
}

}
}
}

#endif // AVAKAR_LIBAWAIT_VARIANT_STORAGE_H
