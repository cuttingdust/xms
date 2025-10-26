#include "XAuthDao.h"
#include "XAuthServer.h"
#include "XAuthHandle.h"

#include <XTools.h>
#include <XRegisterClient.h>
#include <XConfigAndRegister.h>

#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    std::cout << "=================Auth Server====================" << std::endl;
    std::cout << "xauth_server SERVER_PORT REGISTER_IP REGISTER_PORT" << std::endl;
    std::cout << "xauth_server install" << std::endl;
    if (!XAuthDao::get()->init())
    {
        std::cout << "DB init failed!" << std::endl;
        return -1;
    }
    if (!XAuthDao::get()->install())
    {
        std::cout << "DB create table failed!" << std::endl;
        return -2;
    }
    XAuthHandle::regMsgCallback();


    int server_port = AUTH_PORT;
    if (argc > 1)
    {
        server_port = atoi(argv[1]);
    }
    std::cout << "server port is " << server_port << std::endl;

    auto register_ip = XTools::XGetHostByName(API_REGISTER_SERVER_NAME);
    if (argc > 2)
    {
        register_ip = argv[2];
    }

    if (register_ip.empty())
    {
        register_ip = "127.0.0.1";
    }

    int register_port = REGISTER_PORT;
    if (argc > 3)
    {
        register_port = atoi(argv[3]);
    }


    static xmsg::XAuthConfig config;
    XConfigAndRegister::init(AUTH_NAME, 0, server_port, register_ip.c_str(), register_port, &config);


    XAuthServer service;
    service.setServerPort(server_port);
    service.start();

    XAuthServer::wait();
    return 0;
}
