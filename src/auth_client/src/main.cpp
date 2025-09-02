#include "XAuthClient.h"
#include "XThreadPool.h"

#include <thread>
#include <iostream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    //////////////////////////////////////////////////////////////////
    std::cout << "XAuth Client!\n";
    XAuthClient::regMsgCallback();

    XAuthClient client;
    client.setServerIp("127.0.0.1");
    client.setServerPort(AUTH_PORT);
    client.startConnect();
    client.waitConnected(3);
    xmsg::XAddUserReq adduser;
    adduser.set_username("root");
    adduser.set_password("123456");
    adduser.set_rolename("root");
    client.addUserReq(&adduser);

    client.loginReq("root", "123456");
    XThreadPool::wait();

    return 0;
}
