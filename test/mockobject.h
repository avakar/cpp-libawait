#ifndef AVAKAR_AWAIT_TEST_MOCKOBJECT_H
#define AVAKAR_AWAIT_TEST_MOCKOBJECT_H

template <typename T>
struct basic_mockobject
{
	basic_mockobject(int * counter, int a = 1, int b = 2)
		: counter_(counter), value(a + b)
	{
		++*counter_;
	}

	basic_mockobject(int a = 1, int b = 2)
		: counter_(nullptr), value(a + b)
	{
	}

	basic_mockobject(basic_mockobject && o)
		: counter_(o.counter_), value(o.value)
	{
		o.counter_ = nullptr;
		o.value = -1;
	}

	basic_mockobject(basic_mockobject const & o)
		: counter_(o.counter_), value(o.value)
	{
		if (counter_)
			++*counter_;
	}

	template <typename U,
		typename = std::enable_if_t<std::is_convertible<U, T>::value>>
		basic_mockobject(basic_mockobject<U> && o)
		: counter_(o.counter_), value(o.value)
	{
		o.counter_ = nullptr;
		o.value = -1;
	}

	template <typename U,
		typename = std::enable_if_t<std::is_convertible<U &, T &>::value>>
		basic_mockobject(basic_mockobject<U> const & o)
		: counter_(o.counter_), value(o.value)
	{
		if (counter_)
			++*counter_;
	}

	~basic_mockobject()
	{
		if (counter_)
			--*counter_;
	}

	basic_mockobject & operator=(basic_mockobject && o)
	{
		counter_ = o.counter_;
		value = o.value;
		o.value = -1;

		if (counter_)
			++*counter_;
		return *this;
	}

	basic_mockobject & operator=(basic_mockobject const & o)
	{
		counter_ = o.counter_;
		value = o.value;

		if (counter_)
			++*counter_;
		return *this;
	}

	int * counter_;
	int value;
};

using mockobject = basic_mockobject<int>;

#endif // AVAKAR_AWAIT_TEST_MOCKOBJECT_H
