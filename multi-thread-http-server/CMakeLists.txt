add_executable(multithread-http-server server.cpp)
target_link_libraries(multithread-http-server http-lib)
target_compile_features(multithread-http-server PRIVATE cxx_std_20)
