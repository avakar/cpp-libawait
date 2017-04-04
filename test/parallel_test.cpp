#include <avakar/await/task.h>
#include <avakar/await/run.h>
#include <mutest/test.h>

TEST("parallel should work")
{
	static size_t const count = 40;

	int called[count] = {};

	aw::task<void> t = aw::value();
	for (size_t i = 0; i < count; ++i)
	{
		t |= aw::postpone().then([&called, i] {
			++called[i];
		});
	}

	chk !t.empty();
	for (size_t i = 0; i < count; ++i)
		chk called[i] == 0;

	aw::run(std::move(t));
	chk t.empty();

	for (size_t i = 0; i < count; ++i)
		chk called[i] == 1;
}
