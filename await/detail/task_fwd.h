#ifndef AWAIT_DETAIL_TASK_FWD_H
#define AWAIT_DETAIL_TASK_FWD_H

namespace aw {

template <typename T>
struct task;

namespace detail {

enum class task_kind { empty, value, exception, command };

struct task_access
{
	template <typename T>
	static task_kind get_kind(task<T> const & t);

	template <typename T>
	static void set_kind(task<T> & t, task_kind kind);

	template <typename T>
	static void * storage(task<T> & t);
};

}

}

#endif // AWAIT_DETAIL_TASK_FWD_H
