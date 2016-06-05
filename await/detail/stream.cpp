#include "../stream.h"

aw::task<void> aw::write_all(aw::stream & s, uint8_t const * buf, size_t size)
{
	// XXX
	return s.write_any(buf, size).ignore_result();
}
