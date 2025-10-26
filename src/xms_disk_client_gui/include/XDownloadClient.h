/**
 * @file   XDownloadClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-24
 */

#ifndef XDOWNLOADCLIENT_H
#define XDOWNLOADCLIENT_H

#include <XServiceClient.h>

namespace xdisk
{
    class XFileInfo;
}

class XDownloadClient : public XServiceClient
{
public:
    XDownloadClient();
    ~XDownloadClient() override;

public:
    auto connectCB() -> void override;

    /// \brief 通过定时器跟踪进度
    auto timerCB() -> void override;

    auto set_task_id(int task_id) -> void;

    auto task_id() const -> int;

    auto setFile(const xdisk::XFileInfo &file) -> bool;

    auto downloadFileRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto downloadSliceReq(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XDOWNLOADCLIENT_H
