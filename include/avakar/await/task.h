#ifndef AVAKAR_LIBAWAIT_TASK_H
#define AVAKAR_LIBAWAIT_TASK_H

#include "result.h"
#include "../../../src/command_intf.h"
#include <avakar/meta.h>
#include "../../../src/variant_storage.h"
#include <system_error>
#include <exception>

namespace avakar {
namespace libawait {

struct nulltask_t {};
constexpr nulltask_t nulltask{};

template <typename T>
struct task
{
	static_assert(
		std::is_same<std::decay_t<T>, T>::value,
		"T shall not be cv-qualified, nor of reference, array or function type");
	static_assert(!std::is_same<T, nulltask_t>::value, "T shall not be nulltask_t");
	static_assert(!std::is_same<T, std::error_code>::value, "T shall not be std::error_code");
	static_assert(!std::is_same<T, std::exception_ptr>::value, "T shall not be std::exception_ptr");

	task();

	template <typename U>
	task(U && u);

	template <typename U, typename... Args>
	task(in_place_type_t<U>, Args &&... args);

	task(task const & o) = delete;
	task(task && o);

	task & operator=(task const & o) = delete;
	task & operator=(task && o);

	~task();

	explicit operator bool() const;
	bool empty() const;

	void clear();
	result<T> dismiss();

private:
	using _types = meta::list<nulltask_t, T, std::error_code, std::exception_ptr, detail::command<T> *>;
	using _implicit_types = meta::list<nulltask_t, T, std::error_code, std::exception_ptr>;

	size_t index_;
	detail::variant_storage_t<_types> storage_;
};

}
}

#include "../../../src/task_impl.h"

#endif // AVAKAR_LIBAWAIT_TASK_H
