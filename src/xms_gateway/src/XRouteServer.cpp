#include "XRouteServer.h"

#include "XRouteHandle.h"

XRouteServer::XRouteServer() = default;

XRouteServer::~XRouteServer() = default;

XServiceHandle *XRouteServer::createHandle()
{
    return new XRouteHandle();
}
