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

    /// \brief 通过定时器跟踪进度
    auto timerCB() -> void override;

    auto uploadFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto uploadFileEndRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto sendSliceRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;

    auto setFile(const xdisk::XFileInfo &file) -> bool;

    auto set_task_id(int task_id) -> void;
    auto task_id() const -> int;


private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XUPLOADCLIENT_H
