#ifndef AWAIT_DETAIL_THEN_TRAITS_H
#define AWAIT_DETAIL_THEN_TRAITS_H

#include "task_traits.h"

namespace aw {
namespace detail {

template <typename T, typename F>
struct then_traits
{
	typedef decltype(std::declval<F>()(std::declval<T>())) return_type;

	typedef typename task_traits<return_type>::value_type result_type;
	typedef task<result_type> task_type;

	static task_type invoke(result<T> && t, F && f);
};

template <typename F>
struct then_traits<void, F>
{
	typedef decltype(std::declval<F>()()) return_type;

	typedef typename task_traits<return_type>::value_type result_type;
	typedef task<result_type> task_type;

	static task_type invoke(result<void> && t, F && f);
};

}
}

#endif // AWAIT_DETAIL_THEN_TRAITS_H
