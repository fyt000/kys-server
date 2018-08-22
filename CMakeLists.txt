project(kys-server)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_COMPILER $ENV{CXX})
set(CMAKE_CXX_FLAGS "-O2 -std=c++14 -pthread")

find_package(Boost 1.67.0 REQUIRED COMPONENTS system coroutine)

file(GLOB SERVER_SRC_LIST src/*.cpp)
file(GLOB CLIENT_SRC_LIST kys-test-client/*.cpp)

add_executable(kys-server ${SERVER_SRC_LIST})
add_executable(kys-test-client ${CLIENT_SRC_LIST})

target_link_libraries(kys-server
    Boost::system
    Boost::coroutine
)

target_link_libraries(kys-test-client
    Boost::system
    Boost::coroutine
)
