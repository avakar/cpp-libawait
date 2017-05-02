#ifndef AVAKAR_LIBAWAIT_COMMAND_INTF_H
#define AVAKAR_LIBAWAIT_COMMAND_INTF_H

#include <avakar/await/result.h>

namespace avakar {
namespace libawait {

template <typename T>
struct task;

namespace detail {

struct scheduler;

template <typename T>
struct task_completion
{
	virtual void on_completion(scheduler & sch, task<T> && t) = 0;
};

template <typename T>
struct command
{
	virtual task<T> start(scheduler & sch, task_completion<T> & sink) noexcept = 0;
	virtual result<T> cancel(scheduler * sch) noexcept = 0;
};

}
}
}

#endif // AVAKAR_LIBAWAIT_COMMAND_INTF_H
