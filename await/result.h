#ifndef AWAIT_RESULT_H
#define AWAIT_RESULT_H

#include <type_traits>
#include <exception>

namespace aw {

struct no_result_error
	: std::exception
{
	char const * what() const
	{
		return "no_result_error";
	}
};

template <typename T>
struct result
{
public:
	result();
	result(result const & o);
	result(result && o);
	result(std::exception_ptr e);
	~result();

	template <typename U>
	result(result<U> const & o);

	template <typename U>
	result(result<U> && o);

	result & operator=(result o);

	bool has_value() const;
	bool has_exception() const;
	T get();
	T & value();
	T const & value() const;
	std::exception_ptr exception() const;
	void rethrow() const;

	template <typename U>
	static result<T> from_value(U && v);

private:
	struct empty_t {};
	explicit result(empty_t);

	enum class kind_t { value, exception };

	kind_t m_kind;
	typename std::aligned_union<0, T, std::exception_ptr>::type m_storage;

	template <typename>
	friend struct result;
};

template <>
struct result<void>
{
	result(std::exception_ptr e);

	bool has_value() const;
	bool has_exception() const;
	void get();
	std::exception_ptr exception() const;
	void rethrow() const;

	static result<void> from_value();

private:
	result();

	std::exception_ptr m_exception;
};

template <typename T>
result<typename std::remove_const<typename std::remove_reference<T>::type>::type> value(T && v);

result<void> value();

} // namespace aw

#include "detail/result_impl.h"

#endif // AWAIT_TASK_H
