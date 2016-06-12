#ifndef AWAIT_DETAIL_COMMAND_H
#define AWAIT_DETAIL_COMMAND_H

#include "../task.h"
#include "task_vtable.h"
#include <exception>

namespace aw {
namespace detail {

template <typename I, typename... P>
task<typename I::value_type> make_command(P &&... p);

}
}

#include "command_impl.h"

#endif // AWAIT_DETAIL_COMMAND_H
