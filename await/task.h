#ifndef AWAIT_TASK_H
#define AWAIT_TASK_H

#include "result.h"
#include "detail/task_access.h"
#include "detail/task_vtable.h"
#include "detail/simple_commands.h"
#include "detail/task_storage.h"
#include "detail/then_traits.h"
#include <type_traits>
#include <exception>
#include <stddef.h>
#include <stdint.h>

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
	explicit operator bool() const;

	task<void> ignore_result();

	template <typename... P>
	task<T> hold(P &&... p);

	template <typename F>
	typename detail::then_traits<T, F>::task_type then(F && f);

	template <typename F>
	decltype(std::declval<F>()(std::declval<result<T>>())) continue_with(F && f);

private:
	detail::task_vtable<T> const * m_vtable;
	typename detail::task_storage<T>::type m_storage;

	friend detail::task_access;
};

template <typename F>
task<void> loop(F && f);

template <typename Ctx, typename StartF, typename UpdateF>
task<void> loop(Ctx c, StartF && start, UpdateF && update);

task<void> postpone();

} // namespace aw

#include "detail/task_impl.h"
#include "detail/then_impl.h"
#include "detail/loop_impl.h"

#endif // AWAIT_TASK_H
