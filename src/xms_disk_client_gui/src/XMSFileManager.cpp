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
    auto login = getLogin();
    XGetDirClient::regMsgCallback();


#ifdef GATE_WAY_FORWARD
    XGetDirClient::get()->setServerIP(server_ip.c_str());
    XGetDirClient::get()->setServerPort(server_port);

#else
    XGetDirClient::get()->setServerIP("127.0.0.1");
    XGetDirClient::get()->setServerPort(DIR_PORT);
#endif

    XGetDirClient::get()->startConnect();
    XGetDirClient::get()->setLogin(&login);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

auto XMSFileManager::getDir(const std::string &root) -> void
{
    this->setRoot(root);
    auto login = this->getLogin();

    xdisk::XGetDirReq req;
    req.set_root(root);
    XGetDirClient::get()->setLogin(&login);
    XGetDirClient::get()->getDirReq(req);
}

auto XMSFileManager::newDir(const std::string &path) -> void
{
    XGetDirClient::get()->newDirReq(path);
}

auto XMSFileManager::deleteFile(const xdisk::XFileInfo &file) -> void
{
    XGetDirClient::get()->deleteFileReq(file);
}
