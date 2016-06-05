#include "../stream.h"
#include <memory>

aw::task<void> aw::write_all(aw::stream & s, uint8_t const * buf, size_t size)
{
	struct ctx
	{
		uint8_t const * buf;
		size_t size;
	};

	return loop(ctx{ buf, size }, [&s](ctx & c) {
		return c.size == 0? nullptr: s.write_any(c.buf, c.size);
	}, [](ctx & c, size_t r) {
		c.buf += r;
		c.size -= r;
	});
}
