#include "XDirServiceHandle.h"


#include <XRegisterClient.h>
#include <XService.h>
#include <XThreadPool.h>
#include <XTools.h>

class XTestService : public XService
{
public:
    XServiceHandle *createHandle() override
    {
        return new XDirServiceHandle();
    }
};


int main(int argc, char *argv[])
{
    std::cout << "test_pb_service SERVER_PORT REGISTER_IP REGISTER_PORT" << std::endl;
    int server_port = 20011;
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
    XRegisterClient::get()->registerServer("dir", server_port, nullptr);

    XDirServiceHandle::regMsgCallback();

    XTestService service;
    service.setServerPort(server_port);
    service.start();
    XThreadPool::wait();

    return 0;
}
