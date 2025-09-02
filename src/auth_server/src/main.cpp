#include "XAuthDao.h"
#include "XAuthServer.h"
#include "XAuthHandle.h"

#include <XRegisterClient.h>

#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    std::cout << "Auth Server" << std::endl;

    int server_port = AUTH_PORT;
    RegisterClient->setServerIp("127.0.0.1");
    RegisterClient->setServerPort(REGISTER_PORT);
    RegisterClient->registerServer(AUTH_NAME, server_port, 0);
    XAuthDao::get()->init();
    XAuthDao::get()->install();

    XAuthHandle::regMsgCallback();

    XAuthServer service;
    service.setServerPort(server_port);
    service.start();

    XAuthServer::wait();
    getchar(); ///  等待输入，防止程序直接退出
    return 0;
}
