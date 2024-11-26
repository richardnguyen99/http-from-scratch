include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_executable(multithread-http-server multi_thread_http_server.cpp server.cpp)
target_include_directories(multithread-http-server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(multithread-http-server http-lib)
target_compile_features(multithread-http-server PRIVATE cxx_std_20)
