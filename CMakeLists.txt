cmake_minimum_required(VERSION 3.15)
project(cwj_co_net)
add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -Wall   -fstack-protector-all -march=native -fnon-call-exceptions)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(test)
add_subdirectory(src)