#ifndef AWAIT_TIMER_H
#define AWAIT_TIMER_H

#include "task.h"
#include <stdint.h>

namespace aw {

task<void> wait_ms(int64_t ms);

}

#endif // AWAIT_TIMER_H
