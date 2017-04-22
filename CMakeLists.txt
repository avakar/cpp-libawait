cmake_minimum_required(VERSION 3.0)
project(avakar_await)

include(deps.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(common_srcs
    include/avakar/await/result.h
    )

if (WIN32)

add_library(avakar_await STATIC
    ${common_srcs}
    )

target_link_libraries(avakar_await ws2_32)

else()

add_library(avakar_await STATIC
    ${common_srcs}
    )

endif()

source_group(include REGULAR_EXPRESSION include/.*)
source_group(src REGULAR_EXPRESSION src/.*)

target_include_directories(
    avakar_await
    PUBLIC include
    )
target_link_libraries(avakar_await avakar_singleton)

add_executable(avakar_await_test
    test/result_test.cpp
    )
target_link_libraries(avakar_await_test avakar_await mutest)
