#include "XRouteServer.h"
#include "XServiceProxy.h"

#include <XThreadPool.h>

#include <iostream>

int main(int argc, char *argv[])
{
    int server_port = API_GATEWAY_PORT;
    std::cout << "API GATEWAY -- server port is " << server_port << std::endl;

    XServiceProxy::get()->init();

    /// 开启自动重连
    XServiceProxy::get()->start();

    XRouteServer service;
    service.setServerPort(server_port);
    service.start();
    XThreadPool::wait();
    return 0;
}
