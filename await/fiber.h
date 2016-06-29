#ifndef AWAIT_FIBER_H
#define AWAIT_FIBER_H

#include "task.h"
#include <functional>

#include <memory>

namespace aw {

struct awwait_completion
{
	virtual void on_completion(detail::scheduler & sch) noexcept = 0;
};

struct awwait_base
{
	virtual void dismiss(cancel_info ci) noexcept = 0;
	virtual bool start(detail::scheduler & sch, awwait_completion & sink) noexcept = 0;
	virtual void cancel(detail::scheduler & sch, cancel_info ci) noexcept = 0;
};

template <typename T>
struct awwait
	: awwait_base, private detail::task_completion<T>
{
	void dismiss(cancel_info ci) noexcept override
	{
		m_result = m_cmd.dismiss(ci);
	}

	bool start(detail::scheduler & sch, awwait_completion & sink) noexcept override
	{
		m_sink = &sink;

		task<T> t = m_cmd.start(sch, *this);
		if (t.empty())
			return false;

		m_result = t.dismiss();
		return true;
	}

	void on_completion(detail::scheduler & sch, task<T> && t) noexcept override
	{
		m_cmd.complete();
		m_cmd = detail::fetch_command(t);
		if (!m_cmd.empty())
			t = m_cmd.start(sch, *this);

		if (!t.empty())
		{
			m_result = t.dismiss();
			m_sink->on_completion(sch);
		}
	}

	void cancel(detail::scheduler & sch, cancel_info ci) noexcept override
	{
		m_result = m_cmd.cancel(sch, ci);
	}

	detail::command_ptr<T> m_cmd;
	result<T> m_result;
	awwait_completion * m_sink;
};

void try_await_impl(awwait_base & a);

template <typename T>
result<T> try_await(task<T> && t)
{
	awwait<T> a;
	a.m_cmd = detail::fetch_command(t);
	if (a.m_cmd.empty())
		return t.dismiss();

	try_await_impl(a);
	return std::move(a.m_result);
}

struct awwaiter
{
	template <typename T>
	T operator%(task<T> && t) { return try_await(std::move(t)).get(); }

	template <typename T>
	T operator%(task<T> & t) { return try_await(std::move(t)).get(); }
};

#define awwait ::aw::awwaiter() %

task<void> fiber_impl(std::function<void()> fn);

template <typename F>
auto fiber(F && f)
	-> typename detail::task_traits<decltype(f())>::task_type
{
	typedef typename detail::task_traits<decltype(f())>::value_type T;

	struct ctx
	{
		ctx(F && f)
			: f(std::forward<F>(f))
		{
		}

		F f;
		result<T> r;
	};

	std::shared_ptr<ctx> p = std::make_shared<ctx>(std::forward<F>(f));
	return fiber_impl([p]() {
		p->r = try_await(detail::invoke(p->f));
	}).then([p]() {
		return std::move(p->r);
	});
}

}

#endif // AWAIT_FIBER_H
