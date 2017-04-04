#include <avakar/await/tcp.h>
#include <avakar/await/cancel.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <avakar/singleton.h>
#include "win32_error.h"
#include "win32_scheduler.h"
#include <memory>

namespace {

struct winsock_guard
{
	winsock_guard()
	{
		WSADATA wd;
		int r = WSAStartup(MAKEWORD(2, 2), &wd);
		if (r != 0)
			throw aw::detail::win32_error(r);
	}

	~winsock_guard()
	{
		WSACleanup();
	}
};

avakar::singleton<winsock_guard> g_winsock;

}

struct addrinfo_guard
{
	explicit addrinfo_guard(ADDRINFOA * addrs)
		: m_addrs(addrs)
	{
	}

	~addrinfo_guard()
	{
		freeaddrinfo(m_addrs);
	}

	ADDRINFOA * m_addrs;
};

struct sock_read_command
	: private aw::detail::scheduler::completion_sink
{
	typedef size_t value_type;

	sock_read_command(SOCKET sock, uint8_t * buf, size_t size)
		: m_sock(sock), m_buf(buf), m_size(size), m_ov()
	{
	}

	aw::result<size_t> dismiss(aw::cancel_info ci)
	{
		return ci;
	}

	aw::task<value_type> start(aw::detail::scheduler & sch, aw::detail::task_completion<value_type> & sink)
	{
		WSABUF b;
		b.buf = (CHAR *)m_buf;
		b.len = m_size;

		DWORD dwFlags = 0;
		int r = WSARecv(m_sock, &b, 1, nullptr, &dwFlags, &m_ov, &completion_routine);
		if (r != 0)
		{
			r = WSAGetLastError();
			if (r != WSA_IO_PENDING)
				return std::make_exception_ptr(aw::detail::win32_error(r));
		}

		m_sleeper_node = sch.register_sleeper(*this);
		m_sink = &sink;
		return nullptr;
	}

	aw::result<size_t> cancel(aw::detail::scheduler & sch, aw::cancel_info ci)
	{
		CancelIoEx((HANDLE)m_sock, &m_ov);
		sch.wait_for_sleeper(*m_sleeper_node);

		if (m_ov.Offset != 0)
		{
			if (m_ov.Offset == ERROR_OPERATION_ABORTED)
				return ci;
			return std::make_exception_ptr(aw::detail::win32_error((DWORD)m_ov.Offset));
		}
		return aw::value((size_t)m_ov.Internal);
	}

	completion_result on_completion(aw::detail::scheduler & sch)
	{
		if (m_ov.Offset != 0)
			m_sink->on_completion(sch, std::make_exception_ptr(aw::detail::win32_error((DWORD)m_ov.Offset)));
		else
			m_sink->on_completion(sch, aw::value((size_t)m_ov.Internal));
		return completion_result::finish;
	}
	
	static void CALLBACK completion_routine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
	{
		(void)dwFlags;

		sock_read_command * self = CONTAINING_RECORD(lpOverlapped, sock_read_command, m_ov);
		lpOverlapped->Offset = dwError;
		lpOverlapped->Internal = cbTransferred;
		self->m_sleeper_node->wakeup();
	}

private:
	SOCKET m_sock;
	uint8_t * m_buf;
	size_t m_size;
	WSAOVERLAPPED m_ov;
	aw::detail::task_completion<value_type> * m_sink;
	aw::detail::scheduler::sleeper_node * m_sleeper_node;
};

