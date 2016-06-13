#ifndef AWAIT_DETAIL_COMMAND_PTR_H
#define AWAIT_DETAIL_COMMAND_PTR_H

#include "../result.h"
#include "command_intf.h"
#include "task_fwd.h"

namespace aw {
namespace detail {

template <typename T>
struct command_ptr
{
	command_ptr() noexcept;
	command_ptr(std::nullptr_t) noexcept;
	explicit command_ptr(command<T> * p) noexcept;

	bool empty() const noexcept;
	explicit operator bool() const noexcept;
	bool operator!() const noexcept;

	command<T> & operator*() const noexcept;
	command<T> * operator->() const noexcept;

	result<T> dismiss() noexcept;
	task<T> start(scheduler & sch, task_completion<T> & sink) noexcept;
	void complete() noexcept;

	command_ptr(command_ptr && o) noexcept;
	command_ptr & operator=(command_ptr o) noexcept;
	~command_ptr();

private:
	command<T> * m_ptr;
};

}
}

#endif // AWAIT_DETAIL_COMMAND_PTR_H
