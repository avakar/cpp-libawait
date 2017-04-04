#include "../fiber.h"
#include "linux_error.h"
#include <string.h>
#include <ucontext.h>
using namespace aw;
using namespace aw::detail;

namespace {

struct fiber_data
{
	std::function<void()> m_fn;

	ucontext_t m_master_fiber;
	ucontext_t m_slave_fiber;
	awwait_base * m_awwaiter;
};

thread_local fiber_data * g_cur_fiber_data = nullptr;

struct fiber_cmd
	: private awwait_completion
{
	typedef void value_type;

	explicit fiber_cmd(std::function<void()> && fn)
		: m_data(new fiber_data())
	{
		m_data->m_fn = std::move(fn);
	}

	result<void> dismiss(cancel_info ci)
	{
		return ci;
	}

	task<void> start(scheduler & sch, task_completion<void> & sink)
	{
		static size_t stack_size = 16*1024;

		m_sink = &sink;
		if (getcontext(&m_data->m_slave_fiber) == -1)
			return std::make_exception_ptr(aw::detail::linux_error(errno));

		m_data->m_slave_fiber.uc_stack.ss_sp = malloc(stack_size);
		if (!m_data->m_slave_fiber.uc_stack.ss_sp)
			return std::make_exception_ptr(std::bad_alloc());
		memset(m_data->m_slave_fiber.uc_stack.ss_sp, 0xcc, stack_size);
		m_data->m_slave_fiber.uc_stack.ss_size = stack_size;
		m_data->m_slave_fiber.uc_link = &m_data->m_master_fiber;
		makecontext(&m_data->m_slave_fiber, &fiber_proc, 0);

		g_cur_fiber_data = m_data.get();
		swapcontext(&m_data->m_master_fiber, &m_data->m_slave_fiber);
		while (m_data->m_awwaiter != nullptr)
		{
			if (!m_data->m_awwaiter->start(sch, *this))
				return nullptr;

			swapcontext(&m_data->m_master_fiber, &m_data->m_slave_fiber);
		}

		free(m_data->m_slave_fiber.uc_stack.ss_sp);
		return value();
	}

	result<void> cancel(scheduler & sch, cancel_info ci)
	{
		g_cur_fiber_data = m_data.get();

		m_data->m_awwaiter->cancel(sch, ci);

		swapcontext(&m_data->m_master_fiber, &m_data->m_slave_fiber);
		while (m_data->m_awwaiter != nullptr)
		{
			m_data->m_awwaiter->dismiss(ci);
			swapcontext(&m_data->m_master_fiber, &m_data->m_slave_fiber);
		}

		free(m_data->m_slave_fiber.uc_stack.ss_sp);
		return value();
	}

private:
	void on_completion(scheduler & sch) noexcept override
	{
		g_cur_fiber_data = m_data.get();
		swapcontext(&m_data->m_master_fiber, &m_data->m_slave_fiber);
		while (m_data->m_awwaiter != nullptr)
		{
			if (!m_data->m_awwaiter->start(sch, *this))
				return;

			swapcontext(&m_data->m_master_fiber, &m_data->m_slave_fiber);
		}

		free(m_data->m_slave_fiber.uc_stack.ss_sp);
		m_sink->on_completion(sch, value());
	}

	static void fiber_proc()
	{
		fiber_data * self = g_cur_fiber_data;
		self->m_fn();
		self->m_awwaiter = nullptr;
	}

	std::unique_ptr<fiber_data> m_data;
	task_completion<void> * m_sink;
};

}

void aw::try_await_impl(awwait_base & a)
{
	fiber_data * self = g_cur_fiber_data;
	self->m_awwaiter = &a;
	swapcontext(&self->m_slave_fiber, &self->m_master_fiber);
}

task<void> aw::fiber_impl(std::function<void()> fn)
{
	return make_command<fiber_cmd>(std::move(fn));
}
