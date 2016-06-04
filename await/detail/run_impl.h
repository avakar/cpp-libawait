#include <utility>

namespace aw {
namespace detail {

result<void> try_run_impl(task<void> && t);

template <typename T>
result<T> try_run_impl(task<T> && t)
{
	assert(!t.empty());

	result<T> r;
	try_run_impl(t.continue_with([&r](result<T> && v) -> task<void> {
		r = std::move(v);
		return aw::value();
	}));

	return std::move(r);
}

}
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
	return detail::try_run_impl(std::move(t));
}
