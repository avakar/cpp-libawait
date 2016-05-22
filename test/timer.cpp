#include <await/timer.h>
#include <await/run.h>
#include <mutest/test.h>

TEST("aw::wait_ms should work")
{
	aw::task<void> t = aw::wait_ms(1);
	aw::run(std::move(t));
}

TEST("aw::wait_ms should work with composition")
{
	aw::task<void> t = aw::wait_ms(1).then([] {
		return aw::wait_ms(1);
	});
	aw::run(std::move(t));
}
