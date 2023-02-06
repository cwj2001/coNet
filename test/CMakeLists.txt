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
aux_source_directory(../src/http HTTP_LIST)

set(SRC_LIST ${SRC_LIST} ${HTTP_LIST})

MESSAGE(${SRC_LIST})

set(LIB_LIST "-lyaml-cpp -lpthread -ldl")

MESSAGE(${LIB_LIST})



add_executable(main main.cpp ${SRC_LIST})
add_executable(test_address test_address.cpp ${SRC_LIST})
add_executable(test_byteArray test_byteArray.cpp ${SRC_LIST})
add_executable(test_config test_config.cpp ${SRC_LIST})
add_executable(test_coroutine test_coroutine.cpp ${SRC_LIST})
add_executable(test_fdmanager test_fdmanager.cpp ${SRC_LIST})
add_executable(test_hook test_hook.cpp ${SRC_LIST})
add_executable(test_http test_http.cpp ${SRC_LIST})
add_executable(test_iomanager test_iomanager.cpp ${SRC_LIST})
add_executable(test_log test_log.cpp  ${SRC_LIST})
add_executable(test_marco test_marco.cpp ${SRC_LIST})
add_executable(test_mutex test_mutex.cpp ${SRC_LIST})
add_executable(test_scheduler test_scheduler.cpp ${SRC_LIST})
add_executable(test_socket test_socket.cpp ${SRC_LIST})
add_executable(test_tcpserver test_tcpserver.cpp ${SRC_LIST})
add_executable(test_template ${SRC_LIST})
add_executable(test_thread test_thread.cpp ${SRC_LIST})
add_executable(test_timer test_timer.cpp ${SRC_LIST})


#target_link_libraries( test_config ${yaml-cpp_LIBS} )

target_link_libraries( main ${LIB_LIST} )
target_link_libraries(test_address ${LIB_LIST} )
target_link_libraries(test_byteArray ${LIB_LIST} )
target_link_libraries( test_config ${LIB_LIST} )
target_link_libraries( test_coroutine ${LIB_LIST})
target_link_libraries( test_fdmanager ${LIB_LIST})
target_link_libraries( test_hook ${LIB_LIST} -ldl)
target_link_libraries( test_http ${LIB_LIST})
target_link_libraries( test_iomanager ${LIB_LIST})
target_link_libraries( test_log ${LIB_LIST} )
target_link_libraries( test_marco ${LIB_LIST})
target_link_libraries( test_mutex ${LIB_LIST})
target_link_libraries( test_scheduler ${LIB_LIST})
target_link_libraries( test_socket ${LIB_LIST})
target_link_libraries( test_tcpserver ${LIB_LIST})
target_link_libraries( test_thread ${LIB_LIST})
target_link_libraries( test_timer ${LIB_LIST})