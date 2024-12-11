cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

project(nrp C CXX)

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
if (WIN32)
    set(LIB_BITARRAY "")
else()
    set(LIB_BITARRAY_DIR ${CMAKE_SOURCE_DIR}/lib/bitarray)
    file(GLOB_RECURSE LIB_BITARRAY
            ${LIB_BITARRAY_DIR}/*.h
            ${LIB_BITARRAY_DIR}/*.c
    )
endif()

include(${CMAKE_SOURCE_DIR}/lib/raylib/FindRaylib.cmake)

# Example executable
add_executable(${PROJECT_NAME}_exec
        ${LIBRARY_SOURCE_FILES} ${LIBRARY_HEADER_FILES}
        ${EXAMPLE_SOURCE_FILES} ${EXAMPLE_HEADER_FILES}
        ${LIB_BITARRAY}
)

target_include_directories(${PROJECT_NAME}_exec
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
        PUBLIC ${LIB_BITARRAY_DIR}
)

target_link_libraries(${PROJECT_NAME}_exec raylib)

# Main library
add_library(${PROJECT_NAME} STATIC
        ${LIBRARY_SOURCE_FILES} ${LIBRARY_HEADER_FILES}
        ${LIB_BITARRAY}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC  ${LIB_BITARRAY_DIR}
)
