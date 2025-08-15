cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(nrp C CXX)
#project(nrp C CXX CUDA)

# Enable testing
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
enable_testing()

# Source files
file(GLOB_RECURSE LIBRARY_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/main/*.cpp
)
file(GLOB_RECURSE LIBRARY_HEADER_FILES
        ${CMAKE_SOURCE_DIR}/src/main/*.h
        ${CMAKE_SOURCE_DIR}/src/main/*.hpp
)

file(GLOB_RECURSE EXAMPLE_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/example/*.cpp
)
file(GLOB_RECURSE EXAMPLE_HEADER_FILES
        ${CMAKE_SOURCE_DIR}/src/example/*.h
        ${CMAKE_SOURCE_DIR}/src/example/*.hpp
)

# Dependencies
include(${CMAKE_SOURCE_DIR}/lib/raylib/FindRaylib.cmake)
#include(${CMAKE_SOURCE_DIR}/lib/postgres/FindPostgres.cmake) # Use project's (not system's) PostgreSQL
find_package(PostgreSQL)
include(${CMAKE_SOURCE_DIR}/lib/libtorch/FindTorch.cmake)
include(${CMAKE_SOURCE_DIR}/lib/tinyxml/FindTinyxml.cmake)

# Verify libtorch dependency
message(STATUS "LibTorch version: ${TORCH_VERSION}")
message(STATUS "LibTorch libraries: ${TORCH_LIBRARIES}")
message(STATUS "LibTorch include dirs: ${TORCH_INCLUDE_DIRS}")

# Main library target
add_library(${PROJECT_NAME} STATIC
        ${LIBRARY_SOURCE_FILES} ${LIBRARY_HEADER_FILES}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
        PUBLIC ${CMAKE_SOURCE_DIR}/src/resources
        PUBLIC ${TORCH_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${TORCH_LIBRARIES}
)
#target_link_options(${PROJECT_NAME} PRIVATE ${TORCH_CXX_FLAGS})

# Embedded resource file generation (for example target)
set(EMBED_SOURCE_DIR ${CMAKE_SOURCE_DIR}/embed)
set(EMBEDDED_RESOURCES_HEADER ${CMAKE_SOURCE_DIR}/src/resources/embedded_resources.h)
add_executable(embed ${EMBED_SOURCE_DIR}/embed.c)
file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/resources/*)
set(EMBED_OUTPUT_DIR ${CMAKE_BINARY_DIR}/embedded_resources)
file(MAKE_DIRECTORY ${EMBED_OUTPUT_DIR})
set(EMBEDDED_RESOURCES_SOURCE ${EMBED_OUTPUT_DIR}/embedded_resources.c)
add_custom_command(
        OUTPUT ${EMBEDDED_RESOURCES_SOURCE}
        COMMAND $<TARGET_FILE:embed> ${RESOURCE_FILES} > ${EMBEDDED_RESOURCES_SOURCE}
        DEPENDS embed ${RESOURCE_FILES}
        COMMENT "Embedding all resources into ${EMBEDDED_RESOURCES_SOURCE}"
        VERBATIM
)
add_custom_target(run_embed ALL DEPENDS ${EMBEDDED_RESOURCES_SOURCE})

# Example executable target
add_executable(${PROJECT_NAME}_exec
        ${EXAMPLE_SOURCE_FILES} ${EXAMPLE_HEADER_FILES}
        ${EMBEDDED_RESOURCES_SOURCE}
)

add_dependencies(${PROJECT_NAME}_exec
        ${PROJECT_NAME}
        run_embed
)

target_include_directories(${PROJECT_NAME}_exec
        PUBLIC ${CMAKE_SOURCE_DIR}/src/main
        PUBLIC ${CMAKE_SOURCE_DIR}/src/example
        PUBLIC ${CMAKE_SOURCE_DIR}/src/resources
)

target_link_libraries(${PROJECT_NAME}_exec PRIVATE
        ${PROJECT_NAME}
        raylib
        tinyxml2::tinyxml2
        PostgreSQL::PostgreSQL
)

# Compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4267)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/O2 /Ob2 /GL>)
    target_compile_options(${PROJECT_NAME}_exec PRIVATE $<$<CONFIG:Release>:/O2 /Ob2 /GL>)
    target_link_options(${PROJECT_NAME}_exec PRIVATE $<$<CONFIG:Release>:/LTCG>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        # clang-cl
        target_compile_options(${PROJECT_NAME} PRIVATE -Xclang -Wno-shift-op-parentheses)
    else()
        # regular Clang
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-shift-op-parentheses)
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3 -march=native -flto>)
    target_compile_options(${PROJECT_NAME}_exec PRIVATE $<$<CONFIG:Release>:-O3 -march=native -flto>)
    target_link_options(${PROJECT_NAME}_exec PRIVATE $<$<CONFIG:Release>:-flto>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-shift-op-parentheses)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3 -march=native -flto>)
    target_compile_options(${PROJECT_NAME}_exec PRIVATE $<$<CONFIG:Release>:-O3 -march=native -flto>)
    target_link_options(${PROJECT_NAME}_exec PRIVATE $<$<CONFIG:Release>:-flto>)
endif()

# Disable debug assertions in release builds
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:NDEBUG>)
target_compile_definitions(${PROJECT_NAME}_exec PRIVATE $<$<CONFIG:Release>:NDEBUG>)

# Copy DLLs on Windows
if(MSVC)
    foreach(lib ${TORCH_LIBRARIES})
        get_filename_component(lib_dir ${lib} DIRECTORY)
        file(GLOB TORCH_DLLS "${lib_dir}/*.dll")
        if(TORCH_DLLS)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${TORCH_DLLS}
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endif()
    endforeach()
endif()

# Print build flags
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

# Add tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/tests)
endif()
