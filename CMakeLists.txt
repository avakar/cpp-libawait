cmake_minimum_required(VERSION 3.0)
project(avakar_await)

include(deps.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(common_srcs
    include/avakar/await/cancel.h
    include/avakar/await/coroutine.h
    include/avakar/await/fiber.h
    include/avakar/await/result.h
    include/avakar/await/run.h
    include/avakar/await/stream.h
    include/avakar/await/task.h
    include/avakar/await/tcp.h
    include/avakar/await/timer.h

    src/command.h
    src/command_impl.h
    src/command_intf.h
    src/command_ptr_fwd.h
    src/command_ptr_impl.h
    src/intrusive_list.h
    src/loop_impl.h
    src/parallel.cpp
    src/result.cpp
    src/result_impl.h
    src/result_kind.h
    src/run_impl.h
    src/stream.cpp
    src/task.cpp
    src/task_fwd.h
    src/task_impl.h
    src/task_storage.h
    src/task_traits.h
    src/task_traits_impl.h
    src/task_utils_fwd.h
    src/task_utils_impl.h
    src/then_impl.h
    src/then_traits.h
    )

if (WIN32)

add_library(avakar_await STATIC
    ${common_srcs}
    src/win32_error.cpp
    src/win32_error.h
    src/win32_fiber.cpp
    src/win32_run.cpp
    src/win32_scheduler.h
    src/win32_tcp.cpp
    src/win32_timer.cpp
    )

target_link_libraries(avakar_await ws2_32)

else()

add_library(avakar_await STATIC
    ${common_srcs}
    src/linux_error.h
    src/linux_fd_task.cpp
    src/linux_fd_task.h
    src/linux_fiber.cpp
    src/linux_run.cpp
    src/linux_scheduler.h
    src/linux_tcp.cpp
    src/linux_timer.cpp
    )

endif()

source_group(public REGULAR_EXPRESSION include/.*)
source_group(src REGULAR_EXPRESSION src/.*)

target_include_directories(
    avakar_await
    PUBLIC include
    )
target_link_libraries(avakar_await avakar_singleton)

add_executable(avakar_await_test
    test/fiber_test.cpp
    test/parallel_test.cpp
    test/result_test.cpp
    test/task_test.cpp
    test/tcp_test.cpp
    test/then_test.cpp
    test/timer_test.cpp
    )
target_link_libraries(avakar_await_test avakar_await mutest)
