#include "../run.h"
#include "linux_scheduler.h"
#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>

aw::result<void> aw::detail::try_run_impl(task<void> && t)
{
	assert(!t.empty());

	struct fake_scheduler
		: scheduler, task_completion<void>
	{
		explicit fake_scheduler(task<void> & t)
			: m_task(t), m_updated(false)
		{
			m_epoll = epoll_create1(EPOLL_CLOEXEC);
		}

		~fake_scheduler()
		{
			close(m_epoll);
		}

		void add_fd(int fd, short events, fd_completion_sink & sink) override
		{
			epoll_event ee;
			ee.events = EPOLLONESHOT;
			if (events & POLLIN)
				ee.events |= EPOLLIN;
			ee.data.ptr = &sink;
			epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &ee);
		}

		void on_completion(scheduler & sch, task<void> && t) override
		{
			assert(&sch == this);
			(void)sch;

			m_task = std::move(t);
			m_updated = true;
		}

		task<void> & m_task;
		bool m_updated;
		int m_epoll;
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
			epoll_event ee;
			epoll_wait(sch.m_epoll, &ee, 1, -1);
			((detail::scheduler::fd_completion_sink *)ee.data.ptr)->on_completion(sch, ee.events);
		}

		vtable = detail::task_access::get_vtable(t);
	}

	detail::task_access::set_vtable(t, nullptr);
	return vtable->get_result(storage);
}
