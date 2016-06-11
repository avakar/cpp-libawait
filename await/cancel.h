#ifndef AWAIT_CANCEL_H
#define AWAIT_CANCEL_H

#include <exception>

namespace aw {

struct task_aborted
	: std::exception
{
	char const * what() const noexcept override
	{
		return "task_aborted";
	}
};

}

#endif // AWAIT_CANCEL_H
