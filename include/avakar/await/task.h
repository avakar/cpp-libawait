#ifndef AVAKAR_LIBAWAIT_TASK_H
#define AVAKAR_LIBAWAIT_TASK_H

#include "result.h"
#include "../../../src/meta.h"
#include "../../../src/variant_storage.h"
#include <system_error>
#include <exception>

namespace avakar {
namespace libawait {

template <typename T>
struct task;

struct scheduler;

template <typename T>
struct task_completion
{
	virtual void on_completion(scheduler & sch, task<T> && t) = 0;
};

template <typename T>
struct command
{
	virtual task<T> start(scheduler & sch, task_completion<T> & sink) noexcept = 0;
	virtual result<T> cancel(scheduler * sch) noexcept = 0;
};

struct nulltask_t {};
constexpr nulltask_t nulltask{};

template <typename T>
struct task
{
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
	using _types = _meta::list<nulltask_t, T, std::error_code, std::exception_ptr, command<T> *>;
	using _implicit_types = _meta::list<nulltask_t, T, std::error_code, std::exception_ptr>;

	size_t index_;
	detail::variant_storage_t<_types> storage_;
};

}
}

#include "../../../src/task_impl.h"

#endif // AVAKAR_LIBAWAIT_TASK_H
