cmake_minimum_required(VERSION 3.20)

project(custom_http_client)

find_package(Boost 1.75.0 REQUIRED COMPONENTS system program_options)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_library(
	custom_http_client_lib
	include/CustomClient/ClientState.hpp
	include/CustomClient/Request.hpp
	include/CustomClient/RequestSender.hpp
	include/CustomClient/ResponseParser.hpp

	src/ClientState.cpp
	src/Request.cpp
	src/RequestSender.cpp
	src/ResponseParser.cpp
	)

add_executable(custom_http_client src/main.cpp)

target_include_directories(custom_http_client_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(custom_http_client_lib PRIVATE cxx_std_17)
target_compile_features(custom_http_client PRIVATE cxx_std_17)

target_compile_options(custom_http_client_lib PRIVATE "-stdlib=libstdc++" )
target_compile_options(custom_http_client PRIVATE "-stdlib=libstdc++" )

target_link_libraries(custom_http_client_lib PUBLIC Boost::system Threads::Threads custom_common_http_lib)
target_link_libraries(custom_http_client PRIVATE custom_http_client_lib)
