#include "XRegisterServer.h"
#include "XRegisterHandle.h"

#include <XThreadPool.h>

void XRegisterServer::main(int argc, char *argv[])
{
    XRegisterHandle::regMsgCallback();

    int port = REGISTER_PORT;
    if (argc > 1)
        port = atoi(argv[1]);

    /// ���÷����������˿�
    this->setServerPort(port);
}

void XRegisterServer::wait()
{
    XThreadPool::wait();
}

XServiceHandle *XRegisterServer::createHandle()
{
    return new XRegisterHandle();
}
