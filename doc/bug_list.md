
1. logger的全局初始化问题
2. macro 的函数调用栈打印问题
3. scheduler中 如果idle以协程的方式运行，而wake和idle用条件变量实现，会发现条件变量会无法被唤醒
4. 日志的无序递归打印