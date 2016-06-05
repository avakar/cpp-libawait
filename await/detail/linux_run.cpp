#include "../run.h"
#include "linux_scheduler.h"
#include "intrusive_list.h"
#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>

static_assert(POLLIN == EPOLLIN, "POLL and EPOLL constants diverge");
static_assert(POLLPRI == EPOLLPRI, "POLL and EPOLL constants diverge");
static_assert(POLLOUT == EPOLLOUT, "POLL and EPOLL constants diverge");
static_assert(POLLERR == EPOLLERR, "POLL and EPOLL constants diverge");
static_assert(POLLHUP == EPOLLHUP, "POLL and EPOLL constants diverge");

aw::result<void> aw::detail::try_run_impl(task<void> && t)
{
	assert(!t.empty());

	struct epoll_item
		: intrusive_hook
	{
		int fd;
		scheduler::fd_completion_sink * sink;
	};

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
			epoll_item * item = new epoll_item();
			item->fd = fd;
			item->sink = &sink;

			epoll_event ee;
			ee.events = EPOLLONESHOT | (events & (POLLIN | POLLPRI | POLLOUT));
			ee.data.ptr = item;
			epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &ee);

			m_items.push_back(*item);
		}

		void on_completion(scheduler & sch, task<void> && t) override
		{
			assert(&sch == this);
			(void)sch;

			m_task = std::move(t);
			m_updated = true;
		}

		task<void> & m_task;
		intrusive_list<epoll_item> m_items;
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

			epoll_item * item = static_cast<epoll_item *>(ee.data.ptr);
			scheduler::fd_completion_sink * sink = item->sink;

			epoll_ctl(sch.m_epoll, EPOLL_CTL_DEL, item->fd, nullptr);
			sch.m_items.remove(*item);
			delete item;

			sink->on_completion(sch, ee.events);
		}

		vtable = detail::task_access::get_vtable(t);
	}

	detail::task_access::set_vtable(t, nullptr);
	return vtable->get_result(storage);
}
