#include "XRegisterServer.h"
#include "XRegisterHandle.h"

#include <XThreadPool.h>

void XRegisterServer::main(int argc, char *argv[])
{
    XRegisterHandle::regMsgCallback();

    int port = REGISTER_PORT;
    if (argc > 1)
        port = atoi(argv[1]);

    /// 设置服务器监听端口
    this->setServerPort(port);
}

void XRegisterServer::wait()
{
    XThreadPool::wait();
}

XServiceHandle *XRegisterServer::createHandle()
{
    auto handle = new XRegisterHandle();
    handle->setReadTime(5000);

    return handle;
}
