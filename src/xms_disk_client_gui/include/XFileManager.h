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
    class XLoginRes;
}

namespace xdisk
{
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

public:
    /// \brief 初始化管理器
    /// \param server_ip
    /// \param server_port
    virtual auto initFileManager(std::string server_ip, int server_port) -> void = 0;

    /// \brief 设置登录信息
    /// \param login
    virtual auto setLogin(xmsg::XLoginRes login) -> void;

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
    virtual auto deleteFile(const xdisk::XFileInfo& file) -> void = 0;

    /// \brief 开始上传文件
    /// \param file
    virtual auto uploadFile(const xdisk::XFileInfo& file) -> void = 0;

signals:
    void RefreshData(xdisk::XFileInfoList file_list, std::string cur_dir);

    void RefreshDiskInfo(xdisk::XDiskInfo info);

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XFILEMANAGER_H
