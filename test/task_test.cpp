#include <avakar/await/task.h>
#include <mutest/test.h>

namespace aw = avakar::libawait;

namespace {

struct mock_command
	: aw::command<int>
{
	mock_command(int val)
		: value_(val), dismiss_count_(0)
	{
	}

	aw::task<int> start(aw::scheduler & sch, aw::task_completion<int> & sink) noexcept override
	{
		++dismiss_count_;
		return value_;
	}

	aw::result<int> cancel(aw::scheduler * sch) noexcept override
	{
		++dismiss_count_;
		return value_;
	}

	int value_;
	int dismiss_count_;
};

}

TEST("aw::task default-constructs into a value")
{
	aw::task<int> t;
	aw::result<int> r = t.dismiss();
	chk r == 0;
}

TEST("aw::task implicitly constructs into nulltask")
{
	aw::task<int> t = aw::nulltask;
	chk !t;
}

TEST("aw::task implicitly constructs into a value")
{
	aw::task<int> t = 1;
	aw::result<int> r = t.dismiss();
	chk r == 1;
}

TEST("aw::task implicitly constructs into an error code")
{
	aw::task<int> t = std::make_error_code(std::errc::invalid_argument);
	aw::result<int> r = t.dismiss();
	chk aw::holds_alternative<std::error_code>(r);
	chk aw::get<std::error_code>(r) == std::errc::invalid_argument;
}

TEST("aw::task implicitly constructs into an exception")
{
	aw::task<int> t = std::make_exception_ptr(1);
	aw::result<int> r = t.dismiss();
	chk aw::holds_alternative<std::exception_ptr>(r);
	chk_exc(int, r.value());
}

TEST("aw::task explicitly constructs into a nulltask")
{
	aw::task<int> t{ aw::in_place_type_t<aw::nulltask_t>() };
	chk !t;
}

TEST("aw::task explicitly constructs into a command")
{
	mock_command cmd(1);

	aw::task<int> t{ aw::in_place_type_t<aw::command<int> *>(), &cmd };
	chk t;

	aw::result<int> r = t.dismiss();
	chk !t;
	chk cmd.dismiss_count_ == 1;
	chk r == 1;
}

TEST("aw::task explicitly constructs into a value")
{
	aw::task<int> t(aw::in_place_type_t<int>(), 2);
	chk t;

	aw::result<int> r = t.dismiss();
	chk !t;
	chk r == 2;
}

TEST("aw::task explicitly constructs into an error code")
{
	aw::task<int> t(aw::in_place_type_t<std::error_code>(), std::make_error_code(std::errc::invalid_argument));
	chk t;

	aw::result<int> r = t.dismiss();
	chk !t;
	chk aw::holds_alternative<std::error_code>(r);
	chk aw::get<std::error_code>(r) == std::errc::invalid_argument;
}

TEST("aw::task explicitly constructs into an exception")
{
	aw::task<int> t(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk t;

	aw::result<int> r = t.dismiss();
	chk !t;
	chk aw::holds_alternative<std::exception_ptr>(r);
	chk_exc(int, r.value());
}

TEST("aw::task dismisses content during destruction")
{
	mock_command cmd(1);
	
	{
		aw::task<int> t(aw::in_place_type_t<aw::command<int> *>(), &cmd);
		chk t;
	}

	chk cmd.dismiss_count_ == 1;
}
