#include "XAuthDao.h"
#include "XAuthServer.h"
#include "XAuthHandle.h"

#include <XTools.h>
#include <XRegisterClient.h>

#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    std::cout << "=================Auth Server====================" << std::endl;

    int  server_port = AUTH_PORT;
    auto ip          = XTools::XGetHostByName(REGISTER_SERVER_NAME);
    RegisterClient->setServerIP(ip.c_str());
    RegisterClient->setServerPort(REGISTER_PORT);
    RegisterClient->registerServer(AUTH_NAME, server_port, 0);

    XAuthDao::get()->init();
    XAuthDao::get()->install();

    XAuthHandle::regMsgCallback();

    XAuthServer service;
    service.setServerPort(server_port);
    service.start();

    XAuthServer::wait();
    getchar();
    return 0;
}
