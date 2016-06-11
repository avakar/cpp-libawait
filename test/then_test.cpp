#include <await/task.h>
#include <await/run.h>
#include <mutest/test.h>

TEST("aw::task::then should take callbacks returning value")
{
	aw::task<int> t = aw::value(42);
	aw::task<int> v = t.then([](int v) -> int {
		return v + 2;
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 44;
}

TEST("aw::task::then should take callbacks returning result")
{
	aw::task<int> t = aw::value(42);
	aw::task<int> v = t.then([](int v) -> aw::result<int> {
		return aw::value(v + 2);
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 44;
}

TEST("aw::task::then should take callbacks returning task")
{
	aw::task<int> t = aw::value(42);
	aw::task<int> v = t.then([](int v) -> aw::task<int> {
		return aw::value(v + 2);
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 44;
}

TEST("aw::task<void>::then should take callbacks returning value")
{
	aw::task<void> t = aw::value();
	aw::task<int> v = t.then([]() -> int {
		return 44;
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 44;
}

TEST("aw::task<void>::then should take callbacks returning result")
{
	aw::task<void> t = aw::value();
	aw::task<int> v = t.then([]() -> aw::result<int> {
		return aw::value(44);
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 44;
}

TEST("aw::task<void>::then should take callbacks returning task")
{
	aw::task<void> t = aw::value();
	aw::task<int> v = t.then([]() -> aw::task<int> {
		return aw::value(44);
	});

	chk t.empty();
	chk !v.empty();
	chk aw::run(std::move(v)) == 44;
}
