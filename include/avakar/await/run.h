#ifndef AWAIT_RUN_H
#define AWAIT_RUN_H

#include "task.h"

namespace aw {

template <typename T>
T run(task<T> && t);

template <typename T>
result<T> try_run(task<T> && t);

}

#include "../../../src/run_impl.h"

#endif // AWAIT_RUN_H
