#include "XDownloadServer.h"

#include "XDownloadHandle.h"

#include <XTools.h>
#include <XRegisterClient.h>

class XDownloadServer::PImpl
{
public:
    PImpl(XDownloadServer *owenr);
    ~PImpl() = default;

public:
    XDownloadServer *owenr_ = nullptr;
};

XDownloadServer::PImpl::PImpl(XDownloadServer *owenr) : owenr_(owenr)
{
}

XDownloadServer::XDownloadServer()
{
    impl_ = std::make_unique<XDownloadServer::PImpl>(this);
}

XDownloadServer::~XDownloadServer()
{
}

auto XDownloadServer::createHandle() -> XServiceHandle *
{
    return new XDownloadHandle;
}

auto XDownloadServer::main(int argc, char *argv[]) -> void
{
    XDownloadHandle::regMsgCallback();
    LOGDEBUG("xms_upload_service register_ip register_port  service_port ");

    int service_port  = DOWNLOAD_PORT;
    int register_port = REGISTER_PORT;
    //std::string register_ip = "127.0.0.1";
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


    // XRegisterClient::get()->setServerIP(register_ip.c_str());
    // XRegisterClient::get()->setServerPort(register_port);
    // XRegisterClient::get()->registerServer(UPLOAD_NAME, service_port, 0);

    XRegisterClient::get()->setServerIP(register_ip.c_str());
    XRegisterClient::get()->setServerPort(register_port);
    XRegisterClient::get()->registerServer(DOWNLOAD_NAME, service_port, 0, true);


    auto log = XLogClient::get();
    log->setServiceName(DOWNLOAD_NAME);
    std::string       logfile = DOWNLOAD_NAME;
    std::stringstream ss;
    ss << DOWNLOAD_NAME << "_" << service_port << ".log";
    log->setOutFile(ss.str());
    //log->StartLog();
}
