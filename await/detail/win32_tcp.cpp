#include "../tcp.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <singleton/singleton.h>
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

singleton<winsock_guard> g_winsock;

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

struct ssocket
	: aw::stream
{
	ssocket(std::shared_ptr<winsock_guard> wg, SOCKET socket)
		: m_winsock(std::move(wg)), m_socket(socket)
	{
	}

	~ssocket()
	{
		if (m_socket)
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

	aw::task<value_type> start(aw::detail::scheduler & sch, aw::detail::task_completion<value_type> & sink)
	{
		sch.add_handle(m_h, *this);
		m_sink = &sink;
		return nullptr;
	}

	aw::task<std::shared_ptr<aw::stream>> complete()
	{
		if (WSAEventSelect(m_s, 0, 0) == SOCKET_ERROR)
			return std::make_exception_ptr(aw::detail::win32_error(WSAGetLastError()));

		std::shared_ptr<ssocket> ss = std::make_shared<ssocket>(m_winsock, m_s);
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

		imp.m_s = WSASocket(addrs->ai_family, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
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
