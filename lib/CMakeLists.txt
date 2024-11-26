add_library(http-lib SHARED http_client.cpp http_server.cpp)
target_include_directories(http-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(http-lib PUBLIC cxx_std_20)
