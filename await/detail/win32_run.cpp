#include "../run.h"
#include "win32_scheduler.h"

aw::result<void> aw::detail::try_run_impl(task<void> && t)
{
	assert(!t.empty());

	struct fake_scheduler
		: scheduler, task_completion<void>
	{
		explicit fake_scheduler(task<void> & t)
			: m_task(t), m_updated(false), m_handle_count(0)
		{
		}

		void add_handle(HANDLE h, handle_completion_sink & sink) override
		{
			m_handles[m_handle_count] = h;
			m_sinks[m_handle_count] = &sink;
			++m_handle_count;
		}

		void on_completion(task<void> && t) override
		{
			m_task = std::move(t);
			m_updated = true;
		}

		task<void> & m_task;
		bool m_updated;
		HANDLE m_handles[64];
		handle_completion_sink * m_sinks[64];
		size_t m_handle_count;
	};

	fake_scheduler sch(t);

	detail::task_vtable<void> const * vtable = detail::task_access::get_vtable(t);
	void * storage = detail::task_access::storage(t);

	while (vtable->get_result == nullptr)
	{
		assert(vtable->start != nullptr);
		task<void> u = vtable->start(storage, sch, sch);
		while (!u.empty())
		{
			t = std::move(u);
			vtable = detail::task_access::get_vtable(t);
			u = vtable->start(storage, sch, sch);
		}

		sch.m_updated = false;
		while (!sch.m_updated)
		{
			DWORD r = WaitForMultipleObjectsEx(sch.m_handle_count, sch.m_handles, FALSE, INFINITE, TRUE);
			if (r >= WAIT_OBJECT_0 && r - WAIT_OBJECT_0 < sch.m_handle_count)
			{
				size_t i = r - WAIT_OBJECT_0;
				auto sink = sch.m_sinks[i];

				std::swap(sch.m_handles[i], sch.m_handles[sch.m_handle_count - 1]);
				std::swap(sch.m_sinks[i], sch.m_sinks[sch.m_handle_count - 1]);
				--sch.m_handle_count;

				sink->on_completion();
			}
		}

		vtable = detail::task_access::get_vtable(t);
	}

	detail::task_access::set_vtable(t, nullptr);
	return vtable->get_result(storage);
}
