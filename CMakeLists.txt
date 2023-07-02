cmake_minimum_required(VERSION 3.15)
project(cwj_co_net)
add_compile_options(-std=c++11 -pthread -g -ggdb -O0 -Wall -fstack-protector-all -march=native -fnon-call-exceptions)
add_compile_options(-rdynamic)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(test)

# 添加头文件搜索路径
include_directories(/usr/local/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/http)

# 添加库文件搜索路径
link_directories(/usr/local/lib)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/http-parser)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib/jemalloc)
message(${PROJECT_SOURCE_DIR})

FILE(GLOB_RECURSE SRC_LIST ${CMAKE_SOURCE_DIR}/src/*.cpp)
FILE(GLOB TEST_SRCS ${CMAKE_SOURCE_DIR}/test/*.cpp)

message(${SRC_LIST})
message(${TEST_SRCS})

set(LIB_LIST yaml-cpp pthread dl http_parser jemalloc)

add_library(cwj-co-net STATIC ${SRC_LIST})
target_link_libraries(cwj-co-net ${LIB_LIST})

FOREACH(test_src ${TEST_SRCS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} cwj-co-net)
ENDFOREACH(test_src)