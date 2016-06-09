#ifndef AWAIT_DETAIL_THEN_TRAITS_H
#define AWAIT_DETAIL_THEN_TRAITS_H

#include "task_traits.h"

namespace aw {
namespace detail {

template <typename T, typename F>
struct continue_with_traits
{
	typedef typename invoke_and_taskify_traits<F, result<T>>::task_type return_type;
};

template <typename T, typename F>
struct then_traits
{
	typedef typename invoke_and_taskify_traits<F, T>::task_type return_type;
	static return_type invoke(result<T> && t, F && f) noexcept;
};

template <typename F>
struct then_traits<void, F>
{
	typedef typename invoke_and_taskify_traits<F>::task_type return_type;
	static return_type invoke(result<void> && t, F && f) noexcept;
};

}
}

#endif // AWAIT_DETAIL_THEN_TRAITS_H
