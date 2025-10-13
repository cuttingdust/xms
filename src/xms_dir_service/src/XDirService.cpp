#include "XDirService.h"
#include "XDirHandle.h"

XDirService::XDirService()
{
}

XDirService::~XDirService() = default;

auto XDirService::createHandle() -> XServiceHandle *
{
    return new XDirHandle;
}
