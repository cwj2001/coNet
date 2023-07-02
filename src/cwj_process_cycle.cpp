//
// Created by 抑~风 on 2023-04-15.
//

#include <signal.h>
#include <cstring>
#include <socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "util.h"
#include "log.h"
#include "process.h"
#include "macro.h"
#include "cwj_channel.h"


#include "cwj_process_cycle.h"
#include "process.h"

namespace CWJ_CO_NET{

    static auto g_logger = GET_LOGGER("system");

    enum class ProcessType{
        MASTER,
        WORKER,
        UNKNOW,
    };

    static Process g_processes[1024];
    static uint32_t G_PROCESSES_MAX_LEN = 1024;
    static uint32_t g_processes_cur_len = 0;
    static int g_process_ind;
    static int g_master_channel = 0;
    static ProcessType g_process_type = ProcessType::UNKNOW;

    // 全局控制位
    static bool     g_is_quit = false;
    static bool     g_is_term = false;
    static volatile size_t   g_is_poll = 0;
    static bool     g_is_update = false;
    static bool     g_reload = false;


    static inline  Process &Get_Process(){
        return g_processes[g_process_ind];
    }

    static void Close_channel(int ind){
        CWJ_ASSERT(ind < g_processes_cur_len);
        auto &one = g_processes[ind].m_channel;
        if(one[0] != -1)  close(one[0]),one[0] = -1;
        if(one[1] != -1)  close(one[1]),one[1] = -1;
    }

    void CWJ_CO_NET::MasterProcess::start_process_cycle() {
        init_signals();
        init_master_process();
        start_work_processes();

        sigset_t  set;

        while (true){

            m_master_cycle(m_iom);

            sigemptyset(&set);

            sigsuspend(&set);

            if(g_is_quit){
                Channel chl = {Channel::Command::CMD_QUIT,-1,G_PROCESSES_MAX_LEN,-1};
                passOpenChannel(-1,chl);
                int state = 0;
                for(int i=0;i<g_processes_cur_len;i++){
                    auto &one = g_processes[i];
                    if(waitpid(one.m_pid,&state,0) == -1){
                        ERROR_LOG(g_logger) << "waitpid : "<<one.m_pid << " fail ";
                    }
                }
                break;
            }

            int pid = -1;

            if (g_is_poll > 0) { //父进程收到一个子进程退出的信号，见ngx_signal_handler
                //这个里面处理退出的子进程(有的worker异常退出，这时我们就需要重启这个worker )，如果所有子进程都退出则会返回0.
                int status  = 0;
                pid = waitpid(-1,&status,WNOHANG);

                INFO_LOG(g_logger) << "master waitpid :"<<pid;

                CWJ_ASSERT(pid > 0);


                if(pid > 0){
                    g_is_poll --;
                    int slot = reclaimChildren(pid);
                    spawnWorker(slot); // 有子进程意外结束，这时需要监控所有的子进程，也就是reap_children方法所做的工作
                }
            }

            if(g_is_term){
                // TODO 可以加入延时控制，然后用时钟来进行信号控制
                signalWorkerProcesses(SIGKILL);
            }

            if(g_is_update){

            }

            if(g_reload){

            }

        }

    }

    bool MasterProcess::init_signals() {
        struct sigaction  sa;
        for(auto ctx : m_signal_ctx_master){
            memset(&sa,0,sizeof(0));
            sa.sa_handler = ctx.m_handler;
            sigemptyset(&sa.sa_mask);
            if(sigaction(ctx.m_signo,&sa, nullptr) == -1){
                ERROR_LOG(g_logger) << "sigaction("<<ctx.m_sig_name<<") failed";
                return false;
            }
        }
        return true;
    }

    bool MasterProcess::init_master_process() {

        g_process_type = ProcessType::MASTER;

        sigset_t  set;
        sigemptyset(&set);

        g_is_quit = false;
        g_is_term = false;
        g_is_update = false;
        g_is_poll = 0;
        g_reload = false;

        for(auto ctx : m_signal_ctx_master){
            sigaddset(&set,ctx.m_signo);
        }
        if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
            //父子进程的继承关系可以参考:http://blog.chinaunix.net/uid-20011314-id-1987626.html
            ERROR_LOG(g_logger) << "sigprocmask() failed" << "errno="<<errno<<" strerror= " << strerror(errno);
        }

        sigemptyset(&set);

        SetProcessName(m_master_name);

