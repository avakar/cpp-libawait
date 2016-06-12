#ifndef AWAIT_DETAIL_TASK_VTABLE_H
#define AWAIT_DETAIL_TASK_VTABLE_H

#include "../result.h"
#include <memory>

namespace aw {

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
	virtual ~command() {}
	virtual result<T> dismiss() = 0;
	virtual task<T> start(scheduler & sch, task_completion<T> & sink) = 0;
	virtual void cancel(scheduler & sch) = 0;
};

struct command_deleter
{
	template <typename T>
	void operator()(command<T> * cmd)
	{
		(void)cmd->dismiss();
		delete cmd;
	}
};

template <typename T>
using command_ptr = std::unique_ptr<command<T>, command_deleter>;

}
}

#endif // AWAIT_DETAIL_TASK_VTABLE_H
