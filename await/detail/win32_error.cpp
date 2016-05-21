#include "win32_error.h"
#include <windows.h>

aw::detail::win32_error::win32_error(uint32_t ec)
	: m_ec(ec)
{
	char buf[65536];
	DWORD r = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, ec, 0x0409, buf, sizeof buf, nullptr);
	if (r != 0)
		m_msg.assign(buf, r);
}

char const * aw::detail::win32_error::what() const
{
	return m_msg.empty()? "win32_error": m_msg.c_str();
}

uint32_t aw::detail::win32_error::errorcode() const
{
	return m_ec;
}
