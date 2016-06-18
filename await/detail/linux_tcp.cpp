#include "../tcp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>

#include "linux_error.h"
#include "linux_fd_task.h"
#include "linux_scheduler.h"
#include "../cancel.h"

namespace {

struct addrinfo_guard
{
	addrinfo_guard(struct addrinfo * addrs)
		: m_addrs(addrs)
	{
	}

	~addrinfo_guard()
	{
		freeaddrinfo(m_addrs);
	}

	struct addrinfo * m_addrs;
};

template <typename F>
struct poll_op_cmd
	: private aw::detail::scheduler::fd_completion_sink
{
	typedef decltype(std::declval<F>()(1)) task_type;
	typedef typename aw::detail::task_traits<task_type>::value_type value_type;

	poll_op_cmd(F && f, int fd, int events)
		: m_f(std::move(f)), m_fd(fd), m_events(events), m_sink(nullptr)
	{
	}

	aw::result<value_type> dismiss()
	{
		return std::make_exception_ptr(aw::task_aborted());
	}

	aw::task<value_type> start(aw::detail::scheduler & sch,
		aw::detail::task_completion<value_type> & sink) noexcept
	{
		m_sink = &sink;
		sch.add_fd(m_fd, m_events, *this);
		return nullptr;
	}

	aw::task<value_type> cancel(aw::detail::scheduler & sch)
	{
		sch.remove_fd(m_fd, *this);
		return std::make_exception_ptr(aw::task_aborted());
	}

private:
	void on_completion(aw::detail::scheduler & sch, short revents) override
	{
		(void)revents;
		aw::task<value_type> t = m_f(m_fd);
		if (t.empty())
			sch.add_fd(m_fd, m_events, *this);
		else
			m_sink->on_completion(sch, std::move(t));
	}

	F m_f;
	int m_fd;
	int m_events;
	aw::detail::task_completion<value_type> * m_sink;
};

template <typename F>
auto poll_op(int fd, int events, F && f) -> decltype(f(1))
{
	if (auto t = f(fd))
		return t;
	return aw::detail::make_command<poll_op_cmd<F>>(std::move(f), fd, events);
}

struct tcp_stream
	: aw::stream
{
	tcp_stream()
		: m_sock(-1)
	{
	}

	~tcp_stream()
	{
		if (m_sock != -1)
			close(m_sock);
	}

	aw::task<size_t> read_any(uint8_t * buf, size_t size) override
	{
		return poll_op(m_sock, POLLIN, [buf, size](int fd) -> aw::task<size_t> {
			ssize_t r = read(fd, buf, size);
			if (r >= 0)
				return aw::value(static_cast<size_t>(r));

			int err = errno;
			if (err != EAGAIN)
				return std::make_exception_ptr(aw::detail::linux_error(err));

			return nullptr;
		});
	}

	aw::task<size_t> write_any(uint8_t const * buf, size_t size) override
	{
		return poll_op(m_sock, POLLOUT, [buf, size](int fd) -> aw::task<size_t> {
			ssize_t r = write(fd, buf, size);
			if (r >= 0)
				return aw::value(static_cast<size_t>(r));

			int err = errno;
			if (err != EAGAIN)
				return std::make_exception_ptr(aw::detail::linux_error(err));

			return nullptr;
		});
	}

	int m_sock;	

	tcp_stream(tcp_stream const &) = delete;
	tcp_stream & operator=(tcp_stream const &) = delete;
};

}

aw::task<std::shared_ptr<aw::stream>> aw::tcp_connect(char const * host, uint16_t port) noexcept
{
	try
	{
		struct addrinfo * addrs;
		int r = getaddrinfo(host, nullptr, nullptr, &addrs);
		if (r != 0)
			return std::make_exception_ptr(detail::linux_error(r));
		addrinfo_guard ag(addrs);

		for (; addrs != nullptr; addrs = addrs->ai_next)
		{
			if (addrs->ai_family == AF_INET || addrs->ai_family == AF_INET6)
				break;
		}

		if (addrs == nullptr)
			return std::make_exception_ptr(unknown_host_error());

		if (addrs->ai_family == AF_INET)
		{
			sockaddr_in & addr = reinterpret_cast<sockaddr_in &>(*addrs->ai_addr);
			addr.sin_port = htons(port);
		}
		else
		{
			sockaddr_in6 & addr = reinterpret_cast<sockaddr_in6 &>(*addrs->ai_addr);
			addr.sin6_port = htons(port);
		}

		std::shared_ptr<tcp_stream> rs = std::make_shared<tcp_stream>();

		rs->m_sock = socket(addrs->ai_family, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
		if (rs->m_sock == -1)
			return std::make_exception_ptr(detail::linux_error(errno));

		if (connect(rs->m_sock, addrs->ai_addr, addrs->ai_addrlen) == -1)
		{
			int r = errno;
			if (r != EINPROGRESS)
				return std::make_exception_ptr(detail::linux_error(r));
		}

		return detail::linux_fd_task(rs->m_sock, POLLOUT).then([rs](short revents) {
			(void)revents;
			std::shared_ptr<stream> r = rs;
			return aw::value(r);
		});
	}
	catch (...)
	{
		return std::current_exception();
	}
}