struct sock_write_command
	: private aw::detail::scheduler::completion_sink
{
	typedef size_t value_type;

	sock_write_command(SOCKET sock, uint8_t const * buf, size_t size)
		: m_sock(sock), m_buf(buf), m_size(size), m_ov()
	{
	}

	aw::result<size_t> dismiss(aw::cancel_info ci)
	{
		return ci;
	}

	aw::task<value_type> start(aw::detail::scheduler & sch, aw::detail::task_completion<value_type> & sink)
	{
		WSABUF b;
		b.buf = (CHAR *)m_buf;
		b.len = m_size;

		DWORD dwFlags = 0;
		int r = WSASend(m_sock, &b, 1, nullptr, dwFlags, &m_ov, &completion_routine);
		if (r != 0)
		{
			r = WSAGetLastError();
			if (r != WSA_IO_PENDING)
				return std::make_exception_ptr(aw::detail::win32_error(r));
		}

		m_sleeper_node = sch.register_sleeper(*this);
		m_sink = &sink;
		return nullptr;
	}

	aw::result<size_t> cancel(aw::detail::scheduler & sch, aw::cancel_info ci)
	{
		CancelIoEx((HANDLE)m_sock, &m_ov);
		sch.wait_for_sleeper(*m_sleeper_node);

		if (m_ov.Offset != 0)
		{
			if (m_ov.Offset == ERROR_OPERATION_ABORTED)
				return ci;
			return std::make_exception_ptr(aw::detail::win32_error((DWORD)m_ov.Offset));
		}
		return aw::value((size_t)m_ov.Internal);
	}

	completion_result on_completion(aw::detail::scheduler & sch)
	{
		if (m_ov.Offset != 0)
			m_sink->on_completion(sch, std::make_exception_ptr(aw::detail::win32_error((DWORD)m_ov.Offset)));
		else
			m_sink->on_completion(sch, aw::value((size_t)m_ov.Internal));
		return completion_result::finish;
	}

	static void CALLBACK completion_routine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
	{
		(void)dwFlags;

		sock_write_command * self = CONTAINING_RECORD(lpOverlapped, sock_write_command, m_ov);
		lpOverlapped->Offset = dwError;
		lpOverlapped->Internal = cbTransferred;
		self->m_sleeper_node->wakeup();
	}

private:
	SOCKET m_sock;
	uint8_t const * m_buf;
	size_t m_size;
	WSAOVERLAPPED m_ov;
	aw::detail::task_completion<value_type> * m_sink;
	aw::detail::scheduler::sleeper_node * m_sleeper_node;
};

struct tcp_socket
	: aw::stream
{
	tcp_socket(std::shared_ptr<winsock_guard> wg, SOCKET socket)
		: m_winsock(std::move(wg)), m_socket(socket)
	{
	}

	tcp_socket(tcp_socket && o)
		: m_winsock(o.m_winsock), m_socket(o.m_socket)
	{
		o.m_socket = INVALID_SOCKET;
	}

	~tcp_socket()
	{
		if (m_socket != INVALID_SOCKET)
			closesocket(m_socket);
	}

	aw::task<size_t> read_any(uint8_t * buf, size_t size) override
	{
		return aw::detail::make_command<sock_read_command>(m_socket, buf, size);
	}

	aw::task<size_t> write_any(uint8_t const * buf, size_t size) override
	{
		return aw::detail::make_command<sock_write_command>(m_socket, buf, size);
	}

private:
	std::shared_ptr<winsock_guard> m_winsock;
	SOCKET m_socket;
};

struct connect_impl
	: private aw::detail::scheduler::completion_sink
{
	typedef std::shared_ptr<aw::stream> value_type;

	connect_impl()
		: m_s(0), m_h(0), m_sink(nullptr)
	{
	}

	connect_impl(connect_impl && o)
		: m_winsock(o.m_winsock), m_h(o.m_h), m_s(o.m_s), m_sink(nullptr)
	{
		assert(o.m_sink == nullptr);
		o.m_winsock.reset();
		o.m_h = 0;
		o.m_s = 0;
	}

	~connect_impl()
	{
		if (m_h)
			CloseHandle(m_h);
		if (m_s)
			closesocket(m_s);
	}

	connect_impl & operator=(connect_impl && o)
	{
		std::swap(m_winsock, o.m_winsock);
		std::swap(m_s, o.m_s);
		std::swap(m_h, o.m_h);
		return *this;
	}

	aw::result<std::shared_ptr<aw::stream>> dismiss(aw::cancel_info ci)
	{
		return ci;
	}

	aw::task<value_type> start(aw::detail::scheduler & sch, aw::detail::task_completion<value_type> & sink)
	{
		sch.add_handle(m_h, *this);
		m_sink = &sink;
		return nullptr;
	}

	aw::result<value_type> cancel(aw::detail::scheduler & sch, aw::cancel_info ci)
	{
		sch.remove_handle(m_h, *this);
		return ci;
	}

	aw::task<std::shared_ptr<aw::stream>> complete()
	{
		if (WSAEventSelect(m_s, 0, 0) == SOCKET_ERROR)
			return std::make_exception_ptr(aw::detail::win32_error(WSAGetLastError()));

		std::shared_ptr<tcp_socket> ss = std::make_shared<tcp_socket>(m_winsock, m_s);
		m_s = 0;
		return aw::value(ss);
	}

	completion_result on_completion(aw::detail::scheduler & sch) override
	{
		WSANETWORKEVENTS ne;
		int r = WSAEnumNetworkEvents(m_s, m_h, &ne);
		if (r == SOCKET_ERROR)
			m_sink->on_completion(sch, std::make_exception_ptr(aw::detail::win32_error(WSAGetLastError())));

		if (ne.lNetworkEvents & FD_CONNECT)
		{
			r = ne.iErrorCode[FD_CONNECT_BIT];
			if (r != 0)
				m_sink->on_completion(sch, std::make_exception_ptr(aw::detail::win32_error(r)));
			else
				m_sink->on_completion(sch, this->complete());
		}
		else
		{
			sch.add_handle(m_h, *this);
		}

		return completion_result::finish;
	}

	std::shared_ptr<winsock_guard> m_winsock;
	SOCKET m_s;
	HANDLE m_h;
	aw::detail::task_completion<value_type> * m_sink;
};

