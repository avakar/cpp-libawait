#ifndef AWAIT_RESULT_H
#define AWAIT_RESULT_H

#include "detail/result_kind.h"
#include <type_traits>
#include <exception>

namespace aw {

struct in_place_t {};
constexpr in_place_t in_place{};

template <typename T>
struct result
{
public:
	result() noexcept;
	result(result const & o) noexcept;
	result(result && o) noexcept;
	result(std::exception_ptr e) noexcept;
	~result();

	template <typename... U>
	explicit result(in_place_t, U &&... u) noexcept;

	template <typename U>
	result(result<U> const & o) noexcept;

	template <typename U>
	result(result<U> && o) noexcept;

	result & operator=(result o) noexcept;

	bool has_value() const noexcept;
	T & value() noexcept;
	T const & value() const noexcept;

	bool has_exception() const noexcept;
	std::exception_ptr & exception() noexcept;
	std::exception_ptr const & exception() const noexcept;

	T && get();
	void rethrow() const;

private:
	detail::result_kind m_kind;
	typename std::aligned_union<0, T, std::exception_ptr>::type m_storage;
};

template <>
struct result<void>
{
	result() noexcept;
	result(std::exception_ptr e) noexcept;
	explicit result(in_place_t) noexcept;

	bool has_value() const noexcept;
	bool has_exception() const noexcept;

	std::exception_ptr & exception() noexcept;
	std::exception_ptr const & exception() const noexcept;

	void get();
	void rethrow() const;

private:
	std::exception_ptr m_exception;
};

template <typename T>
result<typename std::remove_const<typename std::remove_reference<T>::type>::type> value(T && v);

result<void> value();

} // namespace aw

#include "detail/result_impl.h"

#endif // AWAIT_TASK_H
