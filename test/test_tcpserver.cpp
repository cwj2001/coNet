//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <signal.h>

#include "util.h"
#include "tcpserver.h"
#include "config.h"
#include "hook.h"
#include "byteArray.h"
#include "log.h"

using namespace std;
using namespace CWJ_CO_NET;
static auto g_logger = GET_ROOT_LOGGER();
class TestServer : public TcpServer {
public:
    using ptr = std::shared_ptr<TestServer>;

    TestServer(const string &mName, size_t acceptThreadCount, size_t ioThreadCount, bool acceptShared) : TcpServer(
            mName, acceptThreadCount, ioThreadCount, acceptShared) {}

    void handleClient(Socket::ptr sock) override {
        INFO_LOG(GET_ROOT_LOGGER()) << "handleClient(Socket::ptr sock)";
        static char buf[10];
        while (sock->isConnect() && sock->recv(buf, sizeof(buf), 0) > 0) {
            string str(buf);
            if (str != "\r\n") {
                str = "server:" + str;
                sock->send(str.c_str(), str.size(), 0);
            }
            memset(buf, 0, sizeof(buf));
        }
        sock->close();
    }

private:

};

class MoreTestServer: public MoreTcpServer{
public:
    using ptr = std::shared_ptr<MoreTestServer>;

    MoreTestServer(const string &mName, size_t acceptThreadCount, size_t ioThreadCount, bool acceptShared)
            : MoreTcpServer(mName, acceptThreadCount, ioThreadCount, acceptShared) {
    }

    void onConnection(Socket::ptr& sock) override {
        INFO_LOG(g_logger) << "onConnection";
        m_num++;
        ERROR_LOG(g_logger) << *sock;
        ERROR_LOG(g_logger) << "m_num="<<m_num;
    }

    void onClose(Socket::ptr& sock) override {
        INFO_LOG(g_logger) << "onClose";
        m_num--;
    }

    void onWriteComplete(Socket::ptr& sock) override {
    }

    void onMessage(Socket::ptr& sock,ByteArray::ptr &recv_buffer, ByteArray::ptr &send_buffer) override {
//        INFO_LOG(g_logger) <<"onMessage: "<<recv_buffer->getMDataSize()<<" "<<send_buffer->getMDataSize();
        if(!sock->isConnect())  return ;
        const string str = recv_buffer->read(recv_buffer->getMDataSize());

        if(str.size() && str != "\r\n")  send_buffer->write("server:"+str);
//        ERROR_LOG(g_logger) <<"send_buffer.size="<<send_buffer->getMDataSize()<<"   "<< str ;
    }

private:
    size_t m_num = 0;
};

int main() {

    signal (SIGPIPE, SIG_IGN);

//    ConfigManager::loadYamlFromDir("/home/cwj2001/cwj/myCppProject/config");
    TcpServer::ptr server(new MoreTestServer("server", 1, 1, false));
    server->bind(IPv4Address::Create("0.0.0.0", 8035));
    server->start();
    return 0;
}

