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

namespace xdisk
{
    class XFileInfoList;
}

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
    /// \brief 获取目录
    /// \param root
    virtual auto getDir(const std::string& root) -> void = 0;

signals:
    void RefreshData(xdisk::XFileInfoList file_list, std::string cur_dir);

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XFILEMANAGER_H
