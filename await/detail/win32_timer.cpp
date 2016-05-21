#include "../timer.h"
#include "command.h"
#include "win32_scheduler.h"

aw::task<void> aw::wait_ms(int64_t ms)
{
	struct impl
		: private detail::scheduler::handle_completion_sink
	{
		typedef void value_type;

		explicit impl(int64_t ms)
			: m_sink(nullptr)
		{
			m_h = CreateWaitableTimerW(nullptr, TRUE, nullptr);

			LARGE_INTEGER li;
			li.QuadPart = -(ms * 10000);
			SetWaitableTimer(m_h, &li, 0, 0, 0, FALSE);
		}

		~impl()
		{
			if (m_h)
				CloseHandle(m_h);
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
		void on_completion() override
		{
			m_sink->on_completion(aw::value());
		}

		HANDLE m_h;
		detail::task_completion<void> * m_sink;
	};

	return detail::make_command<impl>(ms);
}
