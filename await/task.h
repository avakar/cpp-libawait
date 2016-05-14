#ifndef AWAIT_TASK_H
#define AWAIT_TASK_H

#include "result.h"
#include "detail/task_access.h"
#include <type_traits>
#include <exception>
#include <cstddef>

namespace aw {

template <typename T>
struct task
{
	task();
	task(task && o);
	task & operator=(task && o);
	~task();

	task(std::nullptr_t);
	task(std::exception_ptr e);

	template <typename U>
	task(result<U> && v);

	template <typename U>
	task(result<U> const & v);

	void clear();
	bool empty() const;

private:
	typedef detail::task_kind kind;

	kind m_kind;
	typename std::aligned_union<0, T, std::exception_ptr>::type m_storage;

	friend detail::task_access;
};

template <>
struct task<void>
{
	task();
	task(task && o);
	task & operator=(task && o);
	~task();

	task(std::nullptr_t);
	task(std::exception_ptr e);

	task(result<void> const & v);

	void clear();
	bool empty() const;

private:
	typedef detail::task_kind kind;

	kind m_kind;
	typename std::aligned_union<0, std::exception_ptr>::type m_storage;

	friend detail::task_access;
};

} // namespace aw

#include "detail/task_impl.h"

#endif // AWAIT_TASK_H
