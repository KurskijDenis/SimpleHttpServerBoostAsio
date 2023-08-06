cmake_minimum_required(VERSION 3.20)

project(simple_http_client_server)

set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/boost")
set(Boost_INCLUDE_DIR "${BOOST_ROOT}/")
set(Boost_LIBRARY_DIR "${BOOST_ROOT}/stage/lib")

set(Boost_USE_STATIC_LIBS ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Http)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Server)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Client)
