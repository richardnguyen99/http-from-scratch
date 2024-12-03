# add source files
set(LIBHTTP_SOURCES
    http_client.cpp
    http_server.cpp
    http_request.cpp
    http_response.cpp
    http_router.cpp
    http_uuid.cpp
)   

add_library(http-lib SHARED ${LIBHTTP_SOURCES})
target_include_directories(http-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(http-lib PUBLIC cxx_std_20)

if (HAVE_UUID_UUID_H)
    target_link_libraries(http-lib uuid ${HFS_SELECTED_JSON_LIBRARY} ${HFS_SELECTED_INJA_LIBRARY})
else()
    target_link_libraries(http-lib ${HFS_SELECTED_JSON_LIBRARY} ${HFS_SELECTED_INJA_LIBRARY})
endif()

