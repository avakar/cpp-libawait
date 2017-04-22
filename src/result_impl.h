#include <utility>

namespace avakar {
namespace libawait {

namespace detail {

template <typename T0, typename... Tn>
struct overload_sandbox
	: overload_sandbox<Tn...>
{
	using overload_sandbox<Tn...>::f;
	static T0 f(T0);
};

template <typename T0>
struct overload_sandbox<T0>
{
	static T0 f(T0);
};

template <typename T, typename... Types>
using choose_overload_t = decltype(overload_sandbox<Types...>::f(std::declval<T>()));

}

template <typename T>
result<T>::result() noexcept
	: kind_(kind::value)
{
	new(&storage_) value_type();
}

template <typename T>
template <typename U, typename>
result<T>::result(U && u) noexcept
	: result(
		in_place_type_t<detail::choose_overload_t<U, T, std::error_code, std::exception_ptr>>(),
		std::forward<U>(u))
{
}

template <typename T>
template <typename... Args>
result<T>::result(in_place_type_t<T>, Args &&... args) noexcept
	: kind_(kind::value)
{
	new(&storage_) value_type(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
result<T>::result(in_place_type_t<std::error_code>, Args &&... args) noexcept
	: kind_(kind::error_code)
{
	new(&storage_) std::error_code(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
result<T>::result(in_place_type_t<std::exception_ptr>, Args &&... args) noexcept
	: kind_(kind::exception)
{
	new(&storage_) std::exception_ptr(std::forward<Args>(args)...);
}

template <typename T>
result<T>::result(result const & o) noexcept
	: kind_(o.kind_)
{
	switch (kind_)
	{
	case kind::value:
		new(&storage_) value_type(reinterpret_cast<value_type const &>(o.storage_));
		break;
	case kind::error_code:
		new(&storage_) std::error_code(reinterpret_cast<std::error_code const &>(o.storage_));
		break;
	case kind::exception:
		new(&storage_) std::exception_ptr(reinterpret_cast<std::exception_ptr const &>(o.storage_));
		break;
	}
}

template <typename T>
result<T>::result(result && o) noexcept
	: kind_(o.kind_)
{
	switch (kind_)
	{
	case kind::value:
		new(&storage_) value_type(reinterpret_cast<value_type &&>(o.storage_));
		break;
	case kind::error_code:
		new(&storage_) std::error_code(reinterpret_cast<std::error_code &&>(o.storage_));
		break;
	case kind::exception:
		new(&storage_) std::exception_ptr(reinterpret_cast<std::exception_ptr &&>(o.storage_));
		break;
	}
}

template <typename T>
template <typename U>
result<T>::result(result<U> const & o) noexcept
	: kind_(o.kind_)
{
	switch (kind_)
	{
	case kind::value:
		new(&storage_) value_type(reinterpret_cast<typename result<U>::value_type const &>(o.storage_));
		break;
	case kind::error_code:
		new(&storage_) std::error_code(reinterpret_cast<std::error_code const &>(o.storage_));
		break;
	case kind::exception:
		new(&storage_) std::exception_ptr(reinterpret_cast<std::exception_ptr const &>(o.storage_));
		break;
	}
}

template <typename T>
result<T>::~result()
{
	switch (kind_)
	{
	case kind::value:
		reinterpret_cast<value_type *>(&storage_)->~value_type();
		break;
	case kind::error_code:
		reinterpret_cast<std::error_code *>(&storage_)->~error_code();
		break;
	case kind::exception:
		reinterpret_cast<std::exception_ptr *>(&storage_)->~exception_ptr();
		break;
	}
}

template <typename T>
result<T>::operator bool() const noexcept
{
	return this->holds_value();
}

template <typename T>
bool result<T>::holds_value() const noexcept
{
	return kind_ == kind::value;
}

template <typename T>
bool result<T>::holds_error_code() const noexcept
{
	return kind_ == kind::error_code;
}

template <typename T>
bool result<T>::holds_exception() const noexcept
{
	return kind_ == kind::exception;
}

template <typename T>
auto result<T>::get()
	-> typename std::add_rvalue_reference<T>::type
{
	this->rethrow();
	return std::move(*reinterpret_cast<T *>(&storage_));
}

template <>
auto result<void>::get()
	-> typename std::add_rvalue_reference<void>::type
{
	this->rethrow();
}

template <typename T>
std::error_code result<T>::error_code() const noexcept
{
	return *reinterpret_cast<std::error_code const *>(&storage_);
}

template <typename T>
std::exception_ptr result<T>::exception() const noexcept
{
	return *reinterpret_cast<std::exception_ptr const *>(&storage_);
}

template <typename T>
void result<T>::rethrow() const
{
	switch (kind_)
	{
	case kind::error_code:
		throw std::system_error(this->error_code());
	case kind::exception:
		std::rethrow_exception(this->exception());
	}
}

} // namespace libawait
} // namespace avakar
