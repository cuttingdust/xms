#include "XUploadService.h"

#include "XUploadHandle.h"

#include <XTools.h>
#include <XRegisterClient.h>


XUploadService::XUploadService() = default;

XUploadService::~XUploadService() = default;

auto XUploadService::main(int argc, char *argv[]) -> void
{
    XUploadHandle::regMsgCallback();
    LOGDEBUG("xms_upload_service register_ip register_port  service_port ");

    int service_port  = UPLOAD_PORT;
    int register_port = REGISTER_PORT;
    // std::string register_ip   = "127.0.0.1";

    std::string register_ip = XTools::XGetHostByName(API_REGISTER_SERVER_NAME);
    if (argc > 1)
    {
        register_ip = argv[1];
    }
    if (argc > 2)
    {
        register_port = atoi(argv[2]);
    }
    if (argc > 3)
    {
        service_port = atoi(argv[1]);
    }

    this->setServerPort(service_port);

    /// 注册到注册中心
    // XRegisterClient::get()->registerServer(UPLOAD_NAME, service_port, 0, true);

    XRegisterClient::get()->setServerIP(register_ip.c_str());
    XRegisterClient::get()->setServerPort(register_port);
    XRegisterClient::get()->registerServer(UPLOAD_NAME, service_port, 0, true);
    auto log = XLogClient::get();
    log->setServiceName(UPLOAD_NAME);

    log->setPrint(true);
    std::string       logfile = UPLOAD_NAME;
    std::stringstream ss;
    ss << UPLOAD_NAME << "_" << service_port << ".log";
    log->setOutFile(ss.str());
    //log->StartLog();
}

auto XUploadService::createHandle() -> XServiceHandle *
{
    return new XUploadHandle;
}
