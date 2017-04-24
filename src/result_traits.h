#ifndef AVAKAR_AWAIT_RESULT_TRAITS_H
#define AVAKAR_AWAIT_RESULT_TRAITS_H

#include <type_traits>
#include <system_error>
#include <exception>

namespace avakar {
namespace libawait {

template <typename T>
struct result;

namespace detail {

template <typename T>
struct is_result
	: std::false_type
{
};

template <typename T>
struct is_result<result<T>>
	: std::true_type
{
};

template <size_t I, typename T, typename... Tn>
struct result_index_impl;


template <size_t I, typename T, typename... Tn>
struct result_index_impl<I, T, T, Tn...>
{
	static constexpr bool valid = true;
	static constexpr size_t value = I;
};

template <size_t I, typename T, typename T0, typename... Tn>
struct result_index_impl<I, T, T0, Tn...>
{
	static constexpr bool valid = result_index_impl<I + 1, T, Tn...>::valid;
	static constexpr size_t value = result_index_impl<I + 1, T, Tn...>::value;
};

template <size_t I, typename T>
struct result_index_impl<I, T>
{
	static constexpr bool valid = false;
};

template <typename U, typename T>
struct result_index
{
	using impl = result_index_impl<0, U, T, std::error_code, std::exception_ptr>;

	static constexpr bool valid = impl::valid;
	static constexpr size_t value = impl::value;
};

} // namespace detail
} // namespace libawait
} // namespace avakar

#endif // AVAKAR_AWAIT_RESULT_TRAITS_H
