SRCS= \
	test/mutest/mutest/event_sink.cpp \
	test/mutest/mutest/detail/registry.cpp \
	test/mutest/mutest/detail/repr.cpp \
	test/mutest/utest_main.cpp \
	await/detail/linux_fd_task.cpp \
	await/detail/linux_run.cpp \
	await/detail/linux_tcp.cpp \
	await/detail/linux_timer.cpp \
	await/result.cpp \
	await/detail/simple_commands.cpp \
	await/detail/stream.cpp \
	test/result.cpp \
	test/then.cpp \
	test/timer.cpp \
	test/tcp.cpp \
	test/task.cpp

all:
	$(CXX) $(SRCS) -I . -I test/mutest -g -o libawait-test -std=c++11
