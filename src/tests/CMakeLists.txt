cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

add_library(test_main OBJECT test_main.cpp)

include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
        doctest
        PREFIX ${CMAKE_BINARY_DIR}/doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
)
ExternalProject_Get_Property(doctest source_dir)
set(DOCTEST_INCLUDE_DIR ${source_dir}/doctest CACHE INTERNAL "Path to include folder for doctest")

include_directories(${DOCTEST_INCLUDE_DIR})

target_include_directories(test_main
        PUBLIC ${DOCTEST_INCLUDE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
)

add_executable(dummy_test dummy_test.cpp $<TARGET_OBJECTS:test_main>)
add_test(NAME dummy_test COMMAND dummy_test)
