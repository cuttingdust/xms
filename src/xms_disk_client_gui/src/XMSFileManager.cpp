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
}

XMSFileManager::XMSFileManager()
{
    impl_ = std::make_unique<XMSFileManager::PImpl>(this);
    XFileManager::setParent(this);

    XGetDirClient::regMsgCallback();
    XGetDirClient::get()->setServerIp("127.0.0.1");
    XGetDirClient::get()->setServerPort(DIR_PORT);
    XGetDirClient::get()->startConnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

XMSFileManager::~XMSFileManager() = default;

auto XMSFileManager::getDir(const std::string &root) -> void
{
    this->setRoot(root);

    xdisk::XGetDirReq req;
    req.set_root(root);
    XGetDirClient::get()->getDirReq(req);
}
