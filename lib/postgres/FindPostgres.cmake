cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+

# Dependencies
set(POSTGRES_VERSION 15.10)

string(FIND ${POSTGRES_VERSION} "." postgres_MAJOR_VERSION_separator_index)
string(SUBSTRING ${POSTGRES_VERSION} 0 ${postgres_MAJOR_VERSION_separator_index} POSTGRES_MAJOR_VERSION)
string(LENGTH ${POSTGRES_VERSION} postgres_VERSION_length)
math(EXPR postgres_MINOR_VERSION_start "${postgres_MAJOR_VERSION_separator_index} + 1")
math(EXPR postgres_MINOR_VERSION_length "${postgres_VERSION_length} - ${postgres_MAJOR_VERSION_separator_index} - 1")
string(SUBSTRING ${POSTGRES_VERSION} ${postgres_MINOR_VERSION_start} ${postgres_MINOR_VERSION_length} POSTGRES_MINOR_VERSION)
set(POSTGRES_VERSION_TAG "REL_${POSTGRES_MAJOR_VERSION}_${POSTGRES_MINOR_VERSION}")

if (NOT PostgreSQL_FOUND)
  find_package(postgres ${POSTGRES_VERSION} QUIET) # QUIET or REQUIRED
  if (NOT postgres_FOUND) # If there's none, fetch and build
    include(FetchContent)
    FetchContent_Declare(
      postgres
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      URL https://github.com/postgres/postgres/archive/refs/tags/${POSTGRES_VERSION_TAG}.tar.gz
    )
    FetchContent_GetProperties(postgres)
    if (NOT postgres_POPULATED) # Have we downloaded yet?
      set(FETCHCONTENT_QUIET NO)
      FetchContent_MakeAvailable(postgres)
      set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    endif()
  endif()

  if(WIN32)
    set(VCBUILD_BAT_PATH "${postgres_SOURCE_DIR}/src/tools/msvc/build.bat")
    # Build PostgreSQL on Windows using MSBuild
    execute_process(COMMAND cmd /c "${VCBUILD_BAT_PATH}"
            WORKING_DIRECTORY ${postgres_SOURCE_DIR}/src/tools/msvc)
  else()
    # Build PostgreSQL on Linux
    # TODO: Take example from Windows and create the same flow on Linux
    execute_process(COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/postgresql
            WORKING_DIRECTORY ${postgres_SOURCE_DIR})
    execute_process(COMMAND make -j
            WORKING_DIRECTORY ${postgres_SOURCE_DIR})
    execute_process(COMMAND make install
            WORKING_DIRECTORY ${postgres_SOURCE_DIR})
  endif()

  set(PostgreSQL_ADDITIONAL_VERSIONS ${POSTGRES_VERSION})
  set(PostgreSQL_ROOT ${postgres_SOURCE_DIR})
  set(PostgreSQL_ADDITIONAL_SEARCH_PATHS ${PostgreSQL_ADDITIONAL_SEARCH_PATHS} ${postgres_SOURCE_DIR})
  if(WIN32)
    set(PostgreSQL_LIBRARY "${postgres_SOURCE_DIR}/Release/libpq/libpq.dll")
  else()
    set(PostgreSQL_LIBRARY "${postgres_SOURCE_DIR}/Release/libpq/libpq.so")
  endif()
  set(PostgreSQL_INCLUDE_DIR "${postgres_SOURCE_DIR}/src/include")
endif()
