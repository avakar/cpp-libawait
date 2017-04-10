#include <avakar/await/run.h>
#include "win32_scheduler.h"
#include "intrusive_list.h"
#include <stdexcept>
#include <memory>

struct fake_scheduler;

struct aw::detail::scheduler::token::impl final
	: intrusive_hook
{
	impl()
		: handle_(NULL), alloc_sink_(nullptr), wait_sink_(nullptr)
	{
	}

	~impl()
	{
		if (handle_)
			CloseHandle(handle_);
	}

	HANDLE handle_;
	size_t handle_idx_;

	alloc_event_sink * alloc_sink_;
	completion_sink * wait_sink_;
	fake_scheduler * scheduler_;
};

using namespace aw;
using namespace aw::detail;

scheduler::token::token(impl * ptr) noexcept
	: ptr_(ptr)
{
}

scheduler::token::~token() noexcept
{
	this->clear();
}

scheduler::token::token(token && o) noexcept
	: ptr_(o.ptr_)
{
	o.ptr_ = nullptr;
}

scheduler::token & scheduler::token::operator=(token && o) noexcept
{
	std::swap(ptr_, o.ptr_);
	return *this;
}

HANDLE scheduler::token::get() const noexcept
{
	assert(ptr_ != nullptr);
	return ptr_->handle_;
}

scheduler::token::impl * scheduler::token::get_impl() const noexcept
{
	return ptr_;
}

struct fake_scheduler
	: scheduler, task_completion<void>
{
	token alloc_event(alloc_event_sink & sink, std::error_code & ec) noexcept override
	{
		token::impl * tok = nullptr;

		if (!cached_toks_.empty())
		{
			tok = &cached_toks_.front();
		}
		else
		{
			tok = new(std::nothrow) token::impl;
			if (!tok)
			{
				ec = std::make_error_code(std::errc::not_enough_memory);
				return token();
			}
			cached_toks_.push_back(*tok);
		}

		if (tok->handle_ == NULL && total_handles_ < 63)
		{
			tok->handle_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
			if (!tok->handle_)
			{
				ec = std::error_code(GetLastError(), std::system_category());
				return token();
			}

			++total_handles_;
			ready_toks_.push_back(*tok);
		}
		else
		{
			tok->alloc_sink_ = &sink;
			empty_toks_.push_back(*tok);
		}

		ec.clear();
		return token(tok);
	}

	void add_handle(token const & tok, completion_sink & sink) noexcept override
	{
		auto tt = tok.get_impl();

		assert(tt->wait_sink_ == nullptr);
		assert(tt->handle_ != NULL);

		tt->wait_sink_ = &sink;
		tt->handle_idx_ = m_handle_count++;
		m_handles[tt->handle_idx_] = tt->handle_;
		scheduled_tokens_[tt->handle_idx_] = tt;
	}

	void remove_handle(token const & tok) noexcept override
	{
		auto tt = tok.get_impl();

		assert(tt->wait_sink_ != nullptr);

		std::swap(scheduled_tokens_[tt->handle_idx_], scheduled_tokens_[m_handle_count - 1]);
		std::swap(m_handles[tt->handle_idx_], m_handles[m_handle_count - 1]);

		--m_handle_count;
		tt->wait_sink_ = nullptr;
	}

	void clear_token(token::impl * tok) noexcept
	{
		cached_toks_.push_back(*tok);
		if (tok->handle_ != NULL && !empty_toks_.empty())
		{
			token::impl * empty_tok = &empty_toks_.front();
			empty_tok->handle_ = tok->handle_;
			tok->handle_ = NULL;

			ready_toks_.push_back(*empty_tok);
			empty_tok->alloc_sink_->on_event_ready(*this, empty_tok->handle_);
		}
	}

	struct sleeper_node_impl
		: sleeper_node, intrusive_hook
	{
		sleeper_node_impl(fake_scheduler & sch, completion_sink & sink)
			: m_sch(sch), m_sink(sink), m_complete(false)
		{
		}

		void wakeup() override
		{
			m_sch.m_sleepers.remove(*this);
			m_sch.m_woken_sleepers.push_back(*this);
			m_complete = true;
		}

		fake_scheduler & m_sch;
		completion_sink & m_sink;
		bool m_complete;
	};

	explicit fake_scheduler(task<void> & t)
		: m_task(t), m_updated(false), m_handle_count(0)
	{
	}

	sleeper_node * register_sleeper(completion_sink & sink) override
	{
		sleeper_node_impl * s = new sleeper_node_impl(*this, sink);
		m_sleepers.push_back(*s);
		return s;
	}

	void wait_for_sleeper(sleeper_node & sleeper) override
	{
		sleeper_node_impl & impl = static_cast<sleeper_node_impl &>(sleeper);
		while (!impl.m_complete)
			SleepEx(INFINITE, TRUE);
	}

	cancel_info get_cancel_info() const noexcept
	{
		return nullptr;
	}

	void on_completion(scheduler & sch, task<void> && t) override
	{
		assert(&sch == this);
		(void)sch;

		detail::mark_complete(m_task);
		m_task = std::move(t);
		m_updated = true;
	}

	task<void> & m_task;
	bool m_updated;

	HANDLE m_handles[64];
	token::impl * scheduled_tokens_[64];
	size_t m_handle_count;

	intrusive_list<token::impl> cached_toks_;
	intrusive_list<token::impl> empty_toks_;
	intrusive_list<token::impl> ready_toks_;
	size_t total_handles_ = 0;

	intrusive_list<sleeper_node_impl> m_sleepers;
	intrusive_list<sleeper_node_impl> m_woken_sleepers;
};

