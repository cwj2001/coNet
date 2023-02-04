include_directories(../src)
include_directories(../include)
LINK_DIRECTORIES(../lib)
find_package( yaml-cpp REQUIRED )
find_package(Boost COMPONENTS regex system REQUIRED)

add_compile_options(-rdynamic)

#include_directories( ${yaml-cpp_INCLUDE_DIRS} )
set(BOOST_ROOT "/usr/local/include/boost")
#添加头文件搜索路径
include_directories(/usr/local/include)
#添加库文件搜索路径
link_directories(/usr/local/lib)

aux_source_directory(../src SRC_LIST) # 定义变量，存储当前目录下的所有源文件

MESSAGE(${SRC_LIST})

add_executable(test_template test_template.cpp)

add_executable(main main.cpp ${SRC_LIST})
add_executable(test_address test_address.cpp ${SRC_LIST})
add_executable(test_config test_config.cpp ${SRC_LIST})
add_executable(test_coroutine test_coroutine.cpp ${SRC_LIST})
add_executable(test_iomanager test_iomanager.cpp ${SRC_LIST})
add_executable(test_log test_log.cpp  ${SRC_LIST})
add_executable(test_marco test_marco.cpp ${SRC_LIST})
add_executable(test_mutex test_mutex.cpp ${SRC_LIST})
add_executable(test_scheduler test_scheduler.cpp ${SRC_LIST})
add_executable(test_socket test_socket.cpp ${SRC_LIST})
add_executable(test_thread test_thread.cpp ${SRC_LIST})
add_executable(test_timer test_timer.cpp ${SRC_LIST})


#target_link_libraries( test_config ${yaml-cpp_LIBS} )

target_link_libraries( main -lyaml-cpp -lpthread )
target_link_libraries(test_address -lyaml-cpp -lpthread )
target_link_libraries( test_config -lyaml-cpp -lpthread )
target_link_libraries( test_coroutine -lpthread -lyaml-cpp)
target_link_libraries( test_iomanager -lpthread -lyaml-cpp)
target_link_libraries( test_log -lyaml-cpp -lpthread )
target_link_libraries( test_marco -lyaml-cpp -lpthread)
target_link_libraries( test_mutex -lpthread -lyaml-cpp)
target_link_libraries( test_scheduler -lpthread -lyaml-cpp)
target_link_libraries( test_socket -lpthread -lyaml-cpp)
target_link_libraries( test_thread -lpthread -lyaml-cpp)
target_link_libraries( test_timer -lpthread -lyaml-cpp)