cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32)
set(TORCH_URL https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-2.7.1%2Bcpu.zip)
else()
set(TORCH_URL https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.7.1%2Bcpu.zip)
endif()

# Dependencies
include(FetchContent)
FetchContent_Declare(
        libtorch
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL ${TORCH_URL}
)
FetchContent_GetProperties(libtorch)
set(FETCHCONTENT_QUIET NO)
FetchContent_MakeAvailable(libtorch)

set(Torch_PATH ${libtorch_SOURCE_DIR})
