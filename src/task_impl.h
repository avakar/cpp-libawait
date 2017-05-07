#include <avakar/meta.h>
#include <utility>

namespace avakar {
namespace libawait {

namespace detail {

template <typename T>
struct task_dismisser
{
	result<T> operator()(meta::item<nulltask_t>, void *)
	{
		std::abort();
	}

	result<T> operator()(meta::item<command<T> *> m, void * storage)
	{
		result<T> r = get(m, storage)->cancel(nullptr);
		destroy_member(m, storage);
		new(storage) nulltask_t;
		return r;

	}

	template <typename U>
	result<T> operator()(meta::item<U> m, void * storage)
	{
		result<T> r{ in_place_type_t<U>(), std::move(get(m, storage)) };
		detail::destroy_member(m, storage);
		new(storage) nulltask_t;
		return r;
	}

	result<T> operator()(meta::item<void> m, void * storage)
	{
		result<T> r{ in_place_type_t<void>() };
		new(storage) nulltask_t;
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
	detail::construct_member(meta::item<U>(), &storage_, std::forward<Args>(args)...);
}

template <typename T>
task<T>::task(task && o)
	: index_(o.index_)
{
	meta::visit<_types>(index_, [this, &o](auto m) {
		detail::move_construct_member(m, &storage_, m, &o.storage_);
		detail::destroy_member(m, &o.storage_);
	});

	o.index_ = meta::index_of<nulltask_t, _types>::value;
	new(&o.storage_) nulltask_t;
}

template <typename T>
task<T> & task<T>::operator=(task && o)
{
	this->clear();

	index_ = o.index_;
	meta::visit<_types>(index_, [this, &o](auto m) {
		detail::move_construct_member(m, &storage_, m, &o.storage_);
		detail::destroy_member(m, &o.storage_);
	});

	o.index_ = meta::index_of<nulltask_t, _types>::value;
	new(&o.storage_) nulltask_t;
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
	assert(!this->empty());

	result<T> r = meta::visit<_types>(index_, detail::task_dismisser<T>(), &storage_);
	index_ = meta::index_of<nulltask_t, _types>::value;
	return r;
}

}
}
