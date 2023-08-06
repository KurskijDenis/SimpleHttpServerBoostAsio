cmake_minimum_required(VERSION 3.20)

project(custom_common_http_lib)

add_library(
	custom_common_http_lib
	include/Http/Types.hpp
	include/Http/HttpResponse.hpp
	include/Http/HttpRequest.hpp

	src/Types.cpp
	src/HttpResponse.cpp
	src/HttpRequest.cpp)

target_include_directories(custom_common_http_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(custom_common_http_lib PRIVATE cxx_std_17)

target_compile_options(custom_common_http_lib PRIVATE "-stdlib=libstdc++" )
