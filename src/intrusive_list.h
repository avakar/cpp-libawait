#ifndef AWAIT_DETAIL_INTRUSIVE_LIST_H
#define AWAIT_DETAIL_INTRUSIVE_LIST_H

namespace aw {
namespace detail {

struct intrusive_hook
{
	intrusive_hook() noexcept
		: m_next(nullptr), m_prev(nullptr)
	{
	}

	void detach() noexcept
	{
		m_prev->m_next = m_next;
		m_next->m_prev = m_prev;
		m_next = nullptr;
		m_prev = nullptr;
	}

	intrusive_hook * m_next;
	intrusive_hook * m_prev;
};


template <typename T>
struct intrusive_list
{
	typedef intrusive_hook node;

	struct iterator
	{
		iterator() noexcept
			: m_node(nullptr)
		{
		}

		explicit iterator(node * n) noexcept
			: m_node(n)
		{
		}

		T & operator*() noexcept
		{
			return static_cast<T &>(*m_node);
		}

		iterator & operator++() noexcept
		{
			m_node = m_node->m_next;
			return *this;
		}

		friend bool operator!=(iterator const & lhs, iterator const & rhs) noexcept
		{
			return lhs.m_node != rhs.m_node;
		}

		node * m_node;
	};

	intrusive_list() noexcept
	{
		m_head.m_next = &m_head;
		m_head.m_prev = &m_head;
	}

	~intrusive_list()
	{
		this->clear();
	}

	void clear() noexcept
	{
		node * cur = m_head.m_next;
		while (cur != &m_head)
		{
			node * next = cur->m_next;
			delete static_cast<T *>(cur);
			cur = next;
		}

		m_head.m_next = &m_head;
		m_head.m_prev = &m_head;
	}

	bool empty() const noexcept
	{
		return m_head.m_next == &m_head;
	}

	iterator begin() noexcept
	{
		return iterator(m_head.m_next);
	}

	iterator end() noexcept
	{
		return iterator(&m_head);
	}

	T & front() noexcept
	{
		return *static_cast<T *>(m_head.m_next);
	}

	T const & front() const noexcept
	{
		return *static_cast<T *>(m_head.m_next);
	}

	T & back() noexcept
	{
		return *static_cast<T *>(m_head.m_prev);
	}

	T const & back() const noexcept
	{
		return *static_cast<T *>(m_head.m_prev);
	}

	void push_back(T & v) noexcept
	{
		this->insert(v, this->end());
	}

	void pop_front() noexcept
	{
		m_head.m_next->detach();
	}

	void pop_back() noexcept
	{
		m_head.m_prev->detach();
	}

	void insert(T & v, iterator where) noexcept
	{
		node * n = &v;

		if (n->m_next)
		{
			n->m_next->m_prev = n->m_prev;
			n->m_prev->m_next = n->m_next;
		}

		n->m_prev = where.m_node;
		n->m_next = where.m_node->m_next;
		n->m_prev->m_next = n;
		n->m_next->m_prev = n;
	}

	iterator remove(T & v) noexcept
	{
		node * n = &v;
		return this->erase(iterator(n));
	}

	iterator erase(iterator it) noexcept
	{
		node * n = it.m_node;
		node * r = it.m_node->m_next;
		n->detach();
		return iterator(r);
	}

private:
	node m_head;
};

}
}

#endif // AWAIT_DETAIL_INTRUSIVE_LIST_H
