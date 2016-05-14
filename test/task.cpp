#include <await/task.h>
#include <await/run.h>
#include <mutest/test.h>

namespace {

struct movable
{
	movable(int v): v(v) {}
	movable(movable && o) { std::swap(v, o.v); }
	int v;
};

struct counting
{
	~counting() { --*c; }
	explicit counting(int & c): c(&c) { ++c; }
	counting(counting const & o): c(o.c) { ++*c; }
	counting & operator=(counting const & o) { c = o.c; ++*c; }
	int * c;
};

}

TEST("aw::task should support default construction")
{
	aw::task<int> t;
	chk t.empty();
}

TEST("aw::task should support construction from nullptr")
{
	aw::task<int> t = nullptr;
	chk t.empty();
}

TEST("empty aw::task should support move construction")
{
	aw::task<int> t;
	chk t.empty();

	aw::task<int> t2 = std::move(t);
	chk t2.empty();
	chk t.empty();
}

TEST("empty aw::task should support move assignment")
{
	aw::task<int> t, t2;
	chk t.empty();
	chk t2.empty();

	t2 = std::move(t);
	chk t.empty();
	chk t2.empty();
}

TEST("aw::task should support values")
{
	aw::task<int> t = aw::value(42);
	chk !t.empty();
	chk aw::run(std::move(t)) == 42;
}

TEST("aw::task with a value should support move construction")
{
	aw::task<int> t = aw::value(42);
	chk !t.empty();

	aw::task<int> t2 = std::move(t);
	chk t.empty();
	chk !t2.empty();

	chk aw::run(std::move(t2)) == 42;
}

TEST("aw::task with a value should support move assignment")
{
	aw::task<int> t = aw::value(42);
	chk !t.empty();

	aw::task<int> t2;
	chk t2.empty();

	t2 = std::move(t);
	chk t.empty();
	chk !t2.empty();

	chk aw::run(std::move(t2)) == 42;
}

TEST("aw::task should support move-only values")
{
	aw::task<movable> t = aw::value(42);
	chk !t.empty();

	aw::task<movable> t2 = std::move(t);
	chk t.empty();
	chk !t2.empty();

	aw::task<movable> t3;
	chk t3.empty();

	t3 = std::move(t2);
	chk !t3.empty();
	chk t.empty();
	chk t2.empty();

	chk aw::run(std::move(t3)).v == 42;
}

TEST("aw::task should manage objects correctly")
{
	int counter = 0;
	aw::task<counting> t = aw::value(counting(counter));
	chk counter == 1;
	chk !t.empty();

	{
		aw::task<counting> t2 = aw::value(counting(counter));
		chk !t2.empty();
		chk counter == 2;
	}

	chk counter == 1;
	t.clear();

	chk t.empty();
	chk counter == 0;
}

TEST("aw::task should support exceptions")
{
	int counter = 0;

	{
		aw::task<int> t = std::make_exception_ptr(counting(counter));
		chk !t.empty();

		aw::task<int> t2 = std::move(t);
		chk t.empty();
		chk !t2.empty();

		try
		{
			aw::run(std::move(t2));
		}
		catch (counting e)
		{
			chk e.c == &counter;
		}
	}

	chk counter == 0;
}

TEST("aw::task<void> should support construction from nullptr")
{
	aw::task<void> t = nullptr;
	chk t.empty();
}

TEST("aw::task<void> should support values")
{
	aw::task<void> t = aw::value();
	chk !t.empty();

	aw::task<void> t2 = std::move(t);
	chk t.empty();
	chk !t2.empty();

	aw::task<void> t3;
	chk t3.empty();

	t3 = std::move(t2);
	chk !t3.empty();
	chk t2.empty();

	aw::run(std::move(t3));
}

TEST("aw::task<void> should support clear()")
{
	aw::task<void> t = aw::value();
	chk !t.empty();

	t.clear();
	chk t.empty();
}

TEST("aw::task<void> should support exceptions")
{
	int counter = 0;

	{
		aw::task<void> t = std::make_exception_ptr(counting(counter));
		chk !t.empty();

		aw::task<void> t2 = std::move(t);
		chk t.empty();
		chk !t2.empty();

		try
		{
			aw::run(std::move(t2));
		}
		catch (counting e)
		{
			chk e.c == &counter;
		}
	}

	chk counter == 0;
}
