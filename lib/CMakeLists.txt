# add source files
set(LIBHTTP_SOURCES
    http_client.cpp
    http_server.cpp
    http_request.cpp
    http_response.cpp
    http_router.cpp
    http_uuid.cpp
    http_uri.cpp
)   


add_library(http-lib SHARED ${LIBHTTP_SOURCES})
target_include_directories(http-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(http-lib PUBLIC cxx_std_20)

if (HAVE_UUID_UUID_H)
    message(STATUS "Add linkage libuuid (-luuid)")
    target_link_libraries(http-lib uuid uriparser asan ${HFS_SELECTED_JSON_LIBRARY} ${HFS_SELECTED_INJA_LIBRARY})
else()
    message(STATUS "Excluding libuuid")
    target_link_libraries(http-lib uriparser asan ${HFS_SELECTED_JSON_LIBRARY} ${HFS_SELECTED_INJA_LIBRARY})
endif()

