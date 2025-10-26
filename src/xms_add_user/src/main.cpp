#include <XAuthClient.h>

#include <iostream>
#include <thread>

int main(int argc, char *argv[])
{
    std::string username = "";
    std::string rolename = "";
    std::string password = "";
    std::cout << "Username:";
    std::cin >> username;
    std::cout << "Rolename:";
    std::cin >> rolename;
    std::cout << "Password:";
    std::cin >> password;
    std::cout << username << "/" << password << std::endl;
    XAuthClient::regMsgCallback();
    XAuthClient::get()->setServerIP("127.0.0.1");
    XAuthClient::get()->setServerPort(AUTH_PORT);
    XAuthClient::get()->startConnect();
    while (!XAuthClient::get()->isConnected())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    xmsg::XAddUserReq adduser;
    adduser.set_username(username);
    adduser.set_password(password);
    adduser.set_rolename(rolename);
    XAuthClient::get()->addUserReq(&adduser);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 0;
}
