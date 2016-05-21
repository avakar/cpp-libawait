#include "../timer.h"
#include "task_vtable.h"
#include "win32_scheduler.h"

namespace aw {

template <typename I>
struct command
{
	typedef typename I::value_type value_type;

	static task<value_type> start(void * self, detail::scheduler & sch, detail::task_completion<value_type> & sink)
	{
		return static_cast<I *>(self)->start(sch, sink);
	}

	static result<value_type> get_result(void * self)
	{
		I * ss = static_cast<I *>(self);
		result<value_type> r = ss->get_result();
		ss->~I();
		return std::move(r);
	}

	static void move_to(void * self, void * dst)
	{
		I * ss = static_cast<I *>(self);
		new(dst) I(std::move(*ss));
		ss->~I();
	}

	static void destroy(void * self)
	{
		static_cast<I *>(self)->~I();
	}

};

template <typename I, typename... P>
task<typename I::value_type> make_command(P &&... p)
{
	typedef typename I::value_type T;

	task<T> t;

	void * storage = detail::task_access::storage(t);
	size_t size = detail::task_access::storage_size<T>();

	if (sizeof(I) <= size)
	{
		struct impl
		{
			static task<T> start(void * self, detail::scheduler & sch, detail::task_completion<T> & sink)
			{
				return static_cast<I *>(self)->start(sch, sink);
			}

			static void move_to(void * self, void * dst)
			{
				I * ss = static_cast<I *>(self);
				new(dst) I(std::move(*ss));
				ss->~I();
			}

			static void destroy(void * self)
			{
				static_cast<I *>(self)->~I();
			}
		};

		static detail::task_vtable<T> const vtable = {
			&impl::start,
			nullptr,
			&impl::move_to,
			&impl::destroy,
		};

		new(storage) I(std::forward<P>(p)...);
		detail::task_access::set_vtable(t, &vtable);
	}
	else
	{
		struct impl
		{
			static task<T> start(void * self, detail::scheduler & sch, detail::task_completion<T> & sink)
			{
				return (*static_cast<I **>(self))->start(sch, sink);
			}

			static void move_to(void * self, void * dst)
			{
				I ** ss = static_cast<I **>(self);
				new(dst) I *(*ss);
			}

			static void destroy(void * self)
			{
				I ** ss = static_cast<I **>(self);
				delete ss;
			}
		};

		static detail::task_vtable<T> const vtable = {
			&impl::start,
			nullptr,
			&impl::move_to,
			&impl::destroy,
		};

		I * ss = new(std::nothrow) I(std::forward<P>(p)...);
		if (ss == nullptr)
		{
			t = std::make_exception_ptr(std::bad_alloc());
		}
		else
		{
			new(storage) I *(ss);
			detail::task_access::set_vtable(t, &vtable);
		}
	}

	return std::move(t);
}

}

aw::task<void> aw::wait_ms(int64_t ms)
{
	struct impl
		: handle_completion_sink
	{
		typedef void value_type;

		explicit impl(int64_t ms)
			: m_sink(nullptr)
		{
			m_h = CreateWaitableTimerW(nullptr, TRUE, nullptr);

			LARGE_INTEGER li;
			li.QuadPart = -(ms * 10000);
			SetWaitableTimer(m_h, &li, 0, 0, 0, FALSE);
		}

		~impl()
		{
			if (m_h)
				CloseHandle(m_h);
		}

		impl(impl && o)
			: m_h(o.m_h)
		{
			o.m_h = 0;
		}

		impl & operator=(impl && o)
		{
			m_h = o.m_h;
			o.m_h = 0;
			return *this;
		}

		task<void> start(detail::scheduler & sch, detail::task_completion<void> & sink)
		{
			sch.add_handle(m_h, *this);
			m_sink = &sink;
			return nullptr;
		}

		void on_completion() override
		{
			m_sink->on_completion(aw::value());
		}

		HANDLE m_h;
		detail::task_completion<void> * m_sink;
	};

	return make_command<impl>(ms);
}
