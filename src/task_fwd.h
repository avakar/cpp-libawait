#ifndef AWAIT_DETAIL_TASK_FWD_H
#define AWAIT_DETAIL_TASK_FWD_H

namespace aw {

template <typename T>
struct task;

namespace detail {

enum class task_kind { empty, value, exception, command };

struct task_access;

template <typename T>
struct command;

}
}

#endif // AWAIT_DETAIL_TASK_FWD_H
