#include "XMSFileManager.h"

#include "XGetDirClient.h"
#include "XUploadClient.h"
#include "XDownloadClient.h"

#include <XTools.h>
#include <XDiskCom.pb.h>

#include <thread>
#include <fstream>

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
    // auto login = getLogin();
    XGetDirClient::regMsgCallback();
    XUploadClient::regMsgCallback();
    XDownloadClient::regMsgCallback();

#ifdef GATE_WAY_FORWARD
    XGetDirClient::get()->setServerIP(server_ip.c_str());
    XGetDirClient::get()->setServerPort(server_port);

#else
    XGetDirClient::get()->setServerIP("127.0.0.1");
    XGetDirClient::get()->setServerPort(DIR_PORT);
#endif

    XGetDirClient::get()->startConnect();
    // XGetDirClient::get()->setLogin(&login);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

auto XMSFileManager::getDir(const std::string &root) -> void
{
    this->setRoot(root);
    // auto login = this->getLogin();

    xdisk::XGetDirReq req;
    req.set_root(root);
    // XGetDirClient::get()->setLogin(&login);
    XGetDirClient::get()->getDirReq(req);
}

auto XMSFileManager::newDir(const std::string &path) -> void
{
    XGetDirClient::get()->newDirReq(path);
}

auto XMSFileManager::deleteFile(xdisk::XFileInfo file) -> void
{
    XGetDirClient::get()->deleteFileReq(file);
}

auto XMSFileManager::uploadFile(xdisk::XFileInfo file) -> void
{
    auto ip   = std::string("127.0.0.1");
    auto port = UPLOAD_PORT;
    std::cout << file.DebugString() << std::endl;

    auto servers = upload_servers();
    if (!servers.services().empty())
    {
        static int index = 0;
        /// 轮询使用服务器
        index = index % servers.services().size();
        ip    = servers.services().at(index).ip();
        port  = servers.services().at(index).port();
        index++;
    }
    std::stringstream ss;
    ss << "upload server " << ip << ":" << port;
    LOGINFO(ss.str());


    std::ifstream ifs(file.local_path(), std::ios::ate);
    if (!ifs)
    {
        std::cout << "UploadFile failed!" << file.local_path() << std::endl;
        return;
    }
    long long filesize = ifs.tellg();
    ifs.close();
    file.set_filesize(filesize);

    auto pass = password();
    if (!pass.empty())
    {
        file.set_is_enc(true);
        file.set_password(pass);
    }
    std::cout << file.DebugString();

    auto login  = getLogin();
    auto client = new XUploadClient();
    /// 不用自动重连，失败就关闭重新开始
    client->setAutoConnect(false);
    client->setAutoDelete(false);
    client->setServerIP(ip.c_str());
    client->setServerPort(port);
    client->setLogin(&login);
    if (!client->setFile(file))
    {
        std::cout << "client->LoadFile failed!" << std::endl;
        /// 返回一个错误消息
        delete client;
        return;
    }
    client->startConnect();

    //////////////////////////////////////////////////////////////////
    int task_id = this->addUploadTask(file);
    client->set_task_id(task_id);
}

auto XMSFileManager::downloadFile(xdisk::XFileInfo file) -> void
{
    auto ip   = std::string("127.0.0.1");
    int  port = DOWNLOAD_PORT;
    std::cout << file.DebugString() << std::endl;


    auto servers = download_servers();
    if (!servers.services().empty())
    {
        static int index = 0;
        /// 轮询使用服务器
        index = index % servers.services().size();
        ip    = servers.services().at(index).ip();
        port  = servers.services().at(index).port();
        index++;
    }
    std::stringstream ss;
    ss << "download server " << ip << ":" << port;
    LOGINFO(ss.str());

    auto client = new XDownloadClient();
    auto login  = getLogin();

    /// 不用自动重连，失败就关闭重新开始
    client->setAutoConnect(false);
    client->setAutoDelete(false);
    client->setServerIP(ip.c_str());
    client->setServerPort(port);
    client->setFile(file);
    client->startConnect();

    client->setLogin(&login);

    //这时候不知道文件大小
}

auto XMSFileManager::setLogin(const xmsg::XLoginRes &login) -> void
{
    XGetDirClient::get()->setLogin(&login);
    XFileManager::setLogin(login);
}
