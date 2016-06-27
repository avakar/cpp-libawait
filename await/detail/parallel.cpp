#include "../task.h"
#include <list>
#include <memory>
using namespace aw;
using namespace aw::detail;

namespace {

struct parallel_cmd
	: command<void>
{
	parallel_cmd()
		: m_sink(nullptr)
	{
	}

	std::exception_ptr add_cmd(task<void> & t)
	{
		assert(!t.empty());

		if (command_ptr<void> cmd = fetch_command(t))
		{
			try
			{
				m_items.emplace_back();
			}
			catch (...)
			{
				cancel_info ci = std::current_exception();
				result<void> r = cmd.dismiss();
				if (r.has_exception())
					ci = r.exception();
				r = this->dismiss(ci);
				if (r.has_exception())
					ci = r.exception();
				return ci;
			}

			item & it = m_items.back();
			it.cmd = std::move(cmd);
			it.m_it = std::prev(m_items.end());
			it.m_parent = this;
			return nullptr;
		}

		result<void> r = t.dismiss();
		if (r.has_exception())
		{
			cancel_info ci = r.exception();
			r = this->dismiss(ci);
			if (r.has_exception())
				ci = r.exception();
			return ci;
		}

		return nullptr;
	}

	result<void> dismiss(cancel_info ci) noexcept override
	{
		while (!m_items.empty())
		{
			item & it = m_items.front();
			result<void> r = it.cmd.dismiss(ci);
			if (r.has_exception())
				ci = r.exception();
			m_items.pop_front();
		}

		return ci;
	}

	task<void> start(scheduler & sch, task_completion<void> & sink) noexcept override
	{
		m_sink = &sink;
		for (auto it = m_items.begin(); it != m_items.end(); )
		{
			task<void> t = it->cmd.start(sch, *it);
			if (t.empty())
			{
				++it;
				continue;
			}

			result<void> r = t.dismiss();
			if (r.has_value())
			{
				it = m_items.erase(it);
				continue;
			}

			cancel_info ci = r.exception();
			for (auto it2 = m_items.begin(); it2 != it; ++it2)
			{
				r = it2->cmd.cancel(sch, ci);
				if (r.has_exception())
					ci = r.exception();
			}

			m_items.erase(m_items.begin(), it);
			return this->dismiss(ci);
		}

		if (m_items.empty())
			return value();

		return nullptr;
	}

	result<void> cancel(scheduler & sch, cancel_info ci) noexcept override
	{
		while (!m_items.empty())
		{
			item & it = m_items.front();
			result<void> r = it.cmd.cancel(sch, ci);
			if (r.has_exception())
				ci = r.exception();
			m_items.pop_front();
		}

		return ci;
	}

private:
	struct item
		: task_completion<void>
	{
		void on_completion(scheduler & sch, task<void> && t) override
		{
			cmd.complete(t);
			if (!cmd)
				m_parent->complete_item(sch, *this, t.dismiss());
		}

		command_ptr<void> cmd;
		parallel_cmd * m_parent;
		std::list<item>::iterator m_it;
	};

	void complete_item(scheduler & sch, item & it, result<void> && r)
	{
		m_items.erase(it.m_it);
		if (r.has_value())
		{
			if (m_items.empty())
				m_sink->on_completion(sch, value());
			return;
		}

		m_sink->on_completion(sch, this->cancel(sch, r.exception()));
	}

	std::list<item> m_items;
	task_completion<void> * m_sink;
};

}

task<void> aw::operator|(task<void> && lhs, task<void> && rhs)
{
	if (command_ptr<void> lhs_cmd = fetch_command(lhs))
	{
		if (auto p = dynamic_cast<parallel_cmd *>(lhs_cmd.get()))
		{
			lhs_cmd.release();

			std::exception_ptr e = p->add_cmd(rhs);
			if (e)
			{
				delete p;
				return e;
			}

			return from_command(p);
		}

		lhs = from_command(lhs_cmd);
	}

	std::unique_ptr<parallel_cmd> cmd;

	try
	{
		cmd.reset(new parallel_cmd());
	}
	catch (...)
	{
		cancel_info ci = std::current_exception();
		result<void> r = lhs.dismiss(ci);
		if (r.has_exception())
			ci = r.exception();
		r = rhs.dismiss(ci);
		if (r.has_exception())
			ci = r.exception();
		return ci;
	}

	std::exception_ptr e = cmd->add_cmd(lhs);
	if (e)
	{
		result<void> r = rhs.dismiss(e);
		if (r.has_exception())
			e = r.exception();
		return e;
	}

	e = cmd->add_cmd(rhs);
	if (e)
		return e;

	return from_command(cmd.release());
}
