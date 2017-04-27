#ifndef AVAKAR_AWAIT_RESULT_H
#define AVAKAR_AWAIT_RESULT_H

#include "monostate.h"
#include "../../../src/result_traits.h"
#include <type_traits>
#include <system_error>
#include <exception>

namespace avakar {
namespace libawait {

namespace aw = ::avakar::libawait;

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

}

template <typename T>
struct in_place_type_t
{
};

template <typename T>
struct result
{
	result() noexcept;

	template <typename U,
		typename = std::enable_if_t<!detail::is_result<std::decay_t<U>>::value>>
	result(U && u) noexcept;

	template <typename U, typename... Args,
		typename = std::enable_if_t<detail::result_index<U, T>::valid>>
	result(in_place_type_t<U>, Args &&... args) noexcept;

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

	std::size_t index_;
	std::aligned_union_t<0,
		value_type,
		std::error_code,
		std::exception_ptr
		> storage_;

	template <typename U>
	friend struct result;

	friend struct detail::result_storage;
};

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
