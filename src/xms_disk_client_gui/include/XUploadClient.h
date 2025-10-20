/**
 * @file   XUploadClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-20
 */

#ifndef XUPLOADCLIENT_H
#define XUPLOADCLIENT_H

#include <XServiceClient.h>

namespace xdisk
{
    class XFileInfo;
}

class XUploadClient : public XServiceClient
{
public:
    XUploadClient();
    ~XUploadClient() override;

public:
    auto connectCB() -> void override;

    auto uploadFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto uploadFileEndRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto sendSliceRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;

    auto setFile(const xdisk::XFileInfo &file) -> bool;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XUPLOADCLIENT_H
