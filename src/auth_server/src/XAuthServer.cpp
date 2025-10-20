#include "XAuthServer.h"

#include "XAuthHandle.h"

#include <XThreadPool.h>


XAuthServer::XAuthServer() = default;

XAuthServer::~XAuthServer() = default;

auto XAuthServer::createHandle() -> XServiceHandle *
{
    return new XAuthHandle;
}
