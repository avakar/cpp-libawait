#include <avakar/await/task.h>
#include <mutest/test.h>

#include "mockobject.h"

namespace aw = avakar::libawait;

namespace {

struct mock_command
	: aw::detail::command<int>
{
	mock_command(int val)
		: value_(val), dismiss_count_(0)
	{
	}

	aw::task<int> start(aw::detail::scheduler & sch, aw::detail::task_completion<int> & sink) noexcept override
	{
		std::abort();
	}

	aw::result<int> cancel(aw::detail::scheduler * sch) noexcept override
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
	chk !t.empty();

	aw::result<int> r = t.dismiss();
	chk r == 0;
}

TEST("aw::task<void> default-constructs into a value")
{
	aw::task<void> t;
	chk !t.empty();

	aw::result<void> r = t.dismiss();
	chk r;
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

	aw::task<int> t{ aw::in_place_type_t<aw::detail::command<int> *>(), &cmd };
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

TEST("aw::task<void> explicitly constructs into a value")
{
	aw::task<void> t{ aw::in_place_type_t<void>() };
	chk t;

	aw::result<void> r = t.dismiss();
	chk !t;
	chk r;
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
		aw::task<int> t(aw::in_place_type_t<aw::detail::command<int> *>(), &cmd);
		chk t;
	}

	chk cmd.dismiss_count_ == 1;
}

TEST("aw::task destroys value during destruction")
{
	int counter = 0;

	{
		aw::task<mockobject> t = &counter;
		chk t;
		chk counter == 1;
	}

	chk counter == 0;
}

TEST("aw::task moves a value")
{
	int counter = 0;

	aw::task<mockobject> t = &counter;
	chk t;
	chk counter == 1;

	aw::task<mockobject> u = std::move(t);
	chk !t;
	chk u;
	chk counter == 1;

	chk u.dismiss()->value == 3;
	chk counter == 0;
}

TEST("aw::task moves an error_code")
{
	aw::task<int> t = std::make_error_code(std::errc::invalid_argument);
	chk t;

	aw::task<int> u = std::move(t);
	chk !t;
	chk u;

	chk u.dismiss() == std::make_error_code(std::errc::invalid_argument);
}

TEST("aw::task moves an exception")
{
	aw::task<int> t = std::make_exception_ptr(1);
	chk t;

	aw::task<int> u = std::move(t);
	chk !t;
	chk u;

	chk_exc(int, u.dismiss().value());
}

TEST("aw::task moves a command")
{
	mock_command cmd(1);

	aw::task<int> t{ aw::in_place_type_t<aw::detail::command<int> *>(), &cmd };
	chk t;

	aw::task<int> u = std::move(t);
	chk !t;
	chk u;
}

TEST("aw::task move assignes a value")
{
	aw::task<int> t(1);
	chk t;

	aw::task<int> u;
	u = std::move(t);
	chk !t;
	chk u;

	chk u.dismiss() == 1;
}

TEST("aw::task move assignes an error_code")
{
	aw::task<int> t = std::make_error_code(std::errc::invalid_argument);
	chk t;

	aw::task<int> u;
	u = std::move(t);
	chk !t;
	chk u;

	chk u.dismiss() == std::make_error_code(std::errc::invalid_argument);
}

TEST("aw::task move assignes an exception")
{
	aw::task<int> t = std::make_exception_ptr(1);
	chk t;

	aw::task<int> u;
	u = std::move(t);
	chk !t;
	chk u;

	chk_exc(int, u.dismiss().value());
}

TEST("aw::task move assignes a command")
{
	mock_command cmd(1);

	aw::task<int> t{ aw::in_place_type_t<aw::detail::command<int> *>(), &cmd };
	chk t;

	aw::task<int> u;
	u = std::move(t);
	chk !t;
	chk u;
}
