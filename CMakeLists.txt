cmake_minimum_required(VERSION 3.5.0)
project(http-from-scratch VERSION 0.1.0 LANGUAGES C CXX)

# Set options
option(HFS_USE_EMBEDDED_JSON "Use the shipped json header if not available on the system" ON)
option(HFS_USE_EMBEDDED_INJA "Use the shipped inja header if not available on the system" ON)


# If Build type is not set, set it to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


# Check if the compiler supports C++20, if not set the standard to C++17. 
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)

if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_STANDARD 20)
else()
    # Check if the compiler supports C++17
    CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)

    if(COMPILER_SUPPORTS_CXX17)
        set(CMAKE_CXX_STANDARD 17)
    else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
    endif()
endif()

# Find third party libraries
if(HFS_USE_EMBEDDED_JSON)
  # find_package(nlohmann_json QUIET)
  if(NOT nlohmann_json_FOUND)
    set(HFS_PACKAGE_USE_EMBEDDED_JSON ON)
    add_library(nlohmann_json INTERFACE)
    add_library(hfs::nlohmann_json ALIAS nlohmann_json)

    target_include_directories(nlohmann_json INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third-party/include>
      $<INSTALL_INTERFACE:${HFS_INSTALL_INCLUDE_DIR}/nlohmann_json>
    )

    target_compile_features(nlohmann_json INTERFACE cxx_std_17)
    install(TARGETS nlohmann_json EXPORT injaTargets)
    set(HFS_SELECTED_JSON_LIBRARY "hfs::nlohmann_json")

  message(STATUS "Using embedded nlohmann_json library")
  else()
    set(HFS_SELECTED_JSON_LIBRARY "nlohmann_json::nlohmann_json")
  endif()
else()
  # If target already exists, e.g. by git submodules
  if(TARGET nlohmann_json)
    set(HFS_SELECTED_JSON_LIBRARY "nlohmann_json::nlohmann_json")

    install(TARGETS nlohmann_json EXPORT injaTargets)
  else()
    find_package(nlohmann_json REQUIRED)
    set(HFS_SELECTED_JSON_LIBRARY "nlohmann_json::nlohmann_json")
  endif()
endif()

# Find third party libraries
if(HFS_USE_EMBEDDED_INJA)
  # find_package(nlohmann_json QUIET)
  if(NOT pantor_inja_FOUND)
    set(HFS_PACKAGE_USE_EMBEDDED_INJA ON)
    add_library(pantor_inja INTERFACE)
    add_library(hfs::pantor_inja ALIAS pantor_inja)

    target_include_directories(pantor_inja INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third-party/include>
      $<INSTALL_INTERFACE:${HFS_INSTALL_INCLUDE_DIR}/>
    )

    target_compile_features(pantor_inja INTERFACE cxx_std_17)
    install(TARGETS pantor_inja EXPORT injaTargets)
    set(HFS_SELECTED_INJA_LIBRARY "hfs::pantor_inja")

  message(STATUS "Using embedded pantor_inja library")
  else()
    set(HFS_SELECTED_INJA_LIBRARY "pantor_inja::pantor_inja")
  endif()
else()
  # If target already exists, e.g. by git submodules
  if(TARGET pantor_inja)
    set(HFS_SELECTED_INJA_LIBRARY "pantor_inja::pantor_inja")

    install(TARGETS pantor_inja EXPORT injaTargets)
  else()
    find_package(pantor_inja REQUIRED)
    set(HFS_SELECTED_INJA_LIBRARY "pantor_inja::pantor_inja")
  endif()
endif()

