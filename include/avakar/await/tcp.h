#ifndef AWAIT_TCP_H
#define AWAIT_TCP_H

#include "task.h"
#include "stream.h"
#include <memory>
#include <stdint.h>
#include <exception>
#include <functional>

namespace aw {

struct unknown_host_error
	: std::exception
{
	char const * what() const noexcept override
	{
		return "unknown_host_error";
	}
};

task<std::shared_ptr<stream>> tcp_connect(char const * host, uint16_t port) noexcept;
task<void> tcp_listen(uint16_t port, std::function<void(std::shared_ptr<stream> peer)> const & accept) noexcept;

}

#endif // AWAIT_TCP_H
