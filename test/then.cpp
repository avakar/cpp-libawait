#include <await/task.h>
#include <await/run.h>
#include <mutest/test.h>

TEST("aw::task should support then")
{
	aw::task<int> t = aw::value(42);
	aw::task<int> v = t.then([](int v) {
		return aw::value(v + 2);
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 44;
}

TEST("aw::task<void> should support then")
{
	aw::task<void> t = aw::value();
	aw::task<int> v = t.then([]() {
		return aw::value(2);
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 2;
}
