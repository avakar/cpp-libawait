#include <await/fiber.h>
#include <mutest/test.h>
#include <await/tcp.h>
#include <await/run.h>

TEST("aw::fiber should work")
{
	aw::task<size_t> t = aw::fiber([]() -> size_t {
		std::shared_ptr<aw::stream> s = awwait aw::tcp_connect("www.google.com", 80);

		static uint8_t const msg[] = "GET / HTTP/1.1\n\rHost: www.google.com\r\n\r\n";
		awwait write_all(*s, msg, sizeof msg - 1);

		uint8_t buf[1024];
		size_t r = awwait s->read_any(buf, sizeof buf);
		return r;
	});

	aw::run(std::move(t));
}
