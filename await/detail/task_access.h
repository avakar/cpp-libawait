#ifndef AWAIT_DETAIL_TASK_ACCESS_H
#define AWAIT_DETAIL_TASK_ACCESS_H

#include <exception>

namespace aw {

template <typename T>
struct task;

namespace detail {

enum class task_kind { empty, value, exception };

struct task_access
{
	template <typename T>
	static task_kind get_kind(task<T> const & t);

	template <typename T>
	static T & as_value(task<T> & t);

	template <typename T>
	static T const & as_value(task<T> const & t);

	template <typename T>
	static std::exception_ptr & as_exception(task<T> & t);

	template <typename T>
	static std::exception_ptr const & as_exception(task<T> const & t);
};

}
}

#endif // AWAIT_DETAIL_TASK_ACCESS_H
