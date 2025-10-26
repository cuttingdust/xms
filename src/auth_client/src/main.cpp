#include "XAuthClient.h"

#include <thread>
#include <iostream>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh_CN.UTF-8");

    //////////////////////////////////////////////////////////////////
    std::cout << "XAuth Client!\n";
    XAuthClient::regMsgCallback();

    XAuthClient::get()->setServerIP("127.0.0.1");
    XAuthClient::get()->setServerPort(AUTH_PORT);
    XAuthClient::get()->startConnect();
    XAuthClient::get()->waitConnected(3);
    xmsg::XAddUserReq adduser;
    adduser.set_username("guest");
    adduser.set_password("12345678");
    adduser.set_rolename("guest");
    XAuthClient::get()->addUserReq(&adduser);

    XAuthClient::get()->login("guest", "12345678");
    XAuthClient::get()->wait();

    return 0;
}
