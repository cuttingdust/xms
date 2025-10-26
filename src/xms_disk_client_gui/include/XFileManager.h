/**
 * @file   XFileManager.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-13
 */

#ifndef XFILEMANAGER_H
#define XFILEMANAGER_H

#include <QtCore/QObject>

namespace xmsg
{
    class XServiceList;
    class XLoginRes;
} // namespace xmsg

namespace xdisk
{
    class XFileTask;
    class XDiskInfo;
    class XFileInfo;
    class XFileInfoList;
} // namespace xdisk


class XFileManager : public QObject
{
    Q_OBJECT
public:
    static XFileManager* Instance();

protected:
    XFileManager();
    ~XFileManager() override;

public:
    /// \brief 设置父类
    /// \param parent
    auto setParent(XFileManager* parent) -> void;

    /// \brief 设置目录
    /// \param root
    auto setRoot(const std::string& root) -> void;

    /// \brief 刷新目录
    auto refreshDir() -> void;

    /// \brief 返回任务ID
    /// \param file
    /// \return
    auto addUploadTask(const xdisk::XFileInfo& file) -> int;

    auto addDownloadTask(const xdisk::XFileInfo& file) -> int;

    auto upload_servers() const -> xmsg::XServiceList;
    auto download_servers() const -> xmsg::XServiceList;

    auto set_upload_servers(const xmsg::XServiceList& servers) -> void;
    auto set_download_servers(const xmsg::XServiceList& servers) -> void;

public:
    /// \brief 初始化管理器
    /// \param server_ip
    /// \param server_port
    virtual auto initFileManager(std::string server_ip, int server_port) -> void = 0;

    /// \brief 设置登录信息
    /// \param login
    virtual auto setLogin(const xmsg::XLoginRes& login) -> void;

    /// \brief 得到登录信息
    /// \return
    virtual auto getLogin() const -> xmsg::XLoginRes;

    /// \brief 获取目录
    /// \param root
    virtual auto getDir(const std::string& root) -> void = 0;

    /// \brief 新建目录
    /// \param root
    virtual auto newDir(const std::string& root) -> void = 0;

    /// \brief 删除目录
    /// \param file
    virtual auto deleteFile(xdisk::XFileInfo file) -> void = 0;

    /// \brief 开始上传文件
    /// \param file
    virtual auto uploadFile(xdisk::XFileInfo file) -> void = 0;

    /// \brief 设置文件密钥
    /// \param pass
    virtual auto set_password(const std::string& pass) -> void;

    virtual auto password() const -> std::string;

    /// \brief 进度从0~1000 更新上传列表进度 线程安全
    /// \param task_id_
    /// \param sended
    virtual auto uploadProcess(int task_id, int sended) -> void;
    virtual auto uploadEnd(int task_id) -> void;

    /// \brief 开始下载文件
    /// \param file
    virtual auto downloadFile(xdisk::XFileInfo file) -> void = 0;

    virtual auto downloadProcess(int task_id, int recved) -> void;
    virtual auto downloadEnd(int task_id) -> void;

signals:
    void RefreshData(xdisk::XFileInfoList file_list, std::string cur_dir);

    void RefreshUploadTask(std::list<xdisk::XFileTask> file_list);

    void RefreshDownloadTask(std::list<xdisk::XFileTask> file_list);

    void RefreshDiskInfo(xdisk::XDiskInfo info);

    void ErrorSig(std::string str);

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XFILEMANAGER_H
