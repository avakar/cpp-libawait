#ifndef AWAIT_DETAIL_INTRUSIVE_LIST_H
#define AWAIT_DETAIL_INTRUSIVE_LIST_H

namespace aw {
namespace detail {

struct intrusive_hook
{
	intrusive_hook()
		: m_next(nullptr), m_prev(nullptr)
	{
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
		iterator()
			: m_node(nullptr)
		{
		}

		explicit iterator(node * n)
			: m_node(n)
		{
		}

		T & operator*()
		{
			return static_cast<T &>(*m_node);
		}

		iterator & operator++()
		{
			m_node = m_node->m_next;
			return *this;
		}

		friend bool operator!=(iterator const & lhs, iterator const & rhs)
		{
			return lhs.m_node != rhs.m_node;
		}

		node * m_node;
	};

	intrusive_list()
	{
		m_head.m_next = &m_head;
		m_head.m_prev = &m_head;
	}

	iterator begin()
	{
		return iterator(m_head.m_next);
	}

	iterator end()
	{
		return iterator(&m_head);
	}

	void push_back(T & v)
	{
		this->insert(v, this->end());
	}

	void insert(T & v, iterator where)
	{
		node * n = &v;
		n->m_prev = where.m_node;
		n->m_next = where.m_node->m_next;
		n->m_prev->m_next = n;
		n->m_next->m_prev = n;
	}

	iterator remove(T & v)
	{
		node * n = &v;
		return this->erase(iterator(n));
	}

	iterator erase(iterator it)
	{
		node * n = it.m_node;
		node * r = it.m_node->m_next;

		n->m_prev->m_next = n->m_next;
		n->m_next->m_prev = n->m_prev;
		n->m_next = nullptr;
		n->m_prev = nullptr;
		return iterator(r);
	}

	void clear()
	{
		node * p = m_head.m_next;
		while (p != &m_head)
		{
			node * q = p->m_next;
			p->m_next = nullptr;
			p->m_prev = nullptr;
			p = q;
		}

		m_head.m_next = &m_head;
		m_head.m_prev = &m_head;
	}

private:
	node m_head;
};

}
}

#endif // AWAIT_DETAIL_INTRUSIVE_LIST_H
