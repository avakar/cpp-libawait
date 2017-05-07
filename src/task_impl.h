#include <avakar/meta.h>
#include <utility>

namespace avakar {
namespace libawait {

namespace detail {

template <typename T>
struct task_dismisser
{
	template <size_t I>
	result<T> operator()(detail::variant_member<nulltask_t, I>, void *)
	{
		std::abort();
	}

	template <size_t I>
	result<T> operator()(detail::variant_member<command<T> *, I> m, void * storage)
	{
		result<T> r = m.get(storage)->cancel(nullptr);
		m.destruct(storage);
		detail::variant_member<nulltask_t>::construct(storage);
		return r;

	}

	template <typename U, size_t I>
	result<T> operator()(detail::variant_member<U, I> m, void * storage)
	{
		result<T> r{ in_place_type_t<U>(), std::move(m.get(storage)) };
		m.destruct(storage);
		detail::variant_member<nulltask_t>::construct(storage);
		return r;
	}
};

}

template <typename T>
task<T>::task()
	: task(in_place_type_t<T>())
{
}

template <typename T>
template <typename U>
task<T>::task(U && u)
	: task(in_place_type_t<meta::choose_overload_t<U, _implicit_types>>(), std::forward<U>(u))
{
}

template <typename T>
template <typename U, typename... Args>
task<T>::task(in_place_type_t<U>, Args &&... args)
	: index_(meta::index_of<U, _types>::value)
{
	detail::variant_member<U>::construct(&storage_, std::forward<Args>(args)...);
}

template <typename T>
task<T>::task(task && o)
	: index_(o.index_)
{
	detail::variant_visit<_types>(index_, [this, &o](auto m) {
		m.move(&storage_, &o.storage_);
		m.destruct(&o.storage_);
	});

	o.index_ = meta::index_of<nulltask_t, _types>::value;
	detail::variant_member<nulltask_t>::construct(&o.storage_);
}

template <typename T>
task<T> & task<T>::operator=(task && o)
{
	this->clear();

	index_ = o.index_;
	detail::variant_visit<_types>(index_, [this, &o](auto m) {
		m.move(&storage_, &o.storage_);
		m.destruct(&o.storage_);
	});

	o.index_ = meta::index_of<nulltask_t, _types>::value;
	detail::variant_member<nulltask_t>::construct(&o.storage_);
	return *this;
}

template <typename T>
task<T>::~task()
{
	this->clear();
}

template <typename T>
task<T>::operator bool() const
{
	return !this->empty();
}

template <typename T>
bool task<T>::empty() const
{
	return index_ == meta::index_of<nulltask_t, _types>::value;
}

template <typename T>
void task<T>::clear()
{
	if (!this->empty())
		this->dismiss();
}

template <typename T>
result<T> task<T>::dismiss()
{
	result<T> r = detail::variant_visit<_types>(index_, detail::task_dismisser<T>(), &storage_);
	index_ = meta::index_of<nulltask_t, _types>::value;
	return r;
}

}
}
