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
    auto initBev(int com_sock) -> bool;

public:
    XComTask           *owenr_        = nullptr;
    struct bufferevent *bev_          = nullptr;
    std::string         serverPath_   = "";
    char                serverIp_[16] = { 0 };
    int                 serverPort_   = -1;
    char                buffer_[1024] = { 0 };
    XMsg                msg_;
    bool                isRecvMsg = true; ///< 是否接受消息

    /// 客户单的连接状态
    /// 1 未处理  => 开始连接 （加入到线程池处理）
    /// 2 连接中 => 等待连接成功
    /// 3 已连接 => 做业务操作
    /// 4 连接后失败 => 根据连接间隔时间，开始连接
    bool        is_connecting_ = true;  ///< 连接中
    bool        is_connected_  = false; ///< 连接成功
    std::mutex *mtx_           = nullptr;
};

XComTask::PImpl::PImpl(XComTask *owenr) : owenr_(owenr)
{
    mtx_ = new std::mutex();
}

XComTask::PImpl::~PImpl()
{
    if (mtx_)
    {
        delete mtx_;
        mtx_ = nullptr;
    }
}

auto XComTask::PImpl::initBev(int com_sock) -> bool
{
    /// 用bufferevent建立连接
    bev_ = bufferevent_socket_new(owenr_->base(), com_sock, BEV_OPT_CLOSE_ON_FREE);
    if (!bev_)
    {
        LOGERROR("bufferevent_socket_new failed");
        return false;
    }

    bufferevent_setcb(bev_, SReadCb, SWriteCb, SEventCb, owenr_);
    bufferevent_enable(bev_, EV_READ | EV_WRITE);
    return true;
}

XComTask::XComTask()
{
    impl_ = std::make_shared<PImpl>(this);
}

XComTask::~XComTask() = default;

auto XComTask::connect() const -> bool
{
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(impl_->serverPort_);
    evutil_inet_pton(AF_INET, impl_->serverIp_, &sin.sin_addr.s_addr);
    XMutex xMtx(impl_->mtx_);
    impl_->is_connected_  = false;
    impl_->is_connecting_ = false;
    if (!impl_->bev_)
        impl_->initBev(-1);
    if (!impl_->bev_)
    {
        LOGERROR("XComTask::Connect failed! bev is null!");
        return false;
    }

    int ret = bufferevent_socket_connect(impl_->bev_, reinterpret_cast<sockaddr *>(&sin), sizeof(sin));
    if (ret < 0)
    {
        std::cerr << "bufferevent_socket_connect failed" << std::endl;
        return false;
    }
    /// 开始连接
    impl_->is_connecting_ = true;
    return true;
}

auto XComTask::isConnected() const -> bool
{
    return impl_->is_connected_;
}

auto XComTask::isConnecting() const -> bool
{
    return impl_->is_connecting_;
}

auto XComTask::init() -> bool
{
    /// 区分客户端和服务器
    int comSock = this->sock();
    if (comSock <= 0)
        comSock = -1;

    {
        XMutex xMtx(impl_->mtx_);
        impl_->initBev(comSock);
    }

    // timeval tv = { 3, 0 };
    // bufferevent_set_timeouts(impl_->bev_, &tv, &tv);

    /// 连接服务器
    if (impl_->serverIp_[0] == '\0')
    {
        return true;
    }

    return connect();
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
        std::stringstream ss;
        ss << "connnect server " << impl_->serverIp_ << ":" << impl_->serverPort_ << " success!";
        LOGINFO(ss.str().c_str());

        /// 连接成功后发送消息
        impl_->is_connected_  = true;
        impl_->is_connecting_ = false;
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
    std::cout << "XComTask::writeCB" << std::endl;
}

bool XComTask::write(const void *data, int size)
{
    XMutex xMtx(impl_->mtx_);
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
    {
        XMutex xMtx(impl_->mtx_);

        impl_->is_connected_  = false;
        impl_->is_connecting_ = false;

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
    }

    /// TODO 清理连接对象空间，如果断开重连，需要单独处理
    delete this;
}
