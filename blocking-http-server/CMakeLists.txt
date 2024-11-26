add_executable(blocking-http-server server.cpp)
target_link_libraries(blocking-http-server http-lib)
target_compile_features(blocking-http-server PRIVATE cxx_std_20)
