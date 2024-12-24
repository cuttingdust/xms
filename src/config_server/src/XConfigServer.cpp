#include "XConfigServer.h"

#include "XConfigHandle.h"

#include <XTools.h>
#include <XThreadPool.h>
#include <XRegisterClient.h>

XConfigServer::XConfigServer()
{
}

XConfigServer::~XConfigServer()
{
}

void XConfigServer::main(int argc, char *argv[])
{
    ///注册中心的配置
    LOGDEBUG("config_server register_ip register_port  service_port ");

    ///注册回调函数
    XConfigHandle::regMsgCallback();
    int         service_port  = CONFIG_PORT;
    int         register_port = REGISTER_PORT;
    std::string register_ip   = "127.0.0.1";
    if (argc > 1)
        register_ip = argv[1];
    if (argc > 2)
        register_port = atoi(argv[2]);
    if (argc > 3)
        service_port = atoi(argv[1]);

    /// 设置服务器监听端口
    this->setServerPort(service_port);

    /// 向注册中心注册
    XRegisterClient::get()->registerServer(CONFIG_NAME, service_port, 0);
}

XServiceHandle *XConfigServer::createHandle()
{
    return new XConfigHandle();
}

void XConfigServer::wait()
{
    XThreadPool::wait();
}
