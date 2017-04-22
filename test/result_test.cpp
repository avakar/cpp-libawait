#include <avakar/await/result.h>
#include <mutest/test.h>

namespace aw = avakar::libawait;

namespace {

struct mockobject
{
	mockobject(int * counter, int a = 1, int b = 2)
		: counter_(counter), value(a + b)
	{
		++*counter_;
	}

	mockobject(int a = 1, int b = 2)
		: counter_(nullptr), value(a + b)
	{
	}

	mockobject(mockobject && o)
		: counter_(o.counter_), value(o.value)
	{
		o.counter_ = nullptr;
		o.value = -1;
	}

	mockobject(mockobject const & o)
		: counter_(o.counter_), value(o.value)
	{
		if (counter_)
			++*counter_;
	}

	~mockobject()
	{
		if (counter_)
			--*counter_;
	}

	mockobject & operator=(mockobject && o)
	{
		counter_ = o.counter_;
		value = o.value;
		o.value = -1;

		if (counter_)
			++*counter_;
		return *this;
	}

	mockobject & operator=(mockobject const & o)
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

}

TEST("aw::result<T>() shall hold T()")
{
	aw::result<mockobject> r;
	chk r;
	chk r.get().value == 3;
}

TEST("aw::result::holds_value is true while holding a value")
{
	aw::result<mockobject> r;
	chk r.holds_value();
}

TEST("aw::result::holds_value is false while holding an error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk !r.holds_value();
}

TEST("aw::result::holds_value is false while holding an exception")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !r.holds_value();
}

TEST("aw::result::holds_error_code is false while holding a value")
{
	aw::result<mockobject> r;
	chk !r.holds_error_code();
}

TEST("aw::result::holds_error_code is true while holding an error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk r.holds_error_code();
}

TEST("aw::result::holds_error_code is false while holding an exception")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !r.holds_error_code();
}

TEST("aw::result::holds_exception is false while holding a value")
{
	aw::result<mockobject> r;
	chk !r.holds_exception();
}

TEST("aw::result::holds_exception is false while holding an error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk !r.holds_exception();
}

TEST("aw::result::holds_exception is true while holding an exception")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk r.holds_exception();
}

TEST("aw::result<void>() shall hold a value")
{
	aw::result<void> r;
	chk r;
	r.get();
}

TEST("aw::result<T> can be constructed with a T in-place")
{
	{
		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), 3, 4);
		chk r;
		chk r.get().value == 7;
	}

	{
		mockobject m = 3;

		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), std::move(m));
		chk r;
		chk r.get().value == 5;
		chk m.value == -1;
	}

	{
		mockobject m = 3;

		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), m);
		chk r;
		chk r.get().value == 5;
		chk m.value == 5;
	}
}

TEST("aw::result<T> properly destroys the value")
{
	int counter = 0;

	{
		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), &counter);
		chk r;
		chk r.get().value == 3;
		chk counter == 1;
	}

	chk counter == 0;
}

TEST("aw::result<T> can be constructed with error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk !r;
	chk_exc(std::system_error, r.get());
}

TEST("aw::result<T> can be constructed with an exception_ptr")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !r;
	chk_exc(int, r.get());
}

TEST("aw::result<void> can be constructed with an exception_ptr")
{
	aw::result<void> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !r;
	chk_exc(int, r.get());
}

TEST("aw::result can initialize implicitly from value")
{
	aw::result<mockobject> r = 3;
	chk r.get().value == 5;
}

TEST("aw::result can initialize implicitly from error_code")
{
	aw::result<mockobject> r = std::make_error_code(std::errc::invalid_argument);
	chk r.holds_error_code();
	chk_exc(std::system_error, r.get());
}

TEST("aw::result can initialize implicitly from exception_ptr")
{
	aw::result<mockobject> r = std::make_exception_ptr(1);
	chk r.holds_exception();
	chk_exc(int, r.get());
}

TEST("aw::result copies correctly")
{
	int c = 0;

	aw::result<mockobject> r1 = &c;
	aw::result<mockobject> r2 = r1;

	chk c == 2;
}

TEST("aw::result moves correctly")
{
	int c = 0;

	aw::result<mockobject> r1 = &c;
	aw::result<mockobject> r2 = std::move(r1);

	chk c == 1;
}

TEST("aw::result converts correctly")
{
	aw::result<int> r1 = 1;
	aw::result<long> r2 = r1;

	chk r2.get() == 1;
}
