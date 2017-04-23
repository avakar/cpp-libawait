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

template <typename U, typename T>
struct choose_result_overload
{
	using type = choose_overload_t<U, T, std::error_code, std::exception_ptr>;
};

template <typename U>
struct choose_result_overload<U, void>
{
	using type = choose_overload_t<U, std::error_code, std::exception_ptr>;
};

template <typename U, typename T>
using choose_result_overload_t = typename choose_result_overload<U, T>::type;

}

template <typename T>
result<T>::result() noexcept
	: kind_(detail::result_kind::value)
{
	new(&storage_) value_type();
}

template <typename T>
template <typename U, typename>
result<T>::result(U && u) noexcept
	: result(
		in_place_type_t<detail::choose_result_overload_t<U, T>>(),
		std::forward<U>(u))
{
}

template <typename T>
template <typename... Args>
result<T>::result(in_place_type_t<T>, Args &&... args) noexcept
	: kind_(detail::result_kind::value)
{
	new(&storage_) value_type(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
result<T>::result(in_place_type_t<std::error_code>, Args &&... args) noexcept
	: kind_(detail::result_kind::error_code)
{
	new(&storage_) std::error_code(std::forward<Args>(args)...);
}

template <typename T>
template <typename... Args>
result<T>::result(in_place_type_t<std::exception_ptr>, Args &&... args) noexcept
	: kind_(detail::result_kind::exception)
{
	new(&storage_) std::exception_ptr(std::forward<Args>(args)...);
}

template <typename T>
result<T>::result(result const & o) noexcept
	: kind_(o.kind_)
{
	o.visit([&](auto const & v) {
		using T = std::decay_t<decltype(v)>;
		new(&storage_) T(v);
	});
}

template <typename T>
result<T>::result(result && o) noexcept
	: kind_(o.kind_)
{
	o.visit([&](auto && v) {
		using T = std::decay_t<decltype(v)>;
		new(&storage_) T(std::move(v));
	});
}

template <typename T>
template <typename U>
result<T>::result(result<U> const & o) noexcept
	: kind_(o.kind_)
{
	o.visit([&](auto const & v) {
		using T = std::decay_t<decltype(v)>;
		new(&storage_) T(v);
	});
}

template <typename T>
template <typename U>
result<T>::result(result<U> && o) noexcept
	: kind_(o.kind_)
{
	o.visit([&](auto && v) {
		using T = std::decay_t<decltype(v)>;
		new(&storage_) T(std::move(v));
	});
}

template <typename T>
result<T>::~result()
{
	this->visit([](auto const & v) {
		using T = std::decay_t<decltype(v)>;
		v.~T();
	});
}

template <typename T>
T * result<T>::operator->()
{
	this->rethrow();
	return reinterpret_cast<value_type *>(&storage_);
}

template <typename T>
T const * result<T>::operator->() const
{
	this->rethrow();
	return reinterpret_cast<value_type const *>(&storage_);
}

template <typename T>
std::add_lvalue_reference_t<T> result<T>::operator*() &
{
	this->rethrow();
	return reinterpret_cast<value_type &>(storage_);
}

template <>
inline std::add_lvalue_reference_t<void> result<void>::operator*() &
{
	this->rethrow();
}

template <typename T>
std::add_rvalue_reference_t<T> result<T>::operator*() &&
{
	this->rethrow();
	return reinterpret_cast<value_type &&>(storage_);
}

template <>
inline std::add_rvalue_reference_t<void> result<void>::operator*() &&
{
	this->rethrow();
}

template <typename T>
std::add_lvalue_reference_t<T const> result<T>::operator*() const &
{
	this->rethrow();
	return reinterpret_cast<value_type const &>(storage_);
}

template <>
inline std::add_lvalue_reference_t<void const> result<void>::operator*() const &
{
	this->rethrow();
}

template <typename T>
std::add_rvalue_reference_t<T const> result<T>::operator*() const &&
{
	this->rethrow();
	return reinterpret_cast<value_type const &&>(storage_);
}

template <>
inline std::add_rvalue_reference_t<void const> result<void>::operator*() const &&
{
	this->rethrow();
}

template <typename T>
result<T>::operator bool() const noexcept
{
	return this->has_value();
}

template <typename T>
bool result<T>::has_value() const noexcept
{
	return kind_ == detail::result_kind::value;
}

template <typename T>
bool result<T>::has_error_code() const noexcept
{
	return kind_ == detail::result_kind::error_code;
}

template <typename T>
bool result<T>::has_exception() const noexcept
{
	return kind_ == detail::result_kind::exception;
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
	case detail::result_kind::error_code:
		throw std::system_error(this->error_code());
	case detail::result_kind::exception:
		std::rethrow_exception(this->exception());
	}
}

template <typename T>
template <typename Visitor>
void result<T>::visit(Visitor && vis)
{
	switch (kind_)
	{
	case detail::result_kind::value:
		std::forward<Visitor>(vis)(reinterpret_cast<value_type &&>(storage_));
		break;
	case detail::result_kind::error_code:
		std::forward<Visitor>(vis)(reinterpret_cast<std::error_code &&>(storage_));
		break;
	case detail::result_kind::exception:
		std::forward<Visitor>(vis)(reinterpret_cast<std::exception_ptr &&>(storage_));
		break;
	}
}

template <typename T>
template <typename Visitor>
void result<T>::visit(Visitor && vis) const
{
	switch (kind_)
	{
	case detail::result_kind::value:
		std::forward<Visitor>(vis)(reinterpret_cast<value_type const &>(storage_));
		break;
	case detail::result_kind::error_code:
		std::forward<Visitor>(vis)(reinterpret_cast<std::error_code const &>(storage_));
		break;
	case detail::result_kind::exception:
		std::forward<Visitor>(vis)(reinterpret_cast<std::exception_ptr const &>(storage_));
		break;
	}
}

} // namespace libawait
} // namespace avakar
