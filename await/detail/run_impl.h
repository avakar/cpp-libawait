#include <utility>

namespace aw {
namespace detail {

template <typename T>
struct uninitialized
{
	uninitialized()
	{
	}

	~uninitialized()
	{
		reinterpret_cast<T &>(m_storage).~T();
	}

	uninitialized(uninitialized const &) = delete;
	uninitialized & operator=(uninitialized const &) = delete;

	void construct(T const & v)
	{
		new(&m_storage) T(v);
	}

	void construct(T && v)
	{
		new(&m_storage) T(std::move(v));
	}

	T & value()
	{
		return reinterpret_cast<T &>(m_storage);
	}

private:
	typename std::aligned_union<0, T>::type m_storage;
};

result<void> try_run_impl(task<void> && t);

template <typename T>
result<T> try_run_impl(task<T> && t)
{
	assert(!t.empty());

	uninitialized<result<T>> r;
	try_run_impl(t.continue_with([&r](result<T> && v) -> task<void> {
		r.construct(std::move(v));
		return aw::value();
	}));

	return std::move(r.value());
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
