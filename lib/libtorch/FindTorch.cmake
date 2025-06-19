cmake_minimum_required(VERSION 3.11)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configurable LibTorch version and build type
set(TORCH_VERSION "2.7.1" CACHE STRING "LibTorch version")
set(TORCH_BUILD_TYPE "cpu" CACHE STRING "LibTorch build type (cpu or cu121 for CUDA 12.1)")

# Set URL based on platform and build type
if(WIN32)
    set(TORCH_URL_BASE "https://download.pytorch.org/libtorch/${TORCH_BUILD_TYPE}/libtorch-win-shared-with-deps")
else()
    set(TORCH_URL_BASE "https://download.pytorch.org/libtorch/${TORCH_BUILD_TYPE}/libtorch-cxx11-abi-shared-with-deps")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TORCH_URL_BASE "${TORCH_URL_BASE}-debug")
endif()
set(TORCH_URL "${TORCH_URL_BASE}-${TORCH_VERSION}%2B${TORCH_BUILD_TYPE}.zip")

# Download LibTorch
include(FetchContent)
set(FETCHCONTENT_QUIET NO)
FetchContent_Declare(
        libtorch
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL ${TORCH_URL}
)
FetchContent_MakeAvailable(libtorch)

# Set Torch_PATH
set(Torch_PATH ${libtorch_SOURCE_DIR})
if(NOT EXISTS "${Torch_PATH}")
    message(FATAL_ERROR "LibTorch not found at ${Torch_PATH}")
endif()
message(STATUS "LibTorch downloaded to ${Torch_PATH}")

# Ensure find_package(Torch) works
list(APPEND CMAKE_PREFIX_PATH "${Torch_PATH}")
find_package(Torch QUIET)
if(NOT Torch_FOUND)
    message(FATAL_ERROR "Torch CMake config not found in ${Torch_PATH}/share/cmake/Torch")
endif()
