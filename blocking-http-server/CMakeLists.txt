include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_executable(blocking-http-server blocking_http_server.cpp server.cpp)
target_include_directories(blocking-http-server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(blocking-http-server http-lib)
target_compile_features(blocking-http-server PRIVATE cxx_std_20)
