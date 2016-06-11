#include "../run.h"
#include "win32_scheduler.h"
#include "intrusive_list.h"
#include <stdexcept>

aw::result<void> aw::detail::try_run_impl(task<void> && t)
{
	assert(!t.empty());

	struct fake_scheduler
		: scheduler, task_completion<void>
	{
		struct sleeper_node_impl
			: sleeper_node, intrusive_hook
		{
			sleeper_node_impl(fake_scheduler & sch, completion_sink & sink)
				: m_sch(sch), m_sink(sink)
			{
			}

			void wakeup() override
			{
				m_sch.m_sleepers.remove(*this);
				m_sch.m_woken_sleepers.push_back(*this);
			}

			fake_scheduler & m_sch;
			completion_sink & m_sink;
		};

		explicit fake_scheduler(task<void> & t)
			: m_task(t), m_updated(false), m_handle_count(0)
		{
		}

		void add_handle(HANDLE h, completion_sink & sink) override
		{
			if (m_handle_count >= 63)
				throw std::runtime_error("scheduler can accomodate no more handles");

			m_handles[m_handle_count] = h;
			m_sinks[m_handle_count] = &sink;
			++m_handle_count;
		}

		sleeper_node * register_sleeper(completion_sink & sink) override
		{
			sleeper_node_impl * s = new sleeper_node_impl(*this, sink);
			m_sleepers.push_back(*s);
			return s;
		}

		void on_completion(scheduler & sch, task<void> && t) override
		{
			assert(&sch == this);
			(void)sch;

			detail::mark_complete(m_task);
			m_task = std::move(t);
			m_updated = true;
		}

		task<void> & m_task;
		bool m_updated;
		HANDLE m_handles[64];
		completion_sink * m_sinks[64];
		size_t m_handle_count;

		intrusive_list<sleeper_node_impl> m_sleepers;
		intrusive_list<sleeper_node_impl> m_woken_sleepers;
	};

	fake_scheduler sch(t);
	while (detail::start_command(t, sch, sch))
	{
		sch.m_updated = false;
		while (!sch.m_updated)
		{
			if (sch.m_handle_count == 0)
			{
				SleepEx(INFINITE, TRUE);
			}
			else
			{
				DWORD r = WaitForMultipleObjectsEx(sch.m_handle_count, sch.m_handles, FALSE, INFINITE, TRUE);
				if (r >= WAIT_OBJECT_0 && r - WAIT_OBJECT_0 < sch.m_handle_count)
				{
					size_t i = r - WAIT_OBJECT_0;
					auto sink = sch.m_sinks[i];
					HANDLE handle = sch.m_handles[i];

					std::swap(sch.m_handles[i], sch.m_handles[sch.m_handle_count - 1]);
					std::swap(sch.m_sinks[i], sch.m_sinks[sch.m_handle_count - 1]);
					--sch.m_handle_count;

					auto cr = sink->on_completion(sch);
					if (cr == scheduler::completion_sink::completion_result::reschedule)
					{
						sch.m_handles[sch.m_handle_count] = handle;
						sch.m_sinks[sch.m_handle_count] = sink;
						++sch.m_handle_count;
					}
				}
			}

			for (auto it = sch.m_woken_sleepers.begin(); it != sch.m_woken_sleepers.end(); )
			{
				auto & sleeper = *it;

				auto cr = sleeper.m_sink.on_completion(sch);
				it = sch.m_woken_sleepers.remove(sleeper);

				if (cr == scheduler::completion_sink::completion_result::reschedule)
					sch.m_sleepers.push_back(sleeper);
				else
					delete &sleeper;
			}
		}
	}

	return detail::fetch_result(t);
}
