#include "../timer.h"
#include "../cancel.h"
#include "command.h"
#include "win32_scheduler.h"
#include "win32_error.h"

aw::task<void> aw::wait_ms(int64_t ms)
{
	struct impl
		: private detail::scheduler::completion_sink
	{
		typedef void value_type;

		explicit impl(int64_t ms)
			: m_sink(nullptr)
		{
			m_h = CreateWaitableTimerW(nullptr, TRUE, nullptr);
			if (m_h == 0)
				throw detail::win32_error(::GetLastError());

			LARGE_INTEGER li;
			li.QuadPart = -(ms * 10000);
			if (!SetWaitableTimer(m_h, &li, 0, 0, 0, FALSE))
			{
				DWORD ec = ::GetLastError();
				CloseHandle(m_h);
				throw detail::win32_error(ec);
			}
		}

		~impl()
		{
			if (m_h)
				CloseHandle(m_h);
		}

		result<void> dismiss()
		{
			return std::make_exception_ptr(aw::task_aborted());
		}

		impl(impl && o)
			: m_h(o.m_h)
		{
			o.m_h = 0;
		}

		impl & operator=(impl && o)
		{
			m_h = o.m_h;
			o.m_h = 0;
			return *this;
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			sch.add_handle(m_h, *this);
			m_sink = &sink;
			return nullptr;
		}

	private:
		completion_result on_completion(detail::scheduler & sch) override
		{
			m_sink->on_completion(sch, aw::value());
			return completion_result::finish;
		}

		HANDLE m_h;
		detail::task_completion<void> * m_sink;
	};

	return detail::make_command<impl>(ms);
}
