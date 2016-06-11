SRCS= \
	test/mutest/mutest/event_sink.cpp \
	test/mutest/mutest/detail/registry.cpp \
	test/mutest/mutest/detail/repr.cpp \
	test/mutest/utest_main.cpp \
	await/detail/linux_fd_task.cpp \
	await/detail/linux_run.cpp \
	await/detail/linux_tcp.cpp \
	await/detail/linux_timer.cpp \
	await/detail/result.cpp \
	await/detail/stream.cpp \
	await/detail/task.cpp \
	test/result_test.cpp \
	test/then_test.cpp \
	test/timer_test.cpp \
	test/tcp_test.cpp \
	test/task_test.cpp

all:
	$(CXX) $(SRCS) -I . -I test/mutest -g -o libawait-test -std=c++11
