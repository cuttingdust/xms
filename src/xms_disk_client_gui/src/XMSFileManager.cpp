#include "XMSFileManager.h"

#include "XDiskCom.pb.h"
#include "XGetDirClient.h"

#include <thread>

class XMSFileManager::PImpl
{
public:
    PImpl(XMSFileManager *owenr);
    ~PImpl() = default;

public:
    XMSFileManager *owenr_ = nullptr;
};

XMSFileManager::PImpl::PImpl(XMSFileManager *owenr) : owenr_(owenr)
{
    owenr_->setParent(owenr_);
}

XMSFileManager::XMSFileManager()
{
    impl_ = std::make_unique<XMSFileManager::PImpl>(this);
}

XMSFileManager::~XMSFileManager() = default;

#define GATE_WAY_FORWARD

auto XMSFileManager::initFileManager(std::string server_ip, int server_port) -> void
{
    XGetDirClient::regMsgCallback();

#ifdef GATE_WAY_FORWARD
    XGetDirClient::get()->setServerIP(server_ip.c_str());
    XGetDirClient::get()->setServerPort(server_port);

#else
    XGetDirClient::get()->setServerIP("127.0.0.1");
    XGetDirClient::get()->setServerPort(DIR_PORT);
#endif

    XGetDirClient::get()->startConnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

auto XMSFileManager::getDir(const std::string &root) -> void
{
    this->setRoot(root);

    xdisk::XGetDirReq req;
    req.set_root(root);
    XGetDirClient::get()->getDirReq(req);
}