# Check if the compiler has certain header files, if not throw an error
include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX(algorithm HAVE_ALGORITHM_H)
if (NOT HAVE_ALGORITHM_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no algorithm support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(iostream HAVE_IOSTREAM_H)
if (NOT HAVE_IOSTREAM_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no iostream support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(sstream HAVE_SSTREAM_H)
if (NOT HAVE_SSTREAM_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no sstream support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(string HAVE_STRING_H)
if (NOT HAVE_STRING_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no string support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(vector HAVE_VECTOR_H)
if (NOT HAVE_VECTOR_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no vector support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(unordered_map HAVE_UNORDERED_MAP_H)
if (NOT HAVE_UNORDERED_MAP_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no unordered_map support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(memory HAVE_MEMORY_H)
if (NOT HAVE_MEMORY_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no memory support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(thread HAVE_THREAD_H)
if (NOT HAVE_THREAD_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no thread support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(future HAVE_FUTURE_H)
if (NOT HAVE_FUTURE_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no future support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(mutex HAVE_MUTEX_H)
if (NOT HAVE_MUTEX_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no mutex support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(condition_variable HAVE_CONDITION_VARIABLE_H)
if (NOT HAVE_CONDITION_VARIABLE_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no condition_variable support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(functional HAVE_FUNCTIONAL_H)
if (NOT HAVE_FUNCTIONAL_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no functional support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(chrono HAVE_CHRONO_H)
if (NOT HAVE_CHRONO_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no chrono support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(optional HAVE_OPTIONAL_H)
if (NOT HAVE_OPTIONAL_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no optional support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(cstring HAVE_CSTRING_H)
if (NOT HAVE_CSTRING_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no cstring support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(fstream HAVE_FSTREAM_H)
if (NOT HAVE_FSTREAM_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no fstream support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(filesystem HAVE_FILESYSTEM_H)
if (NOT HAVE_FILESYSTEM_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no filesystem support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(ios HAVE_IOS_H)
if (NOT HAVE_IOS_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no ios support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE_CXX(utility HAVE_UTILITY_H)
if (NOT HAVE_UTILITY_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no utility support. Please use a different C++ compiler.")
endif()

# Check some posix headers, if not throw an error
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
if (NOT HAVE_UNISTD_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no unistd.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
if (NOT HAVE_SYS_TYPES_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no sys/types.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
if (NOT HAVE_SYS_SOCKET_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no sys/socket.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(netinet/in.h HAVE_NETINET_IN_H)
if (NOT HAVE_NETINET_IN_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no netinet/in.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARPA_INET_H)
if (NOT HAVE_ARPA_INET_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no arpa/inet.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
if (NOT HAVE_FCNTL_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no fcntl.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(sys/epoll.h HAVE_SYS_EPOLL_H)
if (NOT HAVE_SYS_EPOLL_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no sys/epoll.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(sys/wait.h HAVE_SYS_WAIT_H)
if (NOT HAVE_SYS_WAIT_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no sys/wait.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(sys/mman.h HAVE_SYS_MMAN_H)
if (NOT HAVE_SYS_MMAN_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no sys/mman.h support. Please use a different C++ compiler.")
endif()
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
if (NOT HAVE_SYS_STAT_H)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no sys/stat.h support. Please use a different C++ compiler.")
endif()


# Check if the compiler has some optional headers. If so, set the HAVE_XXX_H to 1
CHECK_INCLUDE_FILE_CXX(sys/sendfile.h HAVE_SYS_SENDFILE_H)
CHECK_INCLUDE_FILE_CXX(sys/uio.h HAVE_SYS_UIO_H)
CHECK_INCLUDE_FILE_CXX(sys/eventfd.h HAVE_SYS_EVENTFD_H)
CHECK_INCLUDE_FILE_CXX(sys/signalfd.h HAVE_SYS_SIGNALFD_H)
CHECK_INCLUDE_FILE_CXX(sys/timerfd.h HAVE_SYS_TIMERFD_H)
CHECK_INCLUDE_FILE_CXX(cstdbool HAVE_CSTDBOOL_H)
CHECK_INCLUDE_FILE_CXX(cstdint HAVE_CSTDINT_H)
CHECK_INCLUDE_FILE_CXX(cstddef HAVE_CSTDDEF_H)
CHECK_INCLUDE_FILE_CXX(uuid/uuid.h HAVE_UUID_UUID_H)



# Add these definitions to the header config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/lib/http_config.h)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror -pedantic")


# If buildd type is debug, set the flags to debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
endif()


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy public folder to the build directory
file(COPY public DESTINATION ${CMAKE_BINARY_DIR})

include_directories(lib)

add_subdirectory(lib)

add_subdirectory(blocking-http-server)
add_subdirectory(multi-thread-http-server)
add_subdirectory(multi-process-http-server)
