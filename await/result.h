#ifndef AWAIT_TASK_H
#define AWAIT_TASK_H

#include <type_traits>
#include <exception>

namespace aw {

template <typename T>
struct result
{
public:
	result(result const & o);
	result(result && o);
	result(std::exception_ptr e);
	~result();

	template <typename U>
	result(result<U> const & o);

	template <typename U>
	result(result<U> && o);

	bool has_value() const;
	bool has_exception() const;
	T & get();
	T const & get() const;
	void rethrow() const;

	template <typename U>
	static result<T> from_value(U && v);

private:
	result();

	enum class kind_t { value, exception };

	kind_t m_kind;
	typename std::aligned_union<0, T, std::exception_ptr>::type m_storage;

	friend struct result;
};

template <typename T>
result<T> value(T && v);

template <typename T>
result<T> value(T const & v);

} // namespace aw

#include "detail/result_impl.h"

#endif // AWAIT_TASK_H
