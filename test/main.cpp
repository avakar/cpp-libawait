#include <await/result.h>
#include <mutest/test.h>

TEST("aw::result should support simple values")
{
	aw::result<int> t = aw::value(42);
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
	chk t.has_value();
	chk !t.has_exception();
	t.rethrow();
	t.get();
}

TEST("aw::result<void> should support exceptions")
{
	aw::result<void> t = std::make_exception_ptr(42);
	chk !t.has_value();
	chk t.has_exception();

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
