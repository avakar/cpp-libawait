#ifndef AVAKAR_AWAIT_RESULT_TRAITS_H
#define AVAKAR_AWAIT_RESULT_TRAITS_H

#include <avakar/meta.h>
#include <type_traits>
#include <system_error>
#include <exception>

namespace avakar {
namespace libawait {

template <typename T>
struct result;

namespace detail {

struct result_storage
{
	template <typename T>
	static void * get(result<T> & r)
	{
		return &r.storage_;
	}

	template <typename T>
	static void const * get(result<T> const & r)
	{
		return &r.storage_;
	}

	template <typename T>
	static size_t index(result<T> const & r)
	{
		return r.index_;
	}
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

template <typename T>
using result_types = meta::list<T, std::error_code, std::exception_ptr>;

template <typename Visitor>
using visit_error_result_t = std::common_type_t<
	decltype(std::declval<Visitor>()(std::declval<std::error_code const &>())),
	decltype(std::declval<Visitor>()(std::declval<std::exception_ptr const &>()))>;

} // namespace detail
} // namespace libawait
} // namespace avakar

#endif // AVAKAR_AWAIT_RESULT_TRAITS_H
