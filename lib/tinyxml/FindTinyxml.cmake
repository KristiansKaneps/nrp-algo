cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(TINYXML_VERSION 11.0.0)

find_package(tinyxml ${TINYXML_VERSION} QUIET) # QUIET or REQUIRED
if (NOT tinyxml_FOUND) # If there's none, fetch and build tinyxml
    include(FetchContent)
    FetchContent_Declare(
            tinyxml
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
            GIT_TAG        ${TINYXML_VERSION}
    )
    FetchContent_GetProperties(tinyxml)
    if (NOT tinyxml_POPULATED) # Have we downloaded tinyxml yet?
        set(FETCHCONTENT_QUIET NO)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(BUILD_TESTING     OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(tinyxml)
    endif()
endif()
