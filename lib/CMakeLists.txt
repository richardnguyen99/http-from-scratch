# add source files
file(GLOB LIBHTTP_SOURCES "*.cpp")

add_library(http-lib SHARED ${LIBHTTP_SOURCES})
target_include_directories(http-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(http-lib PUBLIC cxx_std_20)

if (HAVE_UUID_UUID_H)
    target_link_libraries(http-lib uuid)
endif()
