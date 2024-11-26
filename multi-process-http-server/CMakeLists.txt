add_executable(multiprocess-http-server server.cpp)
target_link_libraries(multiprocess-http-server http-lib)
target_compile_features(multiprocess-http-server PRIVATE cxx_std_20)
