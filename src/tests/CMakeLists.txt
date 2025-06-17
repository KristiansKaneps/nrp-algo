cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

include(${CMAKE_SOURCE_DIR}/lib/libtorch/FindTorch.cmake)
list(APPEND CMAKE_PREFIX_PATH "${Torch_PATH}/share/cmake/Torch")
find_package(Torch REQUIRED)

add_library(test_main OBJECT test_main.cpp)

set(DOCTEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/doctest)

include_directories(${DOCTEST_INCLUDE_DIR})

target_include_directories(test_main
        PUBLIC ${DOCTEST_INCLUDE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
)

# Dummy test
#add_executable(dummy_test dummy_test.cpp $<TARGET_OBJECTS:test_main>)
#add_test(NAME dummy_test COMMAND dummy_test)

# Function to add tests (syntax: `test(testName)` corresponds to testName.cpp file).
function(test testName)
    add_executable(${testName} ${testName}.cpp $<TARGET_OBJECTS:test_main> ${LIBRARY_SOURCE_FILES} ${LIBRARY_HEADER_FILES})
    target_include_directories(${testName}
            PUBLIC ${CMAKE_SOURCE_DIR}/src/main
            PUBLIC ${TORCH_INCLUDE_DIRS}
    )
    target_link_libraries(${testName} PUBLIC torch)
    target_link_options(${testName} PUBLIC ${TORCH_CXX_FLAGS})
    add_test(NAME ${testName} COMMAND ${testName})
endfunction()

# Add tests.
include(AddTests.cmake)
