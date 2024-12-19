#include "XRouteServer.h"
#include "XServiceProxy.h"

#include <XRegisterClient.h>
#include <XThreadPool.h>

#include <iostream>

int main(int argc, char *argv[])
{
    /// xms_gateway
    std::cout << "xms_gateway API_GATEWAY_PORT REGISTER_IP REGISTER_PORT" << std::endl;
    int server_port = API_GATEWAY_PORT;
    if (argc > 1)
        server_port = atoi(argv[1]);
    std::cout << "server port is " << server_port << std::endl;
    std::string register_ip = "127.0.0.1";
    if (argc > 2)
        register_ip = argv[2];
    int register_port = REGISTER_PORT;
    if (argc > 3)
        register_port = atoi(argv[3]);

    /// 设置注册中心的IP和端口
    XRegisterClient::get()->setServerIp(register_ip.c_str());
    XRegisterClient::get()->setServerPort(register_port);

    /// 注册到注册中心
    XRegisterClient::get()->registerServer(API_GATEWAY_NAME, server_port, nullptr);

    /// 等待注册中心连接
    XRegisterClient::get()->waitConnected(3);
    XRegisterClient::get()->getServiceReq(nullptr);

    XServiceProxy::get()->init();

    /// 开启自动重连
    XServiceProxy::get()->start();

    XRouteServer service;
    service.setServerPort(server_port);
    service.start();
    XThreadPool::wait();
    return 0;
}
