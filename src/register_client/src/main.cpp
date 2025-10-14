#include <XRegisterClient.h>

#include <XThreadPool.h>
#include <XTools.h>

#include <iostream>
#include <thread>

int main(int argc, char *argv[])
{
    std::cout << "Register Client " << std::endl;
    std::cout << "register_client 127.0.0.1 20018 " << std::endl;
    /// 注册中心IP 端口
    ///
    std::string ip   = "127.0.0.1";
    int         port = REGISTER_PORT;
    if (argc > 2)
    {
        ip   = argv[1];
        port = atoi(argv[2]);
    }

    XRegisterClient::get()->setServerIP(ip.c_str());
    XRegisterClient::get()->setServerPort(port);
    XRegisterClient::get()->registerServer("test", 20020, 0);
    XRegisterClient::get()->waitConnected(3);

    /// 发送获取全部服务的请求
    XRegisterClient::get()->getServiceReq(nullptr);
    XRegisterClient::get()->getServiceReq("test");
    for (;;)
    {
        XRegisterClient::get()->getServiceReq(nullptr);
        if (const auto services = XRegisterClient::get()->getAllService())
        {
            LOGDEBUG(services->DebugString());
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }


    XThreadPool::wait();
    return 0;
}
