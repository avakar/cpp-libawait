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

template <typename T>
struct result
{
	result() noexcept;

	template <typename U,
		typename = std::enable_if_t<
		!detail::is_result<typename std::decay<U>::type>::value
		>>
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

	template <typename U>
	result(result<U> && o) noexcept;

	~result();

	T * operator->();
	T const * operator->() const;
	std::add_lvalue_reference_t<T> operator*() &;
	std::add_lvalue_reference_t<T const> operator*() const &;
	std::add_rvalue_reference_t<T> operator*() &&;
	std::add_rvalue_reference_t<T const> operator*() const &&;

	explicit operator bool() const noexcept;
	bool has_value() const noexcept;
	bool has_error_code() const noexcept;
	bool has_exception() const noexcept;

	auto get()
		-> std::add_rvalue_reference_t<T>;

private:
	std::error_code error_code() const noexcept;
	std::exception_ptr exception() const noexcept;
	void rethrow() const;

	template <typename Visitor>
	void visit(Visitor && vis);

	template <typename Visitor>
	void visit(Visitor && vis) const;

	using value_type = std::conditional_t<std::is_void<T>::value, monostate, T>;

	detail::result_kind kind_;
	std::aligned_union_t<0,
		value_type,
		std::error_code,
		std::exception_ptr
		> storage_;

	template <typename U>
	friend struct result;
};

} // namespace libawait
} // namespace avakar

#include "../../../src/result_impl.h"

#endif // AVAKAR_AWAIT_RESULT_H