aw::task<std::shared_ptr<aw::stream>> aw::tcp_connect(char const * host, uint16_t port) noexcept
{
	try
	{
		std::shared_ptr<winsock_guard> wg = g_winsock.get();

		ADDRINFOA * addrs;
		int r = getaddrinfo(host, nullptr, nullptr, &addrs);
		if (r == SOCKET_ERROR)
			return std::make_exception_ptr(detail::win32_error(r));
		addrinfo_guard addrs_guard(addrs);

		for (; addrs != nullptr; addrs = addrs->ai_next)
		{
			if (addrs->ai_family == AF_INET6 || addrs->ai_family == AF_INET)
				break;
		}

		if (addrs == nullptr)
			return std::make_exception_ptr(unknown_host_error());

		connect_impl imp;
		imp.m_winsock = wg;

		imp.m_s = WSASocketW(addrs->ai_family, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
		if (imp.m_s == INVALID_SOCKET)
			return std::make_exception_ptr(detail::win32_error(WSAGetLastError()));

		if (addrs->ai_family == AF_INET6)
		{
			sockaddr_in6 & addr = reinterpret_cast<sockaddr_in6 &>(*addrs->ai_addr);
			addr.sin6_port = htons(port);
		}
		else
		{
			sockaddr_in & addr = reinterpret_cast<sockaddr_in &>(*addrs->ai_addr);
			addr.sin_port = htons(port);
		}

		imp.m_h = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (!imp.m_h)
			return std::make_exception_ptr(detail::win32_error(GetLastError()));

		r = WSAEventSelect(imp.m_s, imp.m_h, FD_CONNECT);
		if (r == SOCKET_ERROR)
			return std::make_exception_ptr(detail::win32_error(WSAGetLastError()));

		r = connect(imp.m_s, addrs->ai_addr, addrs->ai_addrlen);
		if (r != SOCKET_ERROR)
			return imp.complete();

		r = WSAGetLastError();
		if (r != WSAEWOULDBLOCK)
			return std::make_exception_ptr(detail::win32_error(r));

		return aw::detail::make_command<connect_impl>(std::move(imp));
	}
	catch (...)
	{
		return std::current_exception();
	}
}

template <int af>
struct bind_all_address;

template <>
struct bind_all_address<AF_INET>
{
	typedef sockaddr_in addr_type;
	static void init(addr_type & addr, uint16_t port)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
	}
};

template <>
struct bind_all_address<AF_INET6>
{
	typedef sockaddr_in6 addr_type;
	static void init(addr_type & addr, uint16_t port)
	{
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(port);
	}
};

struct socket_guard
{
	explicit socket_guard(SOCKET sock)
		: m_sock(sock)
	{
	}

	socket_guard(socket_guard && o)
		: m_sock(o.m_sock)
	{
		o.m_sock = INVALID_SOCKET;
	}

	~socket_guard()
	{
		if (m_sock != INVALID_SOCKET)
			closesocket(m_sock);
	}

	operator SOCKET() const
	{
		return m_sock;
	}

private:
	SOCKET m_sock;
};

struct win32_handle
{
	explicit win32_handle(HANDLE h)
		: m_h(h)
	{
	}

	win32_handle(win32_handle && o)
		: m_h(o.m_h)
	{
		o.m_h = INVALID_HANDLE_VALUE;
	}

	~win32_handle()
	{
		if (m_h != INVALID_HANDLE_VALUE)
			CloseHandle(m_h);
	}

