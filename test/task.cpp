#include <await/task.h>
#include <await/run.h>
#include <mutest/test.h>

TEST("aw::task should support default construction")
{
	aw::task<int> t;
	chk t.empty();
}

TEST("aw::task should support values")
{
	aw::task<int> t = aw::value(42);
	chk !t.empty();
	chk aw::run(std::move(t)) == 42;
}

TEST("aw::task should support exceptions")
{
	aw::task<int> t = std::make_exception_ptr(42);
	chk !t.empty();

	try
	{
		aw::run(std::move(t));
	}
	catch (int e)
	{
		chk e == 42;
	}
}

TEST("aw::task<void> should support values")
{
	aw::task<void> t = aw::value();
	chk !t.empty();

	aw::run(std::move(t));
}

TEST("aw::task<void> should support exceptions")
{
	aw::task<void> t = std::make_exception_ptr(42);
	chk !t.empty();

	try
	{
		aw::run(std::move(t));
	}
	catch (int e)
	{
		chk e == 42;
	}
}
