#include "XUploadService.h"
#include "XUploadHandle.h"

#include <XTools.h>

XUploadService::XUploadService() = default;

XUploadService::~XUploadService() = default;

auto XUploadService::main(int argc, char *argv[]) -> void
{
    XUploadHandle::regMsgCallback();
    LOGDEBUG("xms_upload_service register_ip register_port  service_port ");

    int service_port = UPLOAD_PORT;
    setServerPort(service_port);
}

auto XUploadService::createHandle() -> XServiceHandle *
{
    return new XUploadHandle;
}