	operator HANDLE() const
	{
		return m_h;
	}

private:
	HANDLE m_h;
};

namespace aw {
namespace detail {

template <typename Cancel>
task<void> win32_wait_handle(HANDLE h, Cancel && cancel_fn)
{
	DWORD err = WaitForSingleObject(h, 0);
	if (err == WAIT_OBJECT_0)
		return aw::value();

	if (err != WAIT_TIMEOUT)
		return std::make_exception_ptr(aw::detail::win32_error(err));

	struct cmd
		: private scheduler::completion_sink
	{
		typedef void value_type;

		explicit cmd(HANDLE h, Cancel && cancel_fn)
			: m_h(h), m_sink(nullptr), m_cancel_fn(cancel_fn)
		{
		}

		result<void> dismiss(cancel_info ci)
		{
			return ci;
		}

		task<void> start(scheduler & sch, task_completion<void> & sink)
		{
			m_sink = &sink;
			sch.add_handle(m_h, *this);
			return nullptr;
		}

		result<void> cancel(aw::detail::scheduler & sch, cancel_info ci)
		{
			sch.remove_handle(m_h, *this);

			completion_result cr = m_cancel_fn();
			if (cr != completion_result::finish)
			{
				WaitForSingleObject(m_h, INFINITE);
				return aw::value();
			}

			return ci;
		}

	private:
		completion_result on_completion(scheduler & sch) override
		{
			m_sink->on_completion(sch, aw::value());
			return completion_result::finish;
		}

		HANDLE m_h;
		task_completion<void> * m_sink;
		Cancel m_cancel_fn;
	};

	return aw::detail::make_command<cmd>(h, std::move(cancel_fn));
}

}
}

template <int af>
static aw::task<void> listen_one(uint16_t port, std::function<void(std::shared_ptr<aw::stream> peer)> const & accept_fn)
{
	std::shared_ptr<winsock_guard> winsock = g_winsock.get();

	SOCKET sock = WSASocketW(af, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
	if (sock == INVALID_SOCKET)
		return std::make_exception_ptr(aw::detail::win32_error(WSAGetLastError()));
	socket_guard sg(sock);

	HANDLE ev = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!ev)
		return std::make_exception_ptr(aw::detail::win32_error(GetLastError()));
	win32_handle eg(ev);

	int r = WSAEventSelect(sock, ev, FD_ACCEPT);
	if (r == SOCKET_ERROR)
		return std::make_exception_ptr(aw::detail::win32_error(WSAGetLastError()));

	typename bind_all_address<af>::addr_type addr = {};
	bind_all_address<af>::init(addr, port);
	if (bind(sock, reinterpret_cast<sockaddr const *>(&addr), sizeof addr) == SOCKET_ERROR)
		return std::make_exception_ptr(aw::detail::win32_error(WSAGetLastError()));

	if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
		return std::make_exception_ptr(aw::detail::win32_error(WSAGetLastError()));

	struct Ctx
	{
		std::shared_ptr<winsock_guard> wg;
		socket_guard sock;
		win32_handle event;
	};

	return aw::loop(Ctx{ std::move(winsock), std::move(sg), std::move(eg) }, [](Ctx & ctx) {
		return aw::detail::win32_wait_handle(ctx.event, [] { return aw::detail::scheduler::completion_sink::completion_result::finish; });
	}, [accept_fn](Ctx & ctx) {
		WSANETWORKEVENTS ne;
		if (WSAEnumNetworkEvents(ctx.sock, ctx.event, &ne) == SOCKET_ERROR)
			throw aw::detail::win32_error(WSAGetLastError());

		if (ne.lNetworkEvents & FD_ACCEPT)
		{
			int err = ne.iErrorCode[FD_ACCEPT_BIT];
			if (err != 0)
				throw aw::detail::win32_error(err);
			SOCKET sock = accept(ctx.sock, nullptr, nullptr);
			if (sock == INVALID_SOCKET)
				throw aw::detail::win32_error(WSAGetLastError());

			accept_fn(std::make_shared<tcp_socket>(tcp_socket(ctx.wg, sock)));
		}
	});
}


aw::task<void> aw::tcp_listen(uint16_t port, std::function<void(std::shared_ptr<stream> peer)> const & accept) noexcept
{
	try
	{
		return listen_one<AF_INET>(port, accept);
	}
	catch (...)
	{
		return std::current_exception();
	}
}
