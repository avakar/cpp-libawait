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
	~task();

	task(result<T> && v);
	task(result<T> const & v);

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
	task();
	task(result<void> && v);
	task(result<void> const & v);

	using task<detail::unit_t>::empty;
};

} // namespace aw

#include "detail/task_impl.h"

#endif // AWAIT_TASK_H
