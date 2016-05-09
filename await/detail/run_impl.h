#include <utility>

namespace aw {
namespace detail {

template <typename T>
result<T> to_result(task<T> && t);

result<void> to_result(task<void> && t);

}
}

template <typename T>
aw::result<T> aw::detail::to_result(task<T> && t)
{
	detail::task_kind kind = detail::task_access::get_kind(t);
	if (kind == detail::task_kind::exception)
		return aw::result<T>(std::move(detail::task_access::as_exception(t)));

	assert(kind == detail::task_kind::value);
	return aw::result<T>::from_value(std::move(detail::task_access::as_value(t)));
}

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
	return detail::to_result(std::move(t));
}
