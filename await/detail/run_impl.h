#include <utility>

template <typename T>
T aw::run(task<T> && t)
{
	assert(!t.empty());
	return try_run(std::move(t)).get();
}

template <typename T>
aw::result<T> aw::try_run(task<T> && t)
{
	assert(!t.empty());

	detail::task_vtable<T> const * vtable = detail::task_access::pull_vtable(t);
	void * storage = detail::task_access::storage(t);
	return vtable->get_result(storage);
}
