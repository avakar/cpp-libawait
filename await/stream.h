#ifndef AWAIT_STREAM_H
#define AWAIT_STREAM_H

#include "task.h"
#include <stddef.h>
#include <stdint.h>

namespace aw {

struct stream
{
	virtual task<size_t> read_any(uint8_t * buf, size_t size) = 0;
	virtual task<size_t> write_any(uint8_t const * buf, size_t size) = 0;
};

task<void> write_all(stream & s, uint8_t const * buf, size_t size);

}


#endif // AWAIT_STREAM_H
