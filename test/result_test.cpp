#include <avakar/await/result.h>
#include <mutest/test.h>
#include "mockobject.h"

namespace aw = avakar::libawait;

TEST("aw::result<T>() shall hold a value")
{
	aw::result<mockobject> r;
	chk r;
	chk r.has_value();
}

TEST("a value in aw::result can be accessed via *")
{
	aw::result<mockobject> r;
	chk (*r).value == 3;
}

TEST("a value in aw::result can be accessed via ->")
{
	aw::result<mockobject> r;
	chk r->value == 3;
}

TEST("a value in aw::result can modified via ->")
{
	aw::result<mockobject> r;
	r->value = 4;
	chk r->value == 4;
}

TEST("a value in aw::result can modified via *")
{
	aw::result<mockobject> r;
	(*r).value = 4;
	chk r->value == 4;
}

TEST("a value in const aw::result can accessed via *")
{
	aw::result<mockobject> const r;
	chk (*r).value == 3;
}

TEST("a value in const aw::result can accessed via ->")
{
	aw::result<mockobject> const r;
	chk r->value == 3;
}

TEST("operator * for aw::result<void> is valid")
{
	aw::result<void> r;
	*r;
}

TEST("operator * for const aw::result<void> is valid")
{
	aw::result<void> const r;
	*r;
}

TEST("operator * for moving aw::result returns rvalue ref")
{
	int counter = 0;

	aw::result<mockobject> r1 = &counter;
	aw::result<mockobject> r2 = *std::move(r1);

	chk counter == 1;
}

TEST("operator * for rval const aw::result returns const rvalue ref")
{
	aw::result<mockobject> const r1;
	chk std::is_same<mockobject const &&, decltype(*std::move(r1))>::value;
}

TEST("a value in aw::result can be accessed via .value()")
{
	aw::result<mockobject> r;
	chk r.value().value == 3;
}

TEST("a value in aw::result can modified via .value()")
{
	aw::result<mockobject> r;
	r.value().value = 4;
	chk r->value == 4;
}

TEST("a value in const aw::result can accessed via .value()")
{
	aw::result<mockobject> const r;
	chk r.value().value == 3;
}

TEST(".value() for aw::result<void> is valid")
{
	aw::result<void> r;
	r.value();
}

TEST(".value() for const aw::result<void> is valid")
{
	aw::result<void> const r;
	r.value();
}

TEST(".value() for moving aw::result returns rvalue ref")
{
	int counter = 0;

	aw::result<mockobject> r1 = &counter;
	aw::result<mockobject> r2 = std::move(r1).value();

	chk counter == 1;
}

TEST(".value() for rval const aw::result returns const rvalue ref")
{
	aw::result<mockobject> const r1;
	chk std::is_same<mockobject const &&, decltype(std::move(r1).value())>::value;
}

TEST(".value() for rval aw::result rethrows")
{
	aw::result<mockobject> r = std::make_exception_ptr(1);
	chk_exc(int, std::move(r).value());
}

TEST(".value() for rval aw::result<void> rethrows")
{
	aw::result<void> r = std::make_exception_ptr(1);
	chk_exc(int, std::move(r).value());
}

TEST(".value() for const rval aw::result rethrows")
{
	aw::result<mockobject> const r = std::make_exception_ptr(1);
	chk_exc(int, std::move(r).value());
}

TEST(".value() for const rval aw::result<void> rethrows")
{
	aw::result<void> const r = std::make_exception_ptr(1);
	chk_exc(int, std::move(r).value());
}

TEST("aw::result::has_value is true while holding a value")
{
	aw::result<mockobject> r;
	chk r.has_value();
}

TEST("aw::result::has_value is false while holding an error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk !r.has_value();
}

TEST("aw::result::has_value is false while holding an exception")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !r.has_value();
}

TEST("aw::result::has_error_code is false while holding a value")
{
	aw::result<mockobject> r;
	chk !aw::holds_alternative<std::error_code>(r);
}

TEST("aw::result::has_error_code is true while holding an error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk aw::holds_alternative<std::error_code>(r);
}

