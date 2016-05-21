#ifndef AWAIT_DETAIL_WIN32_ERROR_H
#define AWAIT_DETAIL_WIN32_ERROR_H

#include <exception>
#include <stdint.h>
#include <string>

namespace aw {
namespace detail {

struct win32_error
	: std::exception
{
	explicit win32_error(uint32_t ec);

	char const * what() const override;
	uint32_t errorcode() const;

private:
	uint32_t m_ec;
	std::string m_msg;
};

}
}

#endif // AWAIT_DETAIL_WIN32_ERROR_H
