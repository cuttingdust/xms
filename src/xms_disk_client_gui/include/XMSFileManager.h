/**
 * @file   XMSFileManager.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-13
 */

#ifndef XMSFILEMANAGER_H
#define XMSFILEMANAGER_H

#include "XFileManager.h"

namespace xdisk
{
    class XFileInfo;
}

class XMSFileManager final : public XFileManager
{
public:
    XMSFileManager();
    ~XMSFileManager() override;

public:
    auto initFileManager(std::string server_ip, int server_port) -> void override;

    auto getDir(const std::string &root) -> void override;

    auto newDir(const std::string &path) -> void override;

    auto deleteFile(const xdisk::XFileInfo &file) -> void override;

    auto uploadFile(const xdisk::XFileInfo &file) -> void override;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XMSFILEMANAGER_H
