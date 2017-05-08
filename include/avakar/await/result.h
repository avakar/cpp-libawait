#ifndef AVAKAR_AWAIT_RESULT_H
#define AVAKAR_AWAIT_RESULT_H

#include "monostate.h"
#include <avakar/meta.h>
#include "../../../src/result_traits.h"
#include "../../../src/variant_storage.h"
#include <type_traits>
#include <system_error>
#include <exception>

namespace avakar {
namespace libawait {

namespace aw = ::avakar::libawait;

struct in_place_t
{
};

constexpr in_place_t in_place{};

template <typename T>
struct in_place_type_t
{
};

template <typename T>
struct result
{
	static_assert(
		std::is_same<std::decay_t<T>, T>::value,
		"T shall not be cv-qualified, nor of reference, array or function type");
	static_assert(!std::is_same<T, std::error_code>::value, "T shall not be std::error_code");
	static_assert(!std::is_same<T, std::exception_ptr>::value, "T shall not be std::exception_ptr");

	result() noexcept;

	template <typename U,
		typename = std::enable_if_t<
			!detail::is_result<std::decay_t<U>>::value
			&& std::is_constructible<meta::choose_overload_t<U, detail::result_types<T>>, U>::value
		>>
	result(U && u) noexcept;

	template <typename... Args>
	result(in_place_t, Args &&... args) noexcept;

	template <typename U, typename... Args,
		typename = std::enable_if_t<
			meta::index_of<U, detail::result_types<T>>::value != meta::npos
			&& (std::is_constructible<U, Args...>::value || (std::is_void<U>::value && sizeof...(Args) == 0))
		>>
	result(in_place_type_t<U>, Args &&... args) noexcept;

	result(result const & o) noexcept;
	result(result && o) noexcept;

	template <typename U,
		typename = std::enable_if_t<std::is_constructible<T, U &>::value>>
	result(result<U> const & o) noexcept;

	template <typename U,
		typename = std::enable_if_t<std::is_constructible<T, U>::value>>
	result(result<U> && o) noexcept;

	result & operator=(result const & o);
	result & operator=(result && o);

	~result();

	T * operator->();
	T const * operator->() const;
	std::add_lvalue_reference_t<T> operator*() &;
	std::add_lvalue_reference_t<T const> operator*() const &;
	std::add_rvalue_reference_t<T> operator*() &&;
	std::add_rvalue_reference_t<T const> operator*() const &&;

	std::add_lvalue_reference_t<T> value() &;
	std::add_lvalue_reference_t<T const> value() const &;
	std::add_rvalue_reference_t<T> value() && ;
	std::add_rvalue_reference_t<T const> value() const &&;

	std::exception_ptr exception() const noexcept;

	explicit operator bool() const noexcept;
	bool has_value() const noexcept;

	auto get()
		-> std::add_rvalue_reference_t<T>;

	template <typename U>
	result<U> convert_error();

	bool operator==(result const & rhs) const;
	bool operator!=(result const & rhs) const;

private:
	template <typename Visitor>
	detail::visit_error_result_t<Visitor> visit_error(Visitor && vis) const;

	void rethrow() const;

	using _types = detail::result_types<T>;

	template <typename U>
	using storage_index = meta::index_of<U, _types>;

	std::size_t index_;
	detail::variant_storage_t<_types> storage_;

	template <typename U>
	friend struct result;

	friend struct detail::result_storage;
};

template <typename T, typename U>
bool operator==(U && lhs, result<T> const & rhs);

template <typename T, typename U>
bool operator!=(U && u, result<T> const & rhs);

template <typename U, typename T>
bool holds_alternative(result<T> const & o) noexcept;

template <typename U, typename T>
U const * get_if(result<T> const & o) noexcept;

template <typename U, typename T>
U const * get_if(result<T> const && o) noexcept;

template <typename U, typename T>
U * get_if(result<T> & o) noexcept;

template <typename U, typename T>
U * get_if(result<T> && o) noexcept;

template <typename U, typename T>
std::add_lvalue_reference_t<U const> get(result<T> const & o) noexcept;

template <typename U, typename T>
std::add_rvalue_reference_t<U const> get(result<T> const && o) noexcept;

template <typename U, typename T>
std::add_lvalue_reference_t<U> get(result<T> & o) noexcept;

template <typename U, typename T>
std::add_rvalue_reference_t<U> get(result<T> && o) noexcept;

} // namespace libawait
} // namespace avakar

#include "../../../src/result_impl.h"

#endif // AVAKAR_AWAIT_RESULT_H
