cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

add_library(test_main OBJECT test_main.cpp)

set(DOCTEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/doctest)

target_include_directories(test_main
        PRIVATE ${DOCTEST_INCLUDE_DIR}
        PRIVATE ${CMAKE_SOURCE_DIR}/src/main
)

# Dummy test
#add_executable(dummy_test dummy_test.cpp $<TARGET_OBJECTS:test_main>)
#add_test(NAME dummy_test COMMAND dummy_test)

# Function to add tests (syntax: `test(testName)` corresponds to testName.cpp file).
function(test testName)
    add_executable(${testName} ${testName}.cpp $<TARGET_OBJECTS:test_main>)
    target_include_directories(${testName} PRIVATE ${DOCTEST_INCLUDE_DIR})
    target_link_libraries(${testName} PRIVATE nrp)
    # Compiler options
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${testName} PRIVATE -Wno-shift-op-parentheses)
        target_compile_options(${testName} PRIVATE $<$<CONFIG:Release>:-O3 -march=native -flto>)
        target_link_options(${testName} PRIVATE $<$<CONFIG:Release>:-flto>)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${testName} PRIVATE $<$<CONFIG:Release>:-O3 -march=native -flto>)
        target_link_options(${testName} PRIVATE $<$<CONFIG:Release>:-flto>)
    elseif(MSVC)
        target_compile_options(${testName} PRIVATE /wd4267)
        target_compile_options(${testName} PRIVATE $<$<CONFIG:Release>:/O2 /Ob2 /GL>)
        target_link_options(${testName} PRIVATE $<$<CONFIG:Release>:/LTCG>)
    endif()
    # Disable debug assertions in release builds
    target_compile_definitions(${testName} PRIVATE $<$<CONFIG:Release>:NDEBUG>)
    # Add test
    add_test(NAME ${testName} COMMAND ${testName})
endfunction()

# Add tests.
include(AddTests.cmake)
