#include "XRegisterServer.h"
#include "XRegisterHandle.h"

#include <XThreadPool.h>

auto XRegisterServer::main(int argc, char *argv[]) -> void
{
    XRegisterHandle::regMsgCallback();

    int port = REGISTER_PORT;
    if (argc > 1)
        port = atoi(argv[1]);

    /// 设置服务器监听端口
    this->setServerPort(port);
}

auto XRegisterServer::createHandle() -> XServiceHandle *
{
    auto handle = new XRegisterHandle();
    handle->setReadTimeMs(5000);

    return handle;
}
