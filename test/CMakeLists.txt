include_directories(../src)
include_directories(../include)
LINK_DIRECTORIES(../lib)
find_package( yaml-cpp REQUIRED )
find_package(Boost COMPONENTS regex system REQUIRED)


#include_directories( ${yaml-cpp_INCLUDE_DIRS} )
set(BOOST_ROOT "/usr/local/include/boost")
#添加头文件搜索路径
include_directories(/usr/local/include)
#添加库文件搜索路径
link_directories(/usr/local/lib)

add_executable(main main.cpp ../src/util.cpp ../src/config.cpp ../src/log.cpp)
add_executable(testLog testLog.cpp ../src/log.h ../src/log.cpp ../src/singleton.h)
add_executable(test_config
        ../src/singleton.h test_config.cpp ../src/config.cpp ../src/config.h ../src/log.cpp
        ../src/util.cpp)
#target_link_libraries( test_config ${yaml-cpp_LIBS} )
target_link_libraries( test_config -lyaml-cpp )
target_link_libraries( main -lyaml-cpp )