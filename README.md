## cwj_co_net



一个基于多协程多线程实现的网络库，可用于快捷的搭建各种服务器，包括包括但不现在于tcpserver,httpserver,rpc框架；

该网络库目前分为以下几个模块：

注：下面标注有`TODO`的是未完成，但有提上日程的模块

1. [日志模块](doc/日志模块.md)
2. 配置模块
3. 线程模块
4. 协程模块
5. 协程模块
6. 协程调度器模块

    1. IO协程调度器(epoll)
    2. 条件变量调度器(相当于线程池)
    
7. 定时器模块（小顶堆+epoll , \[ 时间轮（TODO）\]）
8. hook模块（重构系统调用）
9. Address模块（屏蔽各种类型地址）
10. Socket模块（屏蔽各种socket模块）
11. ByteArray模块（仿环形链表）
12. Stream模块
13. Tcpserver模块（提供了两种封装好的Tcpserver以便使用）
14. 客户端模块（支持UDP,TCP）,
15. 内存池模块（TODO）
16. 数据库模块（TODO）
    1. 数据库连接池（mysql,redis）（TODO）
    2. 客户端使用封装（TODO）
17. 守护进程模块（TODO）


## TODO    
例子
 
1. http模块
2. rpc框架