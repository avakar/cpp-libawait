#ifndef AWAIT_TASK_H
#define AWAIT_TASK_H

#include "result.h"
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

template <typename T>
T run(task<T> && t);

template <typename T>
result<T> try_run(task<T> && t);

} // namespace aw

#include "detail/task_impl.h"

#endif // AWAIT_TASK_H
