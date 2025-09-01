#include "XAuthClient.h"
#include "XThreadPool.h"

#include <thread>
#include <iostream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    //////////////////////////////////////////////////////////////////
    std::cout << "XAuth Client!\n";

    XAuthClient client;
    client.setServerIp("127.0.0.1");
    client.setServerPort(AUTH_PORT);
    client.startConnect();
    client.waitConnected(3);
    client.LoginReq("root", "123456");
    XThreadPool::wait();

    return 0;
}
