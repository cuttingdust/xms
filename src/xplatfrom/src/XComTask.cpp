#include "XComTask.h"
#include "XMsg.h"
#include "XTools.h"
#include "XMsgCom.pb.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <iostream>

static void SReadCb(struct bufferevent *bev, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->readCB();
}

static void SWriteCb(struct bufferevent *bev, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->writeCB();
}

static void SEventCb(struct bufferevent *bev, short events, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->eventCB(events);
}

class XComTask::PImpl
{
public:
    PImpl(XComTask *owenr);
    ~PImpl();

public:
    XComTask           *owenr_        = nullptr;
    struct bufferevent *bev_          = nullptr;
    std::string         serverPath_   = "";
    char                serverIp_[16] = { 0 };
    int                 serverPort_   = -1;
    char                buffer_[1024] = { 0 };
    XMsg                msg_;

    bool isRecvMsg = true; /// 是否接受消息
};

XComTask::PImpl::PImpl(XComTask *owenr) : owenr_(owenr)
{
}

XComTask::PImpl::~PImpl() = default;

XComTask::XComTask()
{
    impl_ = std::make_shared<PImpl>(this);
}

XComTask::~XComTask() = default;

auto XComTask::init() -> bool
{
    /// 区分客户端和服务器
    int comSock = this->sock();
    if (comSock <= 0)
        comSock = -1;

    /// 用bufferevent建立连接
    impl_->bev_ = bufferevent_socket_new(base(), comSock, BEV_OPT_CLOSE_ON_FREE);
    if (!impl_->bev_)
    {
        std::cerr << "bufferevent_socket_new failed" << std::endl;
        return false;
    }

    bufferevent_setcb(impl_->bev_, SReadCb, SWriteCb, SEventCb, this);
    bufferevent_enable(impl_->bev_, EV_READ | EV_WRITE);

    timeval tv = { 3, 0 };
    bufferevent_set_timeouts(impl_->bev_, &tv, &tv);

    /// 连接服务器
    if (impl_->serverIp_[0] == '\0')
    {
        return true;
    }

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(impl_->serverPort_);
    evutil_inet_pton(AF_INET, impl_->serverIp_, &sin.sin_addr.s_addr);

    int ret = bufferevent_socket_connect(impl_->bev_, reinterpret_cast<sockaddr *>(&sin), sizeof(sin));
    if (ret < 0)
    {
        std::cerr << "bufferevent_socket_connect failed" << std::endl;
        return false;
    }

    return true;
}

void XComTask::setServerIp(const char *ip)
{
    strncpy(impl_->serverIp_, ip, sizeof(impl_->serverIp_));
}

void XComTask::setServerPort(int port)
{
    impl_->serverPort_ = port;
}

void XComTask::setServerRoot(const std::string path)
{
    impl_->serverPath_ = path;
}

void XComTask::setIsRecvMsg(bool isRecvMsg)
{
    impl_->isRecvMsg = isRecvMsg;
}

void XComTask::eventCB(short events)
{
    if (events & BEV_EVENT_CONNECTED)
    {
        std::cout << "BEV_EVENT_CONNECTED" << std::endl;

        /// 连接成功后发送消息
        connectCB();
    }

    /// 退出要处理缓冲内容
    if (events & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT" << std::endl;
        close();
    }

    if (events & BEV_EVENT_EOF)
    {
        std::cout << "BEV_EVENT_EOF" << std::endl;
        close();
    }
}

void XComTask::connectCB()
{
    std::cout << "XComTask::connectCB" << std::endl;
}

int XComTask::read(void *data, int size)
{
    if (!impl_->bev_)
    {
        LOGERROR("bev not set");
        return 0;
    }
    int re = bufferevent_read(impl_->bev_, data, size);
    return re;
}

void XComTask::writeCB()
{
    std::cout << "writeCB" << std::endl;
}

bool XComTask::write(const void *data, int size)
{
    if (!impl_->bev_ || !data || size <= 0)
    {
        return false;
    }
    int len = bufferevent_write(impl_->bev_, data, size);
    if (len != 0)
    {
        return false;
    }
    return true;
}

void XComTask::beginWriteCB()
{
    if (!impl_->bev_)
        return;

    bufferevent_trigger(impl_->bev_, EV_WRITE, 0);
}

void XComTask::close()
{
    if (impl_->bev_)
    {
        bufferevent_free(impl_->bev_);
        impl_->bev_ = nullptr;
    }

    if (impl_->msg_.data)
    {
        delete impl_->msg_.data;
        memset(&impl_->msg_, 0, sizeof(impl_->msg_));
    }

    delete this;
}
