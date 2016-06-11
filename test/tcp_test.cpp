#include <await/tcp.h>
#include <mutest/test.h>
#include <await/run.h>

#if 0
aw::task<size_t> go(uint8_t * buf, size_t size)
{
	std::shared_ptr<aw::stream> s = co_await aw::tcp_connect("www.google.com", 80);

	static uint8_t const msg[] = "GET / HTTP/1.1\n\rHost: www.google.com\r\n\r\n";
	co_await write_all(*s, msg, sizeof msg - 1);

	size_t r = co_await s->read_any(buf, size);

	r = r;
	return r;
}
#endif

TEST("aw::tcp_connect should work")
{
	uint8_t buf[1024];

	aw::task<size_t> t = aw::tcp_connect("www.google.com", 80).then([&buf](std::shared_ptr<aw::stream> s) {
		static char const msg[] = "GET / HTTP/1.1\n\rHost: www.google.com\r\n\r\n";
		return write_all(*s, (uint8_t const *)msg, sizeof msg - 1).then([&buf, s]() {
			return s->read_any(buf, sizeof buf).hold(s);
		});
	});

	size_t s = aw::run(std::move(t));
	(void)s;
}
