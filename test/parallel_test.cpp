#include <await/task.h>
#include <await/run.h>
#include <mutest/test.h>

TEST("parallel should work")
{
	int lhs_called = 0;
	int rhs_called = 0;
	int rrhs_called = 0;

	aw::task<void> t = aw::postpone().then([&lhs_called] {
		++lhs_called;
	}) | aw::postpone().then([&rhs_called] {
		++rhs_called;
	}) | aw::postpone().then([&rrhs_called] {
		++rrhs_called;
	});

	chk !t.empty();
	chk lhs_called == 0;
	chk rhs_called == 0;
	chk rrhs_called == 0;
	aw::run(std::move(t));

	chk t.empty();
	chk lhs_called == 1;
	chk rhs_called == 1;
	chk rrhs_called == 1;
}
