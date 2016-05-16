#ifndef AWAIT_DETAIL_TASK_VTABLE_H
#define AWAIT_DETAIL_TASK_VTABLE_H

#include "../result.h"

namespace aw {
namespace detail {

template <typename T>
struct task_vtable
{
	result<T> (*get_result)(void * self);
	void (*move_to)(void * self, void * dst);
	void (*destroy)(void * self);
};

}
}

#endif // AWAIT_DETAIL_TASK_VTABLE_H
