#include <avakar/meta.h>
#include <utility>
#include <assert.h>

namespace avakar {
namespace libawait {

template <typename T>
result<T>::result() noexcept
	: result(in_place_type_t<T>())
{
}

template <typename T>
template <typename U, typename>
result<T>::result(U && u) noexcept
	: result(
		in_place_type_t<meta::choose_overload_t<U, _types>>(),
		std::forward<U>(u))
{
}

template <typename T>
template <typename... Args>
result<T>::result(in_place_t, Args &&... args) noexcept
	: result(in_place_type_t<T>(), std::forward<Args>(args)...)
{
}

template <typename T>
template <typename U, typename... Args, typename>
result<T>::result(in_place_type_t<U>, Args &&... args) noexcept
	: index_(meta::index_of<U, _types>::value)
{
	if (!detail::construct_member(meta::item<U>(), &storage_, std::forward<Args>(args)...))
		index_ = meta::index_of<std::exception_ptr, _types>::value;
}

template <typename T>
result<T>::result(result const & o) noexcept
	: index_(o.index_)
{
	meta::visit<_types>(index_, [this, &o](auto m) {
		if (!detail::copy_construct_member(m, &storage_, m, &o.storage_))
			index_ = meta::index_of<std::exception_ptr, _types>::value;
	});
}

template <typename T>
result<T>::result(result && o) noexcept
	: index_(o.index_)
{
	meta::visit<_types>(index_, [this, &o](auto m) {
		if (!detail::move_construct_member(m, &storage_, m, &o.storage_))
			index_ = meta::index_of<std::exception_ptr, _types>::value;
	});
}

template <typename T>
template <typename U>
result<T>::result(result<U> const & o) noexcept
	: index_(o.index_)
{
	meta::visit<_types>(index_, [this, &o](auto m) {
		using M = decltype(m);
		using O = meta::sub_t<typename result<U>::_types, M::index>;

		if (!detail::copy_construct_member(m, &storage_, meta::item<O>(), &o.storage_))
			index_ = meta::index_of<std::exception_ptr, _types>::value;
	});
}

template <typename T>
template <typename U>
result<T>::result(result<U> && o) noexcept
	: index_(o.index_)
{
	meta::visit<_types>(index_, [this, &o](auto m) {
		using M = decltype(m);
		using O = meta::sub_t<typename result<U>::_types, M::index>;

		if (!detail::move_construct_member(m, &storage_, meta::item<O>(), &o.storage_))
			index_ = meta::index_of<std::exception_ptr, _types>::value;
	});
}

template <typename T>
result<T> & result<T>::operator=(result<T> const & o)
{
	meta::visit<_types>(index_, [this](auto m) {
		detail::destroy_member(m, &storage_);
	});

	index_ = o.index_;
	meta::visit<_types>(index_, [this, &o](auto m) {
		if (!detail::copy_construct_member(m, &storage_, m, &o.storage_))
			index_ = meta::index_of<std::exception_ptr, _types>::value;
	});

	return *this;
}

template <typename T>
result<T> & result<T>::operator=(result<T> && o)
{
	meta::visit<_types>(index_, [this](auto m) {
		detail::destroy_member(m, &storage_);
	});

	index_ = o.index_;
	meta::visit<_types>(index_, [this, &o](auto m) {
		if (!detail::move_construct_member(m, &storage_, m, &o.storage_))
			index_ = meta::index_of<std::exception_ptr, _types>::value;
	});

	return *this;
}

template <typename T>
result<T>::~result()
{
	meta::visit<_types>(index_, [this](auto m) {
		detail::destroy_member(m, &storage_);
	});
}

template <typename T>
T * result<T>::operator->()
{
	assert(this->has_value());
	return aw::get_if<T>(*this);
}

template <typename T>
T const * result<T>::operator->() const
{
	assert(this->has_value());
	return aw::get_if<T>(*this);
}

template <typename T>
std::add_lvalue_reference_t<T> result<T>::operator*() &
{
	assert(this->has_value());
	return aw::get<T>(*this);
}

template <typename T>
std::add_rvalue_reference_t<T> result<T>::operator*() &&
{
	assert(this->has_value());
	return aw::get<T>(std::move(*this));
}

template <typename T>
std::add_lvalue_reference_t<T const> result<T>::operator*() const &
{
	assert(this->has_value());
	return aw::get<T>(*this);
}

template <typename T>
std::add_rvalue_reference_t<T const> result<T>::operator*() const &&
{
	assert(this->has_value());
	return aw::get<T>(std::move(*this));
}

template <typename T>
std::add_lvalue_reference_t<T> result<T>::value() &
{
	this->rethrow();
	return aw::get<T>(*this);
}

template <typename T>
std::add_lvalue_reference_t<T const> result<T>::value() const &
{
	this->rethrow();
	return aw::get<T>(*this);
}

template <typename T>
std::add_rvalue_reference_t<T> result<T>::value() &&
{
	this->rethrow();
	return aw::get<T>(std::move(*this));
}

template <typename T>
std::add_rvalue_reference_t<T const> result<T>::value() const &&
{
	this->rethrow();
	return aw::get<T>(std::move(*this));
}

template <typename T>
result<T>::operator bool() const noexcept
{
	return this->has_value();
}

template <typename T>
bool result<T>::has_value() const noexcept
{
	return holds_alternative<T>(*this);
}

template <typename T>
auto result<T>::get()
	-> typename std::add_rvalue_reference<T>::type
{
	this->rethrow();
	return std::move(*reinterpret_cast<T *>(&storage_));
}

template <>
inline auto result<void>::get()
	-> typename std::add_rvalue_reference<void>::type
{
	this->rethrow();
}

template <typename T>
std::exception_ptr result<T>::exception() const noexcept
{
	struct X
	{
		std::exception_ptr operator()(std::error_code const & err)
		{
			return std::make_exception_ptr(std::system_error(err));
		}

		std::exception_ptr operator()(std::exception_ptr const & exc)
		{
			return exc;
		}
	};

	if (!this->has_value())
		return visit_error(X());
	return nullptr;
}

template <typename T>
template <typename Visitor>
detail::visit_error_result_t<Visitor> result<T>::visit_error(Visitor && vis) const
{
	assert(!this->has_value());

	switch (index_)
	{
	case 1:
		return std::forward<Visitor>(vis)(*reinterpret_cast<std::error_code const *>(&storage_));
	case 2:
		return std::forward<Visitor>(vis)(*reinterpret_cast<std::exception_ptr const *>(&storage_));
	}

	std::abort();
}

template <typename T>
void result<T>::rethrow() const
{
	std::exception_ptr exc = this->exception();
	if (exc)
		std::rethrow_exception(exc);
}

template <typename U, typename T>
bool holds_alternative(result<T> const & o) noexcept
{
	return detail::result_storage::index(o) == meta::index_of<U, detail::result_types<T>>::value;
}

template <typename U, typename T>
U const * get_if(result<T> const & o) noexcept
{
	return holds_alternative<U>(o) ? reinterpret_cast<U const *>(detail::result_storage::get(o)) : nullptr;
}

template <typename U, typename T>
U const * get_if(result<T> const && o) noexcept
{
	return holds_alternative<U>(o) ? reinterpret_cast<U const *>(detail::result_storage::get(o)) : nullptr;
}

template <typename U, typename T>
U * get_if(result<T> & o) noexcept
{
	return holds_alternative<U>(o) ? reinterpret_cast<U *>(detail::result_storage::get(o)) : nullptr;
}

template <typename U, typename T>
U * get_if(result<T> && o) noexcept
{
	return holds_alternative<U>(o) ? reinterpret_cast<U *>(detail::result_storage::get(o)) : nullptr;
}

template <typename U, typename T>
std::add_lvalue_reference_t<U const> get(result<T> const & o) noexcept
{
	assert(holds_alternative<U>(o));
	return *reinterpret_cast<U const *>(detail::result_storage::get(o));
}

template <typename U, typename T>
std::add_rvalue_reference_t<U const> get(result<T> const && o) noexcept
{
	assert(holds_alternative<U>(o));
	return std::move(*reinterpret_cast<U const *>(detail::result_storage::get(o)));
}

template <typename U, typename T>
std::add_lvalue_reference_t<U> get(result<T> & o) noexcept
{
	assert(holds_alternative<U>(o));
	return *reinterpret_cast<U *>(detail::result_storage::get(o));
}

template <typename U, typename T>
std::add_rvalue_reference_t<U> get(result<T> && o) noexcept
{
	assert(holds_alternative<U>(o));
	return std::move(*reinterpret_cast<U *>(detail::result_storage::get(o)));
}

template <>
inline std::add_lvalue_reference_t<void const> get<void>(result<void> const & o) noexcept
{
	assert(holds_alternative<void>(o));
}

template <>
inline std::add_rvalue_reference_t<void const> get<void>(result<void> const && o) noexcept
{
	assert(holds_alternative<void>(o));
}

template <>
inline std::add_lvalue_reference_t<void> get<void>(result<void> & o) noexcept
{
	assert(holds_alternative<void>(o));
}

template <>
inline std::add_rvalue_reference_t<void> get<void>(result<void> && o) noexcept
{
	assert(holds_alternative<void>(o));
}

template <typename T>
template <typename U>
result<U> result<T>::convert_error()
{
	return this->visit_error([](auto const & err) {
		using E = std::decay_t<decltype(err)>;
		return result<U>(in_place_type_t<E>(), err);
	});
}

template <typename T>
bool result<T>::operator==(result const & rhs) const
{
	if (index_ != rhs.index_)
		return false;

	return meta::visit<_types>(index_, [this, &rhs](auto m) -> bool {
		return detail::equal_member(m, &storage_, m, &rhs.storage_);
	});
}

template <typename T>
bool result<T>::operator!=(result const & rhs) const
{
	return !(*this == rhs);
}

template <typename T, typename U>
bool operator==(U && lhs, result<T> const & rhs)
{
	return rhs == std::forward<U>(lhs);
}

template <typename T, typename U>
bool operator!=(U && lhs, result<T> const & rhs)
{
	return !(std::forward<U>(lhs) == rhs);
}

} // namespace libawait
} // namespace avakar
