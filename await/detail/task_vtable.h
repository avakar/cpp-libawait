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
	virtual result<T> dismiss() noexcept = 0;
	virtual task<T> start(scheduler & sch, task_completion<T> & sink) noexcept = 0;
	virtual void cancel(scheduler & sch) noexcept = 0;
};

struct command_deleter
{
	template <typename T>
	void operator()(command<T> * cmd) noexcept
	{
		(void)cmd->dismiss();
		delete cmd;
	}
};

template <typename T>
struct command_ptr
{
	command_ptr() noexcept
		: m_ptr(nullptr)
	{
	}

	command_ptr(std::nullptr_t) noexcept
		: m_ptr(nullptr)
	{
	}

	explicit command_ptr(command<T> * p) noexcept
		: m_ptr(p)
	{
	}

	command_ptr(command_ptr && o) noexcept
		: m_ptr(o.m_ptr)
	{
		o.m_ptr = nullptr;
	}

	~command_ptr()
	{
		if (!this->empty())
			this->dismiss();
	}

	command_ptr & operator=(command_ptr o)
	{
		std::swap(m_ptr, o.m_ptr);
		return *this;
	}

	bool empty() const noexcept
	{
		return m_ptr == nullptr;
	}

	explicit operator bool() const noexcept
	{
		return m_ptr != nullptr;
	}

	bool operator!() const noexcept
	{
		return m_ptr == nullptr;
	}

	command<T> & operator*() const noexcept
	{
		return *m_ptr;
	}

	command<T> * operator->() const noexcept
	{
		return m_ptr;
	}

	result<T> dismiss() noexcept
	{
		assert(m_ptr != nullptr);

		result<T> r = m_ptr->dismiss();
		this->complete();
		return r;
	}

	task<T> start(scheduler & sch, task_completion<T> & sink) noexcept
	{
		assert(m_ptr != nullptr);

		for (;;)
		{
			task<T> t = m_ptr->start(sch, sink);
			if (t.empty())
				return nullptr;

			this->complete();
			*this = detail::fetch_command(t);

			if (m_ptr == nullptr)
				return std::move(t);
		}
	}

	void complete() noexcept
	{
		delete m_ptr;
		m_ptr = nullptr;
	}

private:
	command<T> * m_ptr;
};

}
}

#endif // AWAIT_DETAIL_TASK_VTABLE_H
