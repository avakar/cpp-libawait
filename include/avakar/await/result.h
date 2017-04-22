#ifndef AVAKAR_AWAIT_RESULT_H
#define AVAKAR_AWAIT_RESULT_H

#include "monostate.h"
#include "../../../src/result_traits.h"
#include <type_traits>
#include <system_error>
#include <exception>

namespace avakar {
namespace libawait {

template <typename T>
struct in_place_type_t
{
};

enum class kind {
	value,
	error_code,
	exception,
};

template <typename T>
struct result
{
	result() noexcept;

	template <typename U,
		typename = typename std::enable_if<
		!detail::is_result<typename std::decay<U>::type>::value
		>::type>
	result(U && u) noexcept;

	template <typename... Args>
	result(in_place_type_t<T>, Args &&... args) noexcept;

	template <typename... Args>
	result(in_place_type_t<std::error_code>, Args &&... args) noexcept;

	template <typename... Args>
	result(in_place_type_t<std::exception_ptr>, Args &&... args) noexcept;

	result(result const & o) noexcept;
	result(result && o) noexcept;

	template <typename U>
	result(result<U> const & o) noexcept;

	~result();

	explicit operator bool() const noexcept;
	bool holds_value() const noexcept;
	bool holds_error_code() const noexcept;
	bool holds_exception() const noexcept;

	auto get()
		-> typename std::add_rvalue_reference<T>::type;

private:
	std::error_code error_code() const noexcept;
	std::exception_ptr exception() const noexcept;
	void rethrow() const;

	using value_type = typename std::conditional<std::is_void<T>::value, monostate, T>::type;

	kind kind_;
	typename std::aligned_union<0,
		value_type,
		std::error_code,
		std::exception_ptr
		>::type storage_;

	friend struct result;
};

} // namespace libawait
} // namespace avakar

#include "../../../src/result_impl.h"

#endif // AVAKAR_AWAIT_RESULT_H
