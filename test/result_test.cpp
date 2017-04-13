#include <avakar/await/result.h>
#include <mutest/test.h>

TEST("aw::result shall be default-constructible (with a singleton value)")
{
	aw::result<int> r;
	(void)r;
}

TEST("aw::result<void> shall be default-constructible (with a singleton value)")
{
	aw::result<void> r;
	(void)r;
}

TEST("aw::result constructor shall not throw")
{
	mutest::copy_error_tester cet;
	while (cet.next())
	{
		auto t = aw::value(cet.get());
		chk cet.good() == t.has_value();

		auto u = t;
		chk cet.good() == u.has_value();
		chk t.has_value() || u.has_exception();

		try
		{
			u.rethrow();
		}
		catch (mutest::copy_error const &)
		{
		}
	}
}

TEST("aw::result should support simple values")
{
	aw::result<int> t = aw::value(42);
	chk t.kind() == aw::result_kind::value;
	chk t.has_value();
	chk !t.has_exception();
	t.rethrow();
	chk t.get() == 42;
}

TEST("aw::result should support exceptions")
{
	aw::result<int> t = std::make_exception_ptr(42);
	chk !t.has_value();
	chk t.has_exception();
	chk t.exception() != nullptr;

	try
	{
		t.rethrow();
	}
	catch (int e)
	{
		chk e == 42;
	}

	try
	{
		t.get();
	}
	catch (int e)
	{
		chk e == 42;
	}
}

TEST("aw::result<void> should support simple values")
{
	aw::result<void> t = aw::value();
	chk t.kind() == aw::result_kind::value;
	chk t.has_value();
	chk !t.has_exception();
	chk t.exception() == nullptr;
	t.rethrow();
	t.get();
}

TEST("aw::result<void> should support exceptions")
{
	aw::result<void> t = std::make_exception_ptr(42);
	chk t.kind() == aw::result_kind::exception;
	chk !t.has_value();
	chk t.has_exception();
	chk t.exception() != nullptr;

	try
	{
		t.rethrow();
	}
	catch (int e)
	{
		chk e == 42;
	}

	try
	{
		t.get();
	}
	catch (int e)
	{
		chk e == 42;
	}
}
