#ifndef AWAIT_TASK_H
#define AWAIT_TASK_H

#include "result.h"
#include "detail/task_fwd.h"
#include "detail/task_storage.h"
#include "detail/then_traits.h"
#include <exception>
#include <stddef.h>

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

	bool empty() const;
	explicit operator bool() const;

	result<T> dismiss() noexcept;
	result<T> dismiss(cancel_info ci) noexcept;
	void clear();

	template <typename F>
	auto continue_with(F && f)
		-> typename detail::continue_with_traits<T, F>::return_type;

	template <typename F>
	auto then(F && f)
		-> typename detail::then_traits<T, F>::return_type;

	task<void> ignore_result();

	template <typename... P>
	task<T> hold(P &&... p);

private:
	detail::task_kind m_kind;
	typename detail::task_storage<T>::type m_storage;

	friend detail::task_access;
};

template <typename Ctx, typename StartF, typename UpdateF>
task<void> loop(Ctx c, StartF && start, UpdateF && update);

task<void> postpone();

task<void> operator|(task<void> && lhs, task<void> && rhs);

} // namespace aw

#include "detail/task_impl.h"
#include "detail/then_impl.h"
#include "detail/loop_impl.h"

#endif // AWAIT_TASK_H
