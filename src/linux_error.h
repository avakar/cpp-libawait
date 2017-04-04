#ifndef AWAIT_DETAIL_LINUX_ERROR_H
#define AWAIT_DETAIL_LINUX_ERROR_H

#include <exception>

namespace aw {
namespace detail {

struct linux_error
	: std::exception
{
	explicit linux_error(int err)
		: m_err(err)
	{
	}

	char const * what() const noexcept override
	{
		return "linux_error";
	}

	int m_err;
};

}
}

#endif // AWAIT_DETAIL_LINUX_ERROR_H
