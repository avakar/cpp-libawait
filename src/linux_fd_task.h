#ifndef AWAIT_DETAIL_LINUX_FD_TASK_H
#define AWAIT_DETAIL_LINUX_FD_TASK_H

#include <avakar/await/task.h>

namespace aw {
namespace detail {

task<short> linux_fd_task(int fd, short events);

}
}

#endif // AWAIT_DETAIL_LINUX_FD_TASK_H
