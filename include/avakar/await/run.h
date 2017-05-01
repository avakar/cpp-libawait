#ifndef AVAKAR_LIBAWAIT_RUN_H
#define AVAKAR_LIBAWAIT_RUN_H

#include "task.h"

namespace avakar {
namespace libawait {

template <typename T>
result<T> try_run(task<T> && t);

template <typename T>
T run(task<T> && t);

}
}

#endif // AVAKAR_LIBAWAIT_RUN_H
