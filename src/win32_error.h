#ifndef AWAIT_DETAIL_WIN32_ERROR_H
#define AWAIT_DETAIL_WIN32_ERROR_H

#include <exception>
#include <stdint.h>

namespace aw {
namespace detail {

struct win32_error
	: std::exception
{
	explicit win32_error(uint32_t ec);
	win32_error(win32_error const & o);
	win32_error(win32_error && o);
	~win32_error();

	win32_error & operator=(win32_error o);

	char const * what() const override;
	uint32_t errorcode() const;

private:
	uint32_t m_ec;
	char const * m_msg;
};

}
}

#endif // AWAIT_DETAIL_WIN32_ERROR_H
