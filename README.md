## cwj_co_net

一个基于网络事件驱动的协程调度框架，主要用于快捷的搭建各种服务器，包括但不限于tcpserver,httpserver,rpc框架；

### 项目特点

- 利用ucontext实现有栈协程，以网络事件为驱动，对协程进行调度
- 利用hook技术把非阻塞IO操作包装成同步操作，减轻用户编写和调试压力
- 提供丰富的工具模块，以便被一般的项目复用
- 仅支持linux平台

### 各大模块

#### 主模块

协程调度器模块以及其IO子模块

#### 优化模块

- [共享消息多队列机制](doc/共享消息多队列机制.md)(`2023-03-26`引入)
- 分离式协程复用池
- 链式复用缓存区设计
- 内存池优化

#### 工具模块

- 日志系统模块
- 配置系统模块
- 通用的套接字封装类

#### 功能模块

- tcp服务器模块
- http服务器模块

### 框架性能

网络框架性能请移步: [压测报告](doc/stress_report.md)

### NOW

quic协议的实现（模型设计中）

### TODO

1. rpc框架
2. 分布式K-V存储服务
3. http模块的负载均衡
4. slab分配系统优化

### 旧readme

[old_readme](doc/old_readme.md)