TEST("aw::result::has_error_code is false while holding an exception")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !aw::holds_alternative<std::error_code>(r);
}

TEST("aw::result::has_exception is false while holding a value")
{
	aw::result<mockobject> r;
	chk !aw::holds_alternative<std::exception_ptr>(r);
}

TEST("aw::result::has_exception is false while holding an error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk !aw::holds_alternative<std::exception_ptr>(r);
}

TEST("aw::result::has_exception is true while holding an exception")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk aw::holds_alternative<std::exception_ptr>(r);
}

TEST("aw::result<void>() shall hold a value")
{
	aw::result<void> r;
	chk r;
	r.get();
}

TEST("aw::result<T> can be constructed with a T in-place")
{
	{
		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), 3, 4);
		chk r;
		chk r.get().value == 7;
	}

	{
		mockobject m = 3;

		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), std::move(m));
		chk r;
		chk r.get().value == 5;
		chk m.value == -1;
	}

	{
		mockobject m = 3;

		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), m);
		chk r;
		chk r.get().value == 5;
		chk m.value == 5;
	}
}

TEST("aw::result<T> properly destroys the value")
{
	int counter = 0;

	{
		aw::result<mockobject> r(aw::in_place_type_t<mockobject>(), &counter);
		chk r;
		chk r.get().value == 3;
		chk counter == 1;
	}

	chk counter == 0;
}

TEST("aw::result<T> can be constructed with error_code")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::error_code>(),
		std::make_error_code(std::errc::invalid_argument));
	chk !r;
	chk_exc(std::system_error, r.get());
}

TEST("aw::result<T> can be constructed with an exception_ptr")
{
	aw::result<mockobject> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !r;
	chk_exc(int, r.get());
}

TEST("aw::result<void> can be constructed with an exception_ptr")
{
	aw::result<void> r(aw::in_place_type_t<std::exception_ptr>(), std::make_exception_ptr(1));
	chk !r;
	chk_exc(int, r.get());
}

TEST("aw::result can initialize implicitly from value")
{
	aw::result<mockobject> r = 3;
	chk r.get().value == 5;
}

TEST("aw::result can initialize implicitly from error_code")
{
	aw::result<mockobject> r = std::make_error_code(std::errc::invalid_argument);
	chk aw::holds_alternative<std::error_code>(r);
	chk_exc(std::system_error, r.get());
}

TEST("aw::result can initialize implicitly from exception_ptr")
{
	aw::result<mockobject> r = std::make_exception_ptr(1);
	chk aw::holds_alternative<std::exception_ptr>(r);
	chk_exc(int, r.get());
}

TEST("aw::result copies correctly")
{
	int c = 0;

	aw::result<mockobject> r1 = &c;
	aw::result<mockobject> r2 = r1;

	chk c == 2;
}

TEST("aw::result moves correctly")
{
	int c = 0;

	aw::result<mockobject> r1 = &c;
	aw::result<mockobject> r2 = std::move(r1);

	chk c == 1;
}

TEST("aw::result copy-assigned correctly")
{
	int c = 0;

	aw::result<mockobject> r1 = &c;
	aw::result<mockobject> r2;
	r2 = r1;

	chk c == 2;
}

TEST("aw::result copy-assigned correctly")
{
	int c = 0;

	aw::result<mockobject> r1 = &c;
	aw::result<mockobject> r2;
	r2 = std::move(r1);

	chk c == 1;
}

TEST("aw::result converts correctly")
{
	aw::result<mockobject> r1 = 1;
	aw::result<basic_mockobject<long>> r2 = r1;

	chk r1.get().value == 3;
	chk r2.get().value == 3;
}

TEST("aw::result converts by move correctly")
{
	aw::result<mockobject> r1 = 1;
	aw::result<basic_mockobject<long>> r2 = std::move(r1);

	chk r2.get().value == 3;
	chk r1.get().value == 3;
}

TEST("get_if can retrieve T from aw::result<T>")
{
	using aw::get_if;

	aw::result<mockobject> r1 = 1;
	chk get_if<mockobject>(r1) != nullptr;
	chk get_if<std::error_code>(r1) == nullptr;
	chk get_if<std::exception_ptr>(r1) == nullptr;
}

