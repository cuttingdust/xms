#include "XAuthClient.h"

#include <thread>
#include <iostream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    //////////////////////////////////////////////////////////////////
    std::cout << "XAuth Client!\n";
    XAuthClient::regMsgCallback();

    XAuthClient client;
    client.setServerIP("127.0.0.1");
    client.setServerPort(AUTH_PORT);
    client.startConnect();
    client.waitConnected(3);
    xmsg::XAddUserReq adduser;
    adduser.set_username("guest");
    adduser.set_password("12345678");
    adduser.set_rolename("guest");
    client.addUserReq(&adduser);

    client.loginReq("guest", "12345678");
    client.wait();

    return 0;
}
