include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_executable(multiprocess-http-server multi_process_http_server.cpp server.cpp)
target_include_directories(multiprocess-http-server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(multiprocess-http-server http-lib)
target_compile_features(multiprocess-http-server PRIVATE cxx_std_20)
