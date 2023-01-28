include_directories(../src)
include_directories(../include)
LINK_DIRECTORIES(../lib)
find_package( yaml-cpp REQUIRED )

#include_directories( ${yaml-cpp_INCLUDE_DIRS} )

add_executable(main main.cpp)
add_executable(testLog testLog.cpp ../src/log.h ../src/log.cpp ../src/singleton.h)
add_executable(test_config  ../src/singleton.h test_config.cpp ../src/config.cpp ../src/config.h ../src/log.cpp)

#target_link_libraries( test_config ${yaml-cpp_LIBS} )
target_link_libraries( test_config -lyaml-cpp)