TEST("get_if can retrieve error_code from aw::result<T>")
{
	using aw::get_if;

	aw::result<mockobject> r1 = std::make_error_code(std::errc::invalid_argument);
	chk get_if<mockobject>(r1) == nullptr;
	chk get_if<std::error_code>(r1) != nullptr;
	chk get_if<std::exception_ptr>(r1) == nullptr;
}

TEST("get_if can retrieve exception from aw::result<T>")
{
	using aw::get_if;

	aw::result<mockobject> r1 = std::make_exception_ptr(1);
	chk get_if<mockobject>(r1) == nullptr;
	chk get_if<std::error_code>(r1) == nullptr;
	chk get_if<std::exception_ptr>(r1) != nullptr;
}

TEST("get can retrieve T from aw::result<T>")
{
	aw::result<mockobject> r1 = 1;
	chk aw::get<mockobject>(r1).value == 3;
}

TEST("get can retrieve error_code from aw::result<T>")
{
	aw::result<mockobject> r1 = std::make_error_code(std::errc::invalid_argument);
	chk aw::get<std::error_code>(r1) == std::errc::invalid_argument;
}

TEST("get can retrieve exception from aw::result<T>")
{
	aw::result<mockobject> r1 = std::make_exception_ptr(1);
	chk aw::get<std::exception_ptr>(r1) != nullptr;
}

TEST("aw::result<T>::exception can recover an error_code")
{
	aw::result<mockobject> r1 = std::make_error_code(std::errc::invalid_argument);

	std::exception_ptr exc = r1.exception();
	chk exc != nullptr;
	chk_exc(std::system_error, std::rethrow_exception(exc));
}

TEST("aw::result<T>::exception can recover an exception_ptr")
{
	std::exception_ptr exc = std::make_exception_ptr(1);
	aw::result<mockobject> r1 = exc;
	chk r1.exception() == exc;
}

TEST("aw::result<T>::convert_error works with error_code")
{
	aw::result<mockobject> r1 = std::make_error_code(std::errc::invalid_argument);
	aw::result<int> r2 = r1.convert_error<int>();

	chk aw::holds_alternative<std::error_code>(r2);
	chk aw::get<std::error_code>(r2) == std::errc::invalid_argument;
}

TEST("aw::result<T>::convert_error works with exception_ptr")
{
	aw::result<mockobject> r1 = std::make_exception_ptr(1);
	aw::result<int> r2 = r1.convert_error<int>();

	chk aw::holds_alternative<std::exception_ptr>(r2);
	chk_exc(int, r2.value());
}

TEST("aw::result<T> will convert throws during constructions")
{
	aw::result<mockobject> r1{ aw::in_place_type_t<mockobject>(), mockobject_throw, 0 };
	chk aw::holds_alternative<std::exception_ptr>(r1);
}

TEST("aw::result<T> will convert throws into values on copy")
{
	aw::result<mockobject> r1{ aw::in_place_type_t<mockobject>(), mockobject_throw, 1 };
	aw::result<mockobject> r2(r1);

	chk aw::holds_alternative<std::exception_ptr>(r2);
}

TEST("aw::result<T> will convert throws into values on copy")
{
	aw::result<mockobject> r1{ aw::in_place_type_t<mockobject>(), mockobject_throw, 1 };
	aw::result<basic_mockobject<long>> r2(r1);

	chk aw::holds_alternative<std::exception_ptr>(r2);
}

TEST("aw::result<T> will convert throws into values on move")
{
	aw::result<mockobject> r1{ aw::in_place_type_t<mockobject>(), mockobject_throw, 1 };
	aw::result<basic_mockobject<long>> r2(std::move(r1));

	chk aw::holds_alternative<std::exception_ptr>(r2);
}

TEST("aw::result can be constructred with in_place")
{
	aw::result<int> r(aw::in_place, 1);
	chk r.has_value();
	chk r.get() == 1;
}

TEST("aw::result can be compared")
{
	aw::result<int> r = 1;
	chk r == 1;
	chk r != 0;
	chk (1 == r);
	chk (0 != r);
}