void scheduler::token::clear() noexcept
{
	if (ptr_)
	{
		ptr_->scheduler_->clear_token(ptr_);
		ptr_ = nullptr;
	}
}

aw::result<void> aw::detail::try_run_impl(task<void> && t)
{
	assert(!t.empty());

	fake_scheduler sch(t);
	while (detail::start_command(t, sch, sch))
	{
		sch.m_updated = false;
		while (!sch.m_updated)
		{
			if (sch.m_handle_count == 0)
			{
				SleepEx(INFINITE, TRUE);
			}
			else
			{
				DWORD r = WaitForMultipleObjectsEx(sch.m_handle_count, sch.m_handles, FALSE, INFINITE, TRUE);
				if (r >= WAIT_OBJECT_0 && r - WAIT_OBJECT_0 < sch.m_handle_count)
				{
					size_t i = r - WAIT_OBJECT_0;
					HANDLE handle = sch.m_handles[i];
					auto sink = sch.scheduled_tokens_[i];
					auto wait_sink = sink->wait_sink_;


					std::swap(sch.m_handles[i], sch.m_handles[sch.m_handle_count - 1]);
					std::swap(sch.scheduled_tokens_[i], sch.scheduled_tokens_[sch.m_handle_count - 1]);
					--sch.m_handle_count;
					sink->wait_sink_ = nullptr;

					auto cr = wait_sink->on_completion(sch);
					if (cr == scheduler::completion_sink::completion_result::reschedule)
					{
						sch.m_handles[sch.m_handle_count] = handle;
						sch.scheduled_tokens_[sch.m_handle_count] = sink;
						++sch.m_handle_count;
						sink->wait_sink_ = wait_sink;
					}
				}
			}

			for (auto it = sch.m_woken_sleepers.begin(); it != sch.m_woken_sleepers.end(); )
			{
				auto & sleeper = *it;

				auto cr = sleeper.m_sink.on_completion(sch);
				it = sch.m_woken_sleepers.remove(sleeper);

				if (cr == scheduler::completion_sink::completion_result::reschedule)
					sch.m_sleepers.push_back(sleeper);
				else
					delete &sleeper;
			}
		}
	}

	return t.dismiss();
}
