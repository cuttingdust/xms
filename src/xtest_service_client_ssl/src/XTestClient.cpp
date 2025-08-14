#include "XTestClient.h"

#include <XTools.h>
#include <XMsgCom.pb.h>

#include <thread>

void XTestClient::connectCB()
{
    LOGDEBUG("XTestClient::connectCB()");
}

bool XTestClient::getDir(const std::string &path)
{
    LOGDEBUG("XTestClient::getDir");
    if (!autoConnect(300))
    {
        return false;
    }
    std::stringstream ss;
    ss << "XTestClient::getDir " << path;
    LOGDEBUG(ss.str().c_str());

    /// 发送pb的消息给服务
    xmsg::XDirReq req;
    req.set_path(path);

    xmsg::XMsgHead head;
    head.set_msgtype(xmsg::MT_DIR_REQ);
    head.set_servername("dir");
    head.set_token("test token");
    return sendMsg(&head, &req);
}

void XTestClient::dirRes(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("XTestClient::dirRes");
    if (!head || !msg || !msg->data || msg->size <= 0)
    {
        LOGDEBUG("XTestClient::dirRes msg error");
        return;
    }

    xmsg::XDirRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("res.ParseFromArray failed!");
        return;
    }

    std::cout << "============= recv dir ==================" << std::endl;
    for (int i = 0; i < res.dirs_size(); i++)
    {
        const auto       &dir = res.dirs(i);
        std::stringstream ss;
        ss << "filename = " << dir.filename() << " filesize = " << dir.filesize();
        LOGDEBUG(ss.str().c_str());
    }
}

void XTestClient::regMsgCallback()
{
    regCB(xmsg::MT_DIR_RES, static_cast<MsgCBFunc>(&XTestClient::dirRes));
}


bool XTestClient::autoConnect(int timeout_ms)
{
    LOGDEBUG("XTestClient::autoConnect");
    /// 1 已连接
    if (isConnected())
        return true;
    /// 2 未连接 也不在连接中
    if (!isConnecting())
    {
        /// 开始连接
        if (!connect())
            return false;
    }

    int count = timeout_ms / 10;
    /// 连接中
    for (int i = 0; i < count; i++)
    {
        if (isConnected())
            return true;
        if (!isConnecting())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return isConnected();
}
