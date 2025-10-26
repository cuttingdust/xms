#include "XLogServer.h"

#include "XLogHandle.h"

#include <XRegisterClient.h>
#include <XTools.h>

XLogServer::XLogServer()
{
}

XLogServer::~XLogServer()
{
}

auto XLogServer::main(int argc, char *argv[]) -> void
{
    XLogClient::get()->setPrint(false);
    std::cout << "xlog register_ip  register_port service_port" << std::endl;
    ///注册回调函数
    XLogHandle::regMsgCallback();
    int service_port  = XLOG_PORT;
    int register_port = REGISTER_PORT;
    //string register_ip = "127.0.0.1";
    std::string register_ip = XTools::XGetHostByName(API_REGISTER_SERVER_NAME);
    if (argc > 1)
    {
        register_ip = argv[1];
    }
    if (argc > 2)
    {
        register_port = atoi(argv[2]);
    }
    if (argc > 3)
    {
        service_port = atoi(argv[3]);
    }

    /// 设置服务器监听端口
    setServerPort(service_port);


    XRegisterClient::get()->setServerIP(register_ip.c_str());
    XRegisterClient::get()->setServerPort(register_port);

    /// 向注册中心注册
    XRegisterClient::get()->registerServer(CONFIG_NAME, service_port, 0);

    XLogClient::get()->setServerIP("127.0.0.1");
    XLogClient::get()->setServerPort(service_port);
    XLogClient::get()->setServiceName(XLOG_NAME);
    XLogClient::get()->setServerPort(service_port);
    XLogClient::get()->setLogLevel(xmsg::XLOG_INFO);


    XLogClient::get()->startLog();
}

auto XLogServer::createHandle() -> XServiceHandle *
{
    return new XLogHandle;
}
