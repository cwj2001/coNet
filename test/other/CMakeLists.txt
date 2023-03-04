

include_directories(../../include)
LINK_DIRECTORIES(../../lib/jemalloc)
set(OTHER_LIB_LIST "-ljemalloc -ldl")

add_executable(create_test_file create_test_file.cpp)
add_executable(test_error test_error.cpp)
add_executable(test_core_error test_core_error.cpp)
add_executable(test_jemalloc test_jemalloc.cpp)

target_link_libraries(test_jemalloc ${OTHER_LIB_LIST})