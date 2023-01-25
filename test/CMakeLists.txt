include_directories(../src)

add_executable(main main.cpp)
add_executable(testLog testLog.cpp ../src/log.h ../src/log.cpp ../src/singleton.h)