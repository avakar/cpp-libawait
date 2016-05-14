#ifndef AWAIT_TASK_H
#define AWAIT_TASK_H

#include "result.h"
#include "detail/unit.h"
#include "detail/task_access.h"
#include <type_traits>
#include <exception>

namespace aw {

template <typename T>
struct task
{
	task();
	task(task && o);
	task & operator=(task && o);
	~task();

	task(nullptr_t);

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
	: task<detail::unit_t>
{
	task() = default;
	task(task && o);
	task & operator=(task && o);
	task(nullptr_t);
	task(std::exception_ptr e);
	task(result<void> && v);
	task(result<void> const & v);
};

} // namespace aw

#include "detail/task_impl.h"

#endif // AWAIT_TASK_H
