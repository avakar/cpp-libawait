#include <avakar/await/fiber.h>
#include "win32_error.h"
#include <windows.h>
using namespace aw;
using namespace aw::detail;

namespace {

struct fiber_data
{
	std::function<void()> m_fn;

	PVOID m_master_fiber;
	PVOID m_slave_fiber;
	awwait_base * m_awwaiter;
};

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
		m_sink = &sink;

		PVOID master_fiber = ConvertThreadToFiber(0);
		if (!master_fiber)
		{
			DWORD err = GetLastError();
			if (err != ERROR_ALREADY_FIBER)
				return std::make_exception_ptr(win32_error(err));
			master_fiber = GetCurrentFiber();
		}

		PVOID slave_fiber = CreateFiber(12*1024, &FiberProc, m_data.get());
		if (!slave_fiber)
			return std::make_exception_ptr(win32_error(GetLastError()));

		m_data->m_master_fiber = master_fiber;
		m_data->m_slave_fiber = slave_fiber;

		SwitchToFiber(slave_fiber);
		while (m_data->m_awwaiter != nullptr)
		{
			if (!m_data->m_awwaiter->start(sch, *this))
				return nullptr;

			SwitchToFiber(slave_fiber);
		}

		DeleteFiber(slave_fiber);
		return value();
	}

	result<void> cancel(scheduler & sch, cancel_info ci)
	{
		m_data->m_awwaiter->cancel(sch, ci);
		SwitchToFiber(m_data->m_slave_fiber);

		while (m_data->m_awwaiter != nullptr)
		{
			m_data->m_awwaiter->dismiss(ci);
			SwitchToFiber(m_data->m_slave_fiber);
		}

		DeleteFiber(m_data->m_slave_fiber);
		return value();
	}

private:
	void on_completion(scheduler & sch) noexcept override
	{
		SwitchToFiber(m_data->m_slave_fiber);
		while (m_data->m_awwaiter != nullptr)
		{
			if (!m_data->m_awwaiter->start(sch, *this))
				return;

			SwitchToFiber(m_data->m_slave_fiber);
		}

		DeleteFiber(m_data->m_slave_fiber);
		m_sink->on_completion(sch, value());
	}

	static void CALLBACK FiberProc(PVOID lpParam)
	{
		fiber_data * self = static_cast<fiber_data *>(lpParam);
		self->m_fn();
		self->m_awwaiter = nullptr;
		SwitchToFiber(self->m_master_fiber);
	}

	std::unique_ptr<fiber_data> m_data;
	task_completion<void> * m_sink;
};

}

void aw::try_await_impl(awwait_base & a)
{
	fiber_data * self = static_cast<fiber_data *>(GetFiberData());
	self->m_awwaiter = &a;
	SwitchToFiber(self->m_master_fiber);
}

task<void> aw::fiber_impl(std::function<void()> fn)
{
	return make_command<fiber_cmd>(std::move(fn));
}
