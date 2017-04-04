#include <avakar/await/task.h>
#include <avakar/await/run.h>
#include <mutest/test.h>

TEST("continue_with should be called on success")
{
	aw::task<int> t = aw::value(42);
	aw::task<int> u = t.continue_with([](aw::result<int> r) {
		chk r.has_value();
		return r.value() + 2;
	});

	chk aw::run(std::move(u)) == 44;
}

TEST("continue_with should be called on failure")
{
	aw::task<int> t = std::make_exception_ptr(1);
	aw::task<int> u = t.continue_with([](aw::result<int> r) {
		chk r.has_exception();
		return 43;
	});

	chk aw::run(std::move(u)) == 43;
}

TEST("continue_with should support returning tasks")
{
	aw::task<int> t = aw::value(42);
	aw::task<int> u = t.continue_with([](aw::result<int> r) {
		chk r.has_value();
		return aw::postpone().then([r] { return r.value() + 2; });
	});

	chk aw::run(std::move(u)) == 44;
}

TEST("continue_with should support returning void")
{
	int called = 0;

	aw::task<int> t = aw::value(42);
	aw::task<void> u = t.continue_with([&called](aw::result<int> r) {
		++called;
	});

	aw::run(std::move(u));
	chk called == 1;
}

TEST("continue_with should support returning void task")
{
	aw::task<int> t = aw::value(42);
	aw::task<void> u = t.continue_with([](aw::result<int> r) {
		return aw::value();
	});

	aw::run(std::move(u));
}

TEST("continue_with should work postponed")
{
	aw::task<int> u = aw::postpone().continue_with([](aw::result<void> r) {
		chk r.has_value();
		return 45;
	});

	chk aw::run(std::move(u)) == 45;
}

TEST("continue_with should be called even during destruction")
{
	int called = 0;

	{
		aw::task<void> t = aw::postpone().continue_with([&called](aw::result<void> r) {
			chk r.has_exception();
			++called;
		});

		(void)t;
		chk called == 0;
	}

	chk called == 1;
}

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
