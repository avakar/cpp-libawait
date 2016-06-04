#ifndef AWAIT_COROUTINE_H
#define AWAIT_COROUTINE_H

#include "task.h"
#include <experimental/coroutine>

namespace aw {
namespace detail {

struct coro_suspend_completion
{
	virtual void on_completion(aw::detail::scheduler & sch) = 0;
};

struct coro_suspend_base
{
	virtual void start(aw::detail::scheduler & sch, coro_suspend_completion & sink) = 0;
};

template <typename T>
struct coro_task_promise
{
	struct impl
		: private coro_suspend_completion
	{
		typedef T value_type;

		explicit impl(coro_task_promise & promise)
			: m_promise(promise), m_susp(nullptr)
		{
			m_promise.m_impl = this;
		}

		aw::task<T> start(aw::detail::scheduler & sch, aw::detail::task_completion<T> & sink)
		{
			assert(m_susp != nullptr);

			m_sink = &sink;
			m_susp->start(sch, *this);
			return nullptr;
		}

		void on_completion(aw::detail::scheduler & sch) override
		{
			m_susp = nullptr;
			std::experimental::coroutine_handle<coro_task_promise>::from_promise(&m_promise).resume();

			if(m_susp != nullptr)
				m_susp->start(sch, *this);
			else
				m_sink->on_completion(sch, aw::task<T>(std::move(m_result)));
		}

	private:
		friend coro_task_promise;

		coro_task_promise & m_promise;
		coro_suspend_base * m_susp;
		aw::result<T> m_result;
		aw::detail::task_completion<T> * m_sink;

		impl(impl && o) = delete;
		impl & operator=(impl && o) = delete;
	};

	coro_task_promise()
		: m_impl(nullptr)
	{
	}

	auto initial_suspend() { return std::experimental::suspend_never(); }
	auto final_suspend() { return std::experimental::suspend_never(); }

	aw::task<T> get_return_object()
	{

		return aw::detail::make_command<impl>(*this);
	}

	void return_value(T t)
	{
		assert(m_impl);
		m_impl->m_result = aw::result<T>(in_place, std::move(t));
	}

	void set_exception(std::exception_ptr e)
	{
		assert(m_impl);
		m_impl->m_result = std::move(e);
	}

	void set_susp(coro_suspend_base * susp)
	{
		m_impl->m_susp = susp;
	}

	impl * m_impl;
};

template <typename T>
struct coro_suspend
	: coro_suspend_base, private aw::detail::task_completion<T>
{
	explicit coro_suspend(aw::task<T> && t) noexcept
		: m_t(std::move(t)), m_sink(nullptr)
	{
	}

	bool await_ready() noexcept
	{
		aw::detail::task_vtable<T> const * vtable = aw::detail::task_access::get_vtable(m_t);
		return vtable->get_result != nullptr;
	}

	template <typename U>
	void await_suspend(std::experimental::coroutine_handle<coro_task_promise<U>> h) noexcept
	{
		coro_task_promise<U> & p = h.promise();
		p.set_susp(this);
	}

	T await_resume()
	{
		aw::detail::task_vtable<T> const * vtable = aw::detail::task_access::get_vtable(m_t);
		void * storage = aw::detail::task_access::storage(m_t);

		assert(vtable->get_result != nullptr);
		return vtable->get_result(storage).get();
	}

	void start(aw::detail::scheduler & sch, coro_suspend_completion & sink) override
	{
		void * storage = aw::detail::task_access::storage(m_t);

		for (;;)
		{
			aw::detail::task_vtable<T> const * vtable = aw::detail::task_access::get_vtable(m_t);
			assert(vtable->get_result == nullptr);
			aw::task<T> t = vtable->start(storage, sch, *this);
			if (t.empty())
				break;
			m_t = std::move(t);
		}

		m_sink = &sink;
	}

	void on_completion(aw::detail::scheduler & sch, aw::task<T> && t) override
	{
		void * storage = aw::detail::task_access::storage(m_t);

		while (!t.empty())
		{
			m_t = std::move(t);

			aw::detail::task_vtable<T> const * vtable = aw::detail::task_access::get_vtable(m_t);
			if (vtable->get_result != nullptr)
			{
				m_sink->on_completion(sch);
				return;
			}

			t = vtable->start(storage, sch, *this);
		}
	}

private:
	aw::task<T> m_t;
	coro_suspend_completion * m_sink;
};

}

template <typename T>
auto operator co_await(task<T> && t)
{
	return detail::coro_suspend<T>(std::move(t));
}

}

template <typename T, typename... Args>
struct std::experimental::coroutine_traits<aw::task<T>, Args...>
{
	typedef aw::detail::coro_task_promise<T> promise_type;
};

#endif // AWAIT_COROUTINE_H
