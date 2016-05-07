#include <await/task.h>
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
