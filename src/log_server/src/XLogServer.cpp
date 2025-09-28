#include "XLogServer.h"
#include "XLogHandle.h"

XLogServer::XLogServer()
{
}

XLogServer::~XLogServer()
{
}

auto XLogServer::createHandle() -> XServiceHandle *
{
    return new XLogHandle;
}
