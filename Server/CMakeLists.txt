cmake_minimum_required(VERSION 3.20)

project(custom_http_server)

find_package(Boost 1.75.0 REQUIRED COMPONENTS system program_options)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_library(
	custom_http_server_lib
	include/CustomServer/Connection.hpp
	include/CustomServer/HttpRequestConnection.hpp
	include/CustomServer/RequestHandler.hpp
	include/CustomServer/RequestParser.hpp
	include/CustomServer/Server.hpp
	include/CustomServer/ServerState.hpp

	src/Connection.cpp
	src/HttpRequestConnection.cpp
	src/RequestHandler.cpp
	src/RequestParser.cpp
	src/Server.cpp
	src/ServerState.cpp)

add_executable(custom_http_server src/main.cpp)

target_include_directories(custom_http_server_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(custom_http_server_lib PRIVATE cxx_std_17)
target_compile_features(custom_http_server PRIVATE cxx_std_17)

target_compile_options(custom_http_server_lib PRIVATE "-stdlib=libstdc++" )
target_compile_options(custom_http_server PRIVATE "-stdlib=libstdc++" )

target_link_libraries(custom_http_server_lib PUBLIC Boost::system Boost::program_options Threads::Threads custom_common_http_lib)
target_link_libraries(custom_http_server PRIVATE custom_http_server_lib)
