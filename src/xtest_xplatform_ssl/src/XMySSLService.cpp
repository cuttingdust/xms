#include "XMySSLService.h"
#include "XMySSLServiceHandle.h"

XMySSLService::XMySSLService() = default;

XMySSLService::~XMySSLService() = default;

XServiceHandle *XMySSLService::createHandle()
{
    return new XMySSLServiceHandle();
}
