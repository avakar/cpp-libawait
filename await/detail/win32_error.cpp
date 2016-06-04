#include "win32_error.h"
#include <utility>
#include <windows.h>

aw::detail::win32_error::win32_error(uint32_t ec)
	: m_ec(ec)
{
	char * buf = nullptr;
	DWORD r = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, ec, 0, (LPSTR)&buf, 0, nullptr);
	if (r != 0)
		m_msg = buf;
}

aw::detail::win32_error::win32_error(win32_error const & o)
	: m_ec(o.m_ec), m_msg(nullptr)
{
	if (o.m_msg != nullptr)
	{
		size_t len = strlen(o.m_msg);
		char * buf = (char *)LocalAlloc(0, len + 1);
		if (buf != nullptr)
		{
			memcpy(buf, o.m_msg, len + 1);
			m_msg = buf;
		}
	}
}

aw::detail::win32_error::win32_error(win32_error && o)
	: m_ec(o.m_ec), m_msg(o.m_msg)
{
	o.m_msg = nullptr;
}

aw::detail::win32_error::~win32_error()
{
	if (m_msg)
		LocalFree((HLOCAL)m_msg);
}

aw::detail::win32_error & aw::detail::win32_error::operator=(win32_error o)
{
	m_ec = o.m_ec;
	std::swap(m_msg, o.m_msg);
	return *this;
}

char const * aw::detail::win32_error::what() const
{
	return m_msg? m_msg: "win32_error";
}

uint32_t aw::detail::win32_error::errorcode() const
{
	return m_ec;
}
