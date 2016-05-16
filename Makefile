SRCS= \
	test/mutest/mutest/event_sink.cpp \
	test/mutest/mutest/detail/registry.cpp \
	test/mutest/mutest/detail/repr.cpp \
	test/mutest/utest_main.cpp \
	await/result.cpp \
	await/detail/simple_commands.cpp \
	test/result.cpp \
	test/task.cpp

all:
	$(CXX) $(SRCS) -I . -I test/mutest -O2 -g -o libawait-test -std=c++11
