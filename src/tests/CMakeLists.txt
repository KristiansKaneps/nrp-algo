cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

add_library(test_main OBJECT test_main.cpp)

set(DOCTEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/doctest)

include_directories(${DOCTEST_INCLUDE_DIR})

target_include_directories(test_main
        PUBLIC ${DOCTEST_INCLUDE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
)

add_executable(dummy_test dummy_test.cpp $<TARGET_OBJECTS:test_main>)
add_test(NAME dummy_test COMMAND dummy_test)
