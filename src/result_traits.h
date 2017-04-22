#ifndef AVAKAR_AWAIT_RESULT_TRAITS_H
#define AVAKAR_AWAIT_RESULT_TRAITS_H

#include <type_traits>

namespace avakar {
namespace libawait {

template <typename T>
struct result;

namespace detail {

enum class result_kind
{
	value,
	error_code,
	exception,
};

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

} // namespace detail
} // namespace libawait
} // namespace avakar

#endif // AVAKAR_AWAIT_RESULT_TRAITS_H
