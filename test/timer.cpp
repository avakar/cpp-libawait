#include <await/timer.h>
#include <await/run.h>
#include <mutest/test.h>

TEST("aw::wait_ms should work")
{
	aw::task<void> t = aw::wait_ms(10000);
	aw::run(std::move(t));
}