        return true;

    }

    bool MasterProcess::start_work_processes() {
        INFO_LOG(g_logger) << "asterProcess::start_work_processes()" ;
        for(int i=0;i<m_worker_size;i++){
            spawnWorker(i);
        }
    }

    void MasterProcess::spawnWorker(int slot) {
        Channel chl;
        uint32_t ind = spawn_process(m_worker_name + "_" + std::__cxx11::to_string(slot), 0);

        if(g_process_type == ProcessType::MASTER) {
            auto &one = g_processes[ind];
            chl.m_pid = one.m_pid;
            chl.m_cmd = Channel::CMD_OPEN_CHANNEL;
            chl.m_fd = one.m_channel[0];
            INFO_LOG(g_logger) << "spawn_process pid = "<<one.m_pid;
            passOpenChannel(ind, chl);
        }
    }


    uint32_t MasterProcess::spawn_process(std::string name,
                                          void *data) {

        int ind = g_processes_cur_len;
        for(int i=0;i<g_processes_cur_len;i++){
            if(g_processes[i].m_pid == -1){
                ind = i;
                break;
            }
        }

        INFO_LOG(g_logger) << " === process ind = " <<ind;

        if(ind == g_processes_cur_len){
            if(g_processes_cur_len >= G_PROCESSES_MAX_LEN){
                ERROR_LOG(g_logger) << " process create fail, too much ";
                return -1;
            }else{
                ind = g_processes_cur_len;
                g_processes_cur_len += 1;
                INFO_LOG(g_logger) << " ==========  g_processes_cur_len = "<<g_processes_cur_len;
            }
        }

        if(socketpair(AF_UNIX,SOCK_STREAM,0,g_processes[ind].m_channel) == -1){
            ERROR_LOG(g_logger) << "socketpair() failed while spawning " << name ;
            return -1;
        }

        /* 设置master的channel[0](即写端口)，channel[1](即读端口)均为非阻塞方式 */
        if (SetNonblock(g_processes[ind].m_channel[0]) == -1) {
            ERROR_LOG(g_logger) << g_processes[ind].m_channel[0]
                                <<" set nonblock failed while spawning "
                                <<name << "errno="<<errno
                                <<" strerrno= "<<strerror(errno);
            Close_channel(ind);
            CWJ_ASSERT(false);
            return -1;
        }

        if (SetNonblock(g_processes[ind].m_channel[1]) == -1) {
            ERROR_LOG(g_logger) << g_processes[ind].m_channel[0]
                                <<" set nonblock failed while spawning "
                                <<name << "errno="<<errno
                                <<" strerrno= "<<strerror(errno);
            Close_channel(ind);
            CWJ_ASSERT(false);
            return -1;
        }

        /*
            设置异步模式： 这里可以看下《网络编程卷一》的ioctl函数和fcntl函数 or 网上查询
          */
        int on = 1; // 标记位，ioctl用于清除（0）或设置（非0）操作

        /*
          设置channel[0]的信号驱动异步I/O标志
          FIOASYNC：该状态标志决定是否收取针对socket的异步I/O信号（SIGIO）
          其与O_ASYNC文件状态标志等效，可通过fcntl的F_SETFL命令设置or清除
         */

        auto &cur_channel = g_processes[ind].m_channel;
//
//        if (ioctl(cur_channel[0], FIOASYNC, &on) == -1) {
//
//            ERROR_LOG(g_logger) << g_processes[ind].m_channel[0]
//                                <<" ioctl(FIOASYNC) failed while spawning "
//                                <<name << "errno="<<errno
//                                <<" strerrno= "<<strerror(errno);
//            Close_channel(ind);
//            return -1;
//
//        }

        /* F_SETOWN：用于指定接收SIGIO和SIGURG信号的socket属主（进程ID或进程组ID）
          * 这里意思是指定Master进程接收SIGIO和SIGURG信号
          * SIGIO信号必须是在socket设置为信号驱动异步I/O才能产生，即上一步操作
          * SIGURG信号是在新的带外数据到达socket时产生的
         */
//        if (fcntl(cur_channel[0], F_SETOWN, Process::GetProcessId()) == -1) {
//
//            ERROR_LOG(g_logger) << cur_channel[0]
//                                <<" fcntl(F_SETOWN) failed while spawning "
//                                <<name << "errno="<<errno
//                                <<" strerrno= "<<strerror(errno);
//            Close_channel(ind);
//            return -1;
//
//        }


        /* FD_CLOEXEC：用来设置文件的close-on-exec状态标准
          *             在exec()调用后，close-on-exec标志为0的情况下，此文件不被关闭；非零则在exec()后被关闭
          *             默认close-on-exec状态为0，需要通过FD_CLOEXEC设置
          *     这里意思是当Master父进程执行了exec()调用后，关闭socket
          */
        if (fcntl(cur_channel[0], F_SETFD, FD_CLOEXEC) == -1) {

            ERROR_LOG(g_logger) << cur_channel[0]
                                <<" fcntl(FD_CLOEXEC) failed while spawning "
                                <<name << "errno="<<errno
                                <<" strerrno= "<<strerror(errno);
            Close_channel(ind);
            return -1;
        }


        /* 同上，这里意思是当Worker子进程执行了exec()调用后，关闭socket */
        if (fcntl(cur_channel[1], F_SETFD, FD_CLOEXEC) == -1) {

            ERROR_LOG(g_logger) << cur_channel[1]
                                <<" fcntl(FD_CLOEXEC) failed while spawning "
                                <<name << "errno="<<errno
                                <<" strerrno= "<<strerror(errno);
            Close_channel(ind);
            return -1;
        }

        g_master_channel = cur_channel[1];

        int pid = fork();

        Process::UpdateProcessId();

        // 注意这个必须在分流前被赋值，不能会出现g_process数组确实有效数组
        auto &cur_p = g_processes[ind];
        cur_p.m_name = name;
        cur_p.m_cb = std::bind(&MasterProcess::workCycle,this);
        cur_p.m_status = Process::State::EXEC;

        if(!pid)
        INFO_LOG(g_logger) << " fork son " << Process::GetProcessId();

        switch (pid) {
            case -1:
                ERROR_LOG(g_logger) << "fork() failed while spawning"<<name
                                    <<" errno= "<<errno<<" strerror= "
                                    <<strerror(errno);
                break;
            case 0:
                pid = Process::GetProcessId();
                // start会进入事件循环，等到退出事件循环后，需要调用stop回收资源
                cur_p.m_pid = pid;
                g_process_ind = ind;
                workCycle();
                exit(0);
                break;
            default:
                cur_p.m_pid = pid;
                break;
        }

        return ind;

    }

    void MasterProcess::workCycle() {

        initWorkCycle();
        m_iom->start();
        m_iom->stop();
    }

    void MasterProcess::handleChannel() {

        while(true){
            int fd = Get_Process().m_channel[1];
            auto msg = Channel::ReadChannel(fd);
            auto &one = g_processes[msg->m_slot];
            if(msg->m_cmd == Channel::CMD_QUIT){
//                CWJ_ASSERT(false);
                INFO_LOG(g_logger) << " recv channel Channel::CMD_QUIT";
                // TODO 优雅退出，后面需要然后前面的都停止
                m_iom->auto_stop();
                break;
            }else if(msg->m_cmd == Channel::CMD_OPEN_CHANNEL){
                INFO_LOG(g_logger) << "son process pid="<<GetProcessId()<<" open ";
                INFO_LOG(g_logger) << " recv channel Channel::CMD_OPEN_CHANNEL";
                one.m_pid = msg->m_pid;
                one.m_channel[0] = msg->m_fd;
            }else if(msg->m_cmd == Channel::CMD_CLOSE_CHANNEL){
                INFO_LOG(g_logger) << " recv channel Channel::CMD_CLOSE_CHANNEL";
                close(one.m_channel[0]);
                one.m_channel[0] = -1;
            }
            else{
                INFO_LOG(g_logger) << "son process pid="<<GetProcessId()<<" recv fail ";
            }
        }

    }

    void MasterProcess::passOpenChannel(uint32_t slot,const Channel &channel) {


        for(int i=0;i<g_processes_cur_len;i++) {
            auto &one = g_processes[i];
            INFO_LOG(g_logger) << i << " ====== " << one.m_pid<<" "<<one.m_channel[0];
            if (i == slot || one.m_pid == -1 || one.m_channel[0] == -1) continue;
            Channel::WriteChannel(one.m_channel[0], channel);
        }
    }


    static void signalHandler(int signo){

        INFO_LOG(g_logger) << " === signalHandler signo="<< signo <<" isMaster = "<<(g_process_type == ProcessType::MASTER);

        switch (g_process_type) {
            case ProcessType::MASTER:

                switch (signo) {
                    case SIGHUP:
                        g_reload = true;
                        INFO_LOG(g_logger) << "reload";
                        break;
                    case SIGUSR1:
                        INFO_LOG(g_logger) << "reload";
                        break;
                    case SIGUSR2:
                        break;
                    case SIGTERM:
                    case SIGINT:
                        g_is_term = true;
                        INFO_LOG(g_logger) << "TERM";
                        break;
                    case SIGQUIT:
                        g_is_quit = true;
                        INFO_LOG(g_logger) << "SIGQUIT";
                        break;
                    case SIGCHLD:
                        g_is_poll ++;
                        INFO_LOG(g_logger) << "SIGCHLD";
                        break;
                    default:
                        INFO_LOG(g_logger) << "master sigo:"<<signo<<" ignore ";
                }
                
                break;
            case ProcessType::WORKER:

                switch (signo) {
                    case SIGTERM:
                        g_is_term = true;
                        INFO_LOG(g_logger) << "SIGTERM";
                        break;
                    case SIGQUIT:
                        g_is_quit = true;
                        INFO_LOG(g_logger) << "SIGQUIT";
                        break;
                    default:
                        INFO_LOG(g_logger) << "worker sigo:"<<signo<<" ignore ";
                }

                break;
            default:
                ERROR_LOG(g_logger) << "signalHandler error signo="<<signo<<" ProcessType= "<<(int)g_process_type;
                break;
        }
    }

    MasterProcess::MasterProcess(size_t worker_size ,size_t worker_pre_thread_num )
                : m_worker_pre_thread_num(worker_pre_thread_num),
                  m_worker_size(worker_size),
                  m_iom(std::make_shared<IOManager>(worker_pre_thread_num,true,"iom")){

        int fd = eventfd(0,0);

        SetNonblock(fd);
        m_master_cycle = [fd] (IOManager::ptr iom) {
            INFO_LOG(g_logger)<<" m_master_cycle run";
//            int t = sleep(1);
//            uint64_t buf = 8;
//            write(fd,&buf,sizeof(buf));
        };
        m_worker_cycle = [fd] (IOManager::ptr iom){
            INFO_LOG(g_logger)<<" m_worker_cycle run pid:"<<Process::GetProcessId();
            uint64_t buf;
            read(fd,&buf,sizeof(uint64_t));
            INFO_LOG(g_logger) << "====";
            read(fd,&buf,sizeof(uint64_t));
            INFO_LOG(g_logger) << "====";
            read(fd,&buf,sizeof(uint64_t));
            INFO_LOG(g_logger) << "=== eventfd buf read = "<<buf << " === ";
        };

        // 读入配置文件，优雅退出，暴力退出，重新拉起子进进程，热部署
        m_signal_ctx_master = {
                // 读入配置文件
                {SIGHUP,"SIGHUP","reload",&signalHandler},
                // 重新打开文件描述符
                {SIGUSR1,"SIGUSER1","reopen",&signalHandler},
                //热升级通过发送该信号,这里必须保证父进程大于1，父进程小于等于1的话，说明已经由就master启动了本master，则就不能热升级
                {SIGUSR2,"SIGUSER2","re update",&signalHandler},
                // 暴力停止
                {SIGTERM,"SIGTERM","stop",&signalHandler},
                // 优雅退出
                {SIGQUIT,"SIGQUIT","quit",&signalHandler},

                //{ SIGALRM, "SIGALRM", "", &signalHandler },


                /*

                 Linx中的SIGINT信号是用于终止进程的信号，通常由终端用户通过键盘输入CTRL-C发送给运行的程序。
                SIGINT信号的作用是通知进程需要终止运行，并向其发送一个中断信号。进程可以选择捕获SIGINT信号并执行适当的处理操作，例如保存数据或关闭文件。如果进程未捕获SIGINT信号，则操作系统将终止进程的运行。
                触发SIGINT信号的场景包括：
                1. 用户在命令行中按下CTRL-C。
                2. 用户在终端会话中请求中断，例如使用SSH连接时按下CTRL-C组合键。
                进程向自身发送SIGINT信号。
                通常情况下，SIGINT信号用于正常终止未响应的进程，但也可以用于强制终止难以结束的进程。除了SIGINT信号之外，还有一些其他的信号也可以用于终止进程，例如SIGTERM和SIGKILL。
                在当前项目中其和SIGTERM的处理一样
                 * */


                { SIGINT, "SIGINT", "", &signalHandler },

                /*
                    Linx中的SIGIO信号表示异步I/O事件，通常用于非阻塞I/O操作。当进程执行非阻塞I/O操作时，内核会在数据就绪时向进程发送SIGIO信号，告诉进程可以读取或写入数据了。
                    触发SIGIO信号的场景包括：
                    1. 套接字上有数据可读或数据可写。
                    2. 文件描述符上有数据可读或数据可写。
                    3. 终端上有字符输入。
                    4. 信号管道上有数据可读取。
                    当应用程序使用异步I/O进行非阻塞操作时，可以通过捕获SIGIO信号来通知它何时可以读取或写入数据。这种技术可以提高应用程序的性能和响应性。
                */

                //{ SIGIO, "SIGIO", "", &signalHandler },

                /*
                子进程终止, 这时候内核同时向父进程发送个sigchld信号.等待父进程waitpid回收，避免僵死进程
                */

                { SIGCHLD, "SIGCHLD", "", &signalHandler },

                /*
                    Linx中的SIGSYS信号是用于系统调用出错的信号。当进程执行系统调用时，如果发生错误，内核会向进程发送SIGSYS信号。
                    触发SIGSYS信号的场景包括：
                    1. 进程试图调用一个不存在的系统调用。
                    2. 进程试图调用一个未被启用或被禁用的系统调用。
                    3. 进程没有足够的权限执行系统调用。
                    如果进程捕获了SIGSYS信号，则可以采取适当的处理措施，例如输出一条错误消息或者结束进程。在一些安全相关的场合，可以使用SIGSYS信号来检测和预防攻击，例如防止缓冲区溢出和恶意代码注入。
                    需要注意的是，有些程序会使用未公开的系统调用，例如调试器和一些安全软件，这些程序也可能会触发SIGSYS信号。因此，在捕获SIGSYS信号之前，必须先检查系统调用的合法性。
                 * */

                {SIGSYS,"SIGSYS,SIG_IGH","",SIG_IGN},

                /*
                 *
                 * 如果向已经关闭的管道中写入数据，那么就会触发该信号
                 *
                 * */

                { SIGPIPE, "SIGPIPE, SIG_IGN", "", SIG_IGN },


        };


    }


    void MasterProcess::initWorkCycle() {
        // TODO 把CPU和进程绑定  等
        // 初始化

        g_process_type = ProcessType::WORKER;

        g_is_quit = false;
        g_is_term = false;
        g_is_update = false;
        g_is_poll = 0;
        g_reload = false;

        m_iom = std::make_shared<IOManager>(m_worker_pre_thread_num,true,m_worker_name+"_"+std::to_string(g_process_ind));

        SetProcessName(m_worker_name+"_"+std::to_string(g_process_ind));

        // 处理工作进程的频道机制

        for(int i=0;i<g_processes_cur_len;i++){
            if(i == g_process_ind)  continue;
            auto &one = g_processes[i];
            if(one.m_pid == -1 || one.m_channel[1] == -1 )  continue;

            if(close(one.m_channel[1]) == -1){
                ERROR_LOG(g_logger) << " close channel fd= "<<one.m_channel[1]
                                    << " fail errno = "<<errno
                                    <<" strerror= "<<strerror(errno);
            }
        }

        if(close(Get_Process().m_channel[0]) == -1){
            ERROR_LOG(g_logger) << " close channel fd= "<<Get_Process().m_channel[0]
                                << " fail errno = "<<errno
                                <<" strerror= "<<strerror(errno);
        }

        m_iom->schedule(std::bind(&MasterProcess::handleChannel,this),-1);
        m_iom->schedule(std::bind(&MasterProcess::midWorkerCycle,this),-1);
    }

    void MasterProcess::setMWorkerCycle(const MasterProcess::CalllBack &mWorkerCycle) {
        m_worker_cycle = mWorkerCycle;
    }

    void MasterProcess::setMMasterCycle(const MasterProcess::CalllBack &mMasterCycle) {
        m_master_cycle = mMasterCycle;
    }

    int  MasterProcess::reclaimChildren(int pid) {
        int ind = -1,slot = -1;
        for(int i=0;i<g_processes_cur_len;i++){
            auto &one = g_processes[i];
            if(one.m_pid == pid){
                one.m_pid = -1;
                Close_channel(i);
                one.m_status = Process::State::INIT;
                slot = i;
            }
        }
        if(ind > -1) {
            Channel chl = {Channel::Command::CMD_CLOSE_CHANNEL, pid, (uint32_t)ind,-1};
            passOpenChannel(ind,chl);
        }

        return slot;

    }

    void MasterProcess::signalWorkerProcesses(int signo) {

        for(int i=0;i<g_processes_cur_len;i++){

            auto &one = g_processes[i];
            if( one.m_pid == -1 )   continue;
            if (kill(one.m_pid, signo) == -1) { //关闭旧的进程；
                ERROR_LOG(g_logger) << "kill(%P, %d) failed", one.m_pid ;
                continue;
            }

        }

    }

    void MasterProcess::midWorkerCycle() {
//        CWJ_ASSERT(false);
        while(true){
            if(g_is_quit){
                m_iom->auto_stop();
                CWJ_ASSERT(false);
                break;
            }
            if(g_is_term){
                m_iom->auto_stop();
                CWJ_ASSERT(false);
                break;
            }
            m_worker_cycle(m_iom);
        }
        CWJ_ASSERT(false);
    }


}