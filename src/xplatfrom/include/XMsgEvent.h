/**
 * @file   XMsgEvent.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-22
 */

#ifndef XMSGEVENT_H
#define XMSGEVENT_H

#include "XMsgType.pb.h"
#include "XMsg.h"
#include "XComTask.h"
#include "XMsgCom.pb.h"

#include <memory>

/// 不调用bufferevent接口，直接调用XComTask的封装
class XMsgEvent : public XComTask
{
public:
    XMsgEvent();
    ~XMsgEvent() override;

public:
    /// \brief 接收消息 分发消息
    void readCB() override;

public:
    /// \brief 接收数据包，
    /// 1 正确接收到消息  (调用消息处理函数)
    /// 2 消息接收不完整 (等待下一次接收)
    /// 3 消息接收出错 （退出清理空间）
    /// \return  1 2 返回true 3返回false
    auto recvMsg() -> bool;

    /// \brief 获取接收到的数据包，（不包含头部消息）,
    /// 由调用者清理XMsg
    /// \return 如果没有完整的数据包，返回NULL
    auto getMsg() const -> XMsg *;


    auto sendMsg(xmsg::XMsgHead *head, const google::protobuf::Message *msg) -> bool;

    /// \brief 发送消息 包含头部（自动创建）
    /// \param msgType  消息类型
    /// \param msg      消息内容
    auto sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg) -> bool;


    /// \brief 清理缓存消息头和消息内容，用于接收下一次消息
    auto clear() -> void;

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};


#endif // XMSGEVENT_H
