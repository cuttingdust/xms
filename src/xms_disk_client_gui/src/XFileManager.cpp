#include "XFileManager.h"

#include <XMsgCom.pb.h>
#include <XDiskCom.pb.h>
#include <XTools.h>


class XFileManager::PImpl
{
public:
    PImpl(XFileManager *owenr);
    ~PImpl() = default;

public:
    XFileManager        *owenr_ = nullptr;
    std::string          root_  = "";
    static XFileManager *instance_;
    xmsg::XLoginRes      login_;

    /// 上传的服务器列表
    std::mutex         servers_mutex_;
    xmsg::XServiceList upload_servers_;
    xmsg::XServiceList download_servers_;

    std::string                 password_ = ""; ///< 文件密钥
    std::mutex                  uploads_mutex_;
    std::mutex                  downloads_mutex_;
    std::list<xdisk::XFileTask> uploads_;
    std::list<xdisk::XFileTask> downloads_;
};
/// 静态成员的定义和初始化
XFileManager *XFileManager::PImpl::instance_ = nullptr;

XFileManager::PImpl::PImpl(XFileManager *owenr) : owenr_(owenr)
{
}

XFileManager *XFileManager::Instance()
{
    return PImpl::instance_;
}

XFileManager::XFileManager()
{
    impl_ = std::make_unique<XFileManager::PImpl>(this);
}

XFileManager::~XFileManager() = default;

auto XFileManager::setParent(XFileManager *parent) -> void
{
    PImpl::instance_ = parent;
}

auto XFileManager::setRoot(const std::string &root) -> void
{
    impl_->root_ = root;
}

auto XFileManager::refreshDir() -> void
{
    getDir(impl_->root_);
}

auto XFileManager::addUploadTask(const xdisk::XFileInfo &file) -> int
{
    xdisk::XFileTask task;
    const auto       file_task = new xdisk::XFileInfo;
    file_task->CopyFrom(file);
    /// 只能用动态分配的空间，会在task 引用计数为0清理时delete
    task.set_allocated_file(file_task);
    static int task_id = 0;
    task_id++;
    task.set_index(task_id);
    task.set_tasktime(XTools::XGetTime(0));

    {
        XMutex mutex(&impl_->uploads_mutex_);
        impl_->uploads_.push_back(task);
    }
    this->uploadProcess(task_id, 0);
    return task_id;
}

auto XFileManager::addDownloadTask(const xdisk::XFileInfo &file) -> int
{
    xdisk::XFileTask task;
    auto             file_task = new xdisk::XFileInfo();
    file_task->CopyFrom(file);
    /// 只能用动态分配的空间，会在task 引用计数为0清理时delete
    task.set_allocated_file(file_task);
    static int task_id = 0;
    task_id++;
    task.set_index(task_id);
    task.set_tasktime(XTools::XGetTime(0));

    {
        XMutex mutex(&impl_->downloads_mutex_);
        impl_->downloads_.push_back(task);
    }
    this->downloadProcess(task_id, 0);
    return task_id;
}

auto XFileManager::upload_servers() const -> xmsg::XServiceList
{
    XMUTEX(&impl_->servers_mutex_);
    return impl_->upload_servers_;
}

auto XFileManager::download_servers() const -> xmsg::XServiceList
{
    XMUTEX(&impl_->servers_mutex_);
    return impl_->download_servers_;
}

auto XFileManager::set_upload_servers(const xmsg::XServiceList &servers) -> void
{
    XMUTEX(&impl_->servers_mutex_);
    impl_->upload_servers_ = servers;
}

auto XFileManager::set_download_servers(const xmsg::XServiceList &servers) -> void
{
    XMUTEX(&impl_->servers_mutex_);
    impl_->download_servers_ = servers;
}

auto XFileManager::setLogin(const xmsg::XLoginRes &login) -> void
{
    impl_->login_ = login;
}

auto XFileManager::getLogin() const -> xmsg::XLoginRes
{
    return impl_->login_;
}

auto XFileManager::set_password(const std::string &pass) -> void
{
    impl_->password_ = pass;
}

auto XFileManager::password() const -> std::string
{
    return impl_->password_;
}

auto XFileManager::uploadProcess(int task_id, int sended) -> void
{
    XMutex mutex(&impl_->uploads_mutex_);
    //uploads_mutex_.lock();
    for (auto &upload : impl_->uploads_)
    {
        if (task_id == upload.index())
        {
            upload.mutable_file()->set_net_size(sended);
        }
    }
    //uploads_mutex_.unlock();
    RefreshUploadTask(impl_->uploads_);
}

auto XFileManager::uploadEnd(int task_id) -> void
{
    //uploads_mutex_.lock();
    XMutex mutex(&impl_->uploads_mutex_);
    for (auto &upload : impl_->uploads_)
    {
        if (task_id == upload.index())
        {
            upload.set_is_complete(true);
            upload.mutable_file()->set_net_size(upload.file().filesize());
        }
    }
    //uploads_mutex_.unlock();
    RefreshUploadTask(impl_->uploads_);
}

auto XFileManager::downloadProcess(int task_id, int recved) -> void
{
    XMutex mutex(&impl_->downloads_mutex_);
    //uploads_mutex_.lock();
    for (auto &download : impl_->downloads_)
    {
        if (task_id == download.index())
        {
            download.mutable_file()->set_net_size(recved);
        }
    }
    //uploads_mutex_.unlock();
    RefreshDownloadTask(impl_->downloads_);
}

auto XFileManager::downloadEnd(int task_id) -> void
{
    XMutex mutex(&impl_->downloads_mutex_);
    for (auto &download : impl_->downloads_)
    {
        if (task_id == download.index())
        {
            download.set_is_complete(true);
            download.mutable_file()->set_net_size(download.file().filesize());
        }
    }
    //uploads_mutex_.unlock();
    RefreshDownloadTask(impl_->downloads_);
}
