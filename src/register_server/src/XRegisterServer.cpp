#include "XRegisterServer.h"

#include "XRegisterHandle.h"

#include <XLogClient.h>
#include <XThreadPool.h>

auto XRegisterServer::main(int argc, char *argv[]) -> void
{
    XRegisterHandle::regMsgCallback();

    int port = REGISTER_PORT;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    /// 需要考虑从注册中心自己获取日志模块
    XLogClient::get()->setServiceName(REGISTER_NAME);
    XLogClient::get()->setServerPort(XLOG_PORT);
    XLogClient::get()->setAutoConnect(true);
    XLogClient::get()->startLog();

    /// 设置服务器监听端口
    this->setServerPort(port);
}

auto XRegisterServer::createHandle() -> XServiceHandle *
{
    auto handle = new XRegisterHandle();
    handle->setReadTimeMs(5000);

    return handle;
}
