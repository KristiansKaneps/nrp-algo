cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(nrp C CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-shift-op-parentheses)
endif()

# Enable testing
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
enable_testing()

# Add all Source Files
file(GLOB_RECURSE LIBRARY_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/main/*.cpp
)

# Add all Header Files
file(GLOB_RECURSE LIBRARY_HEADER_FILES
        ${CMAKE_SOURCE_DIR}/src/main/*.h
        ${CMAKE_SOURCE_DIR}/src/main/*.hpp
)

# Add all Source Files
file(GLOB_RECURSE EXAMPLE_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/example/*.cpp
)

# Add all Header Files
file(GLOB_RECURSE EXAMPLE_HEADER_FILES
        ${CMAKE_SOURCE_DIR}/src/example/*.h
        ${CMAKE_SOURCE_DIR}/src/example/*.hpp
)

# Libraries
include(${CMAKE_SOURCE_DIR}/lib/raylib/FindRaylib.cmake)
#include(${CMAKE_SOURCE_DIR}/lib/postgres/FindPostgres.cmake) # Use project's (not system's) PostgreSQL
find_package(PostgreSQL)

# Example executable
add_executable(${PROJECT_NAME}_exec
        ${LIBRARY_SOURCE_FILES} ${LIBRARY_HEADER_FILES}
        ${EXAMPLE_SOURCE_FILES} ${EXAMPLE_HEADER_FILES}
        ${LIB_BITARRAY}
)

target_include_directories(${PROJECT_NAME}_exec
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
        PUBLIC ${CMAKE_SOURCE_DIR}/src/example
)

target_link_libraries(${PROJECT_NAME}_exec PRIVATE
        raylib
        PostgreSQL::PostgreSQL
)

# Main library
add_library(${PROJECT_NAME} STATIC
        ${LIBRARY_SOURCE_FILES} ${LIBRARY_HEADER_FILES}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "CXX Release Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
else()
    message(STATUS "CXX Debug Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/tests)
endif()
