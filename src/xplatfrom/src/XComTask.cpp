#include "XComTask.h"
#include "XMsg.h"
#include "XTools.h"
#include "XSSL_CTX.h"
#include "XMsgCom.pb.h"

#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/event.h>

#include <iostream>
#include <thread>

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

void STimerCB(evutil_socket_t s, short w, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->timerCB();
}

void SAutoConnectTimerCB(evutil_socket_t s, short w, void *ctx)
{
    auto task = static_cast<XComTask *>(ctx);
    task->AutoConnectTimerCB();
}

class XComTask::PImpl
{
public:
    PImpl(XComTask *owenr);
    ~PImpl();

public:
    /// \brief 初始化 bev
    /// \param com_sock -1 自动创建socket
    /// \return true 成功，false 失败
    auto initBev(int com_sock) -> bool;

public:
    XComTask           *owenr_        = nullptr;
    struct bufferevent *bev_          = nullptr;
    std::string         serverPath_   = "";
    char                serverIp_[16] = { 0 };
    int                 serverPort_   = -1;
    char                buffer_[1024] = { 0 };
    XMsg                msg_;
    bool                isRecvMsg     = true;  ///< 是否接受消息
    bool                isAutoDelete  = true;  ///< 是否自动删除
    bool                isAutoConnect = false; ///< 是否自动连接

    /// 客户单的连接状态
    /// 1 未处理  => 开始连接 （加入到线程池处理）
    /// 2 连接中 => 等待连接成功
    /// 3 已连接 => 做业务操作
    /// 4 连接后失败 => 根据连接间隔时间，开始连接
    bool        is_connecting_ = true;    ///< 连接中
    bool        is_connected_  = false;   ///< 连接成功
    std::mutex *mtx_           = nullptr; ///< 互斥锁

    struct event *timer_event_              = nullptr; ///< 定时器事件
    struct event *auto_connect_timer_event_ = nullptr; ///< 自动连接定时器事件 close时不清理
    XSSL_CTX     *ssl_ctx_                  = nullptr; ///< ssl上下文

    int read_timeout_ms_ = 0; ///< 读超时时间，毫秒
    int timer_ms_        = 0; ///< TimerCB 定时调用时间
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
    if (ssl_ctx_)
    {
        auto ssl = ssl_ctx_->createXSSL(com_sock);
        if (com_sock < 0)
        {
            bev_ = bufferevent_openssl_socket_new(owenr_->base(), com_sock, ssl->get_ssl(), BUFFEREVENT_SSL_CONNECTING,
                                                  BEV_OPT_CLOSE_ON_FREE);
        }
        else
        {
            bev_ = bufferevent_openssl_socket_new(owenr_->base(), com_sock, ssl->get_ssl(), BUFFEREVENT_SSL_ACCEPTING,
                                                  BEV_OPT_CLOSE_ON_FREE);
        }

        if (!bev_)
        {
            LOGERROR("bufferevent_openssl_socket_new failed");
            return false;
        }
    }
    else
    {
        bev_ = bufferevent_socket_new(owenr_->base(), com_sock, BEV_OPT_CLOSE_ON_FREE);
        if (!bev_)
        {
            LOGERROR("bufferevent_socket_new failed");
            return false;
        }
    }

    /// 设定读超时时间
    if (read_timeout_ms_ > 0)
    {
        ///秒，微妙
        timeval read_tv = { read_timeout_ms_ / 1000, (read_timeout_ms_ % 1000) * 1000 };
        bufferevent_set_timeouts(bev_, &read_tv, 0);
    }

    /// 定时器设定
    if (timer_ms_ > 0)
    {
        owenr_->setTimer(timer_ms_);
    }

    bufferevent_setcb(bev_, SReadCb, SWriteCb, SEventCb, owenr_);
    bufferevent_enable(bev_, EV_READ | EV_WRITE);
    return true;
}

XComTask::XComTask()
{
    impl_ = std::make_unique<PImpl>(this);
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

    setAutoConnectTimer(3000); /// 3秒自动连接一次

    return connect();
}

void XComTask::setServerIp(const char *ip)
{
    strncpy(impl_->serverIp_, ip, sizeof(impl_->serverIp_));
}

const char *XComTask::getServerIp() const
{
    return impl_->serverIp_;
}

void XComTask::setServerPort(int port)
{
    impl_->serverPort_ = port;
}

int XComTask::getServerPort() const
{
    return impl_->serverPort_;
}

void XComTask::setServerRoot(const std::string path)
{
    impl_->serverPath_ = path;
}

void XComTask::setIsRecvMsg(bool isRecvMsg)
{
    impl_->isRecvMsg = isRecvMsg;
}

void XComTask::setAutoDelete(bool bAuto)
{
    impl_->isAutoDelete = bAuto;
}

void XComTask::setAutoConnect(bool bAuto)
{
    impl_->isAutoConnect = bAuto;
    if (bAuto)
    {
        impl_->isAutoDelete = false;
    }
}

bool XComTask::waitConnected(int timeout_sec)
{
    /// 10毫秒监听一次
    int count = timeout_sec * 100;
    for (int i = 0; i < count; i++)
    {
        if (isConnected())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return isConnected();
}

bool XComTask::autoConnect(int timeout_sec)
{
    /// 如果正在连接，则等待，如果没有，则开始连接
    if (isConnected())
        return true;
    if (!isConnecting())
        connect();
    return waitConnected(timeout_sec);
}

void XComTask::setSSLContent(XSSL_CTX *ctx)
{
    impl_->ssl_ctx_ = ctx;
}

XSSL_CTX *XComTask::getSSLContent() const
{
    return impl_->ssl_ctx_;
}

void XComTask::setReadTime(int ms)
{
    impl_->read_timeout_ms_ = ms;
}

void XComTask::setTime(int ms)
{
    impl_->timer_ms_ = ms;
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

        if (auto ssl = bufferevent_openssl_get_ssl(impl_->bev_))
        {
            XSSL xssl;
            xssl.set_ssl(ssl);
            xssl.printCipher();
            xssl.printCert();
        }

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
    if (impl_->isAutoDelete)
    {
        clearTimer();
        delete this;
    }
}

void XComTask::clearTimer()
{
    if (impl_->auto_connect_timer_event_)
        event_free(impl_->auto_connect_timer_event_);
    impl_->auto_connect_timer_event_ = nullptr;

    if (impl_->timer_event_)
        event_free(impl_->timer_event_);
    impl_->timer_event_ = nullptr;
}

void XComTask::setTimer(int ms)
{
    if (!base())
    {
        LOGERROR("SetTimer failed : base not set!");
        return;
    }

    impl_->timer_event_ = event_new(base(), -1, EV_PERSIST, STimerCB, this);
    if (!impl_->timer_event_)
    {
        LOGERROR("set timer failed :event_new faield!");
        return;
    }
    int     sec = ms / 1000;          /// 秒
    int     us  = (ms % 1000) * 1000; /// 微妙
    timeval tv  = { .tv_sec = sec, .tv_usec = us };
    event_add(impl_->timer_event_, &tv);
}

void XComTask::timerCB()
{
    std::cout << "XComTask::timerCB" << std::endl;
}

void XComTask::setAutoConnectTimer(int ms)
{
    if (!base())
    {
        LOGERROR("setAutoConnectTimer failed : base not set!");
        return;
    }

    if (impl_->auto_connect_timer_event_)
    {
        event_free(impl_->auto_connect_timer_event_);
        impl_->auto_connect_timer_event_ = nullptr;
    }

    impl_->auto_connect_timer_event_ = event_new(base(), -1, EV_PERSIST, SAutoConnectTimerCB, this);
    if (!impl_->auto_connect_timer_event_)
    {
        LOGERROR("set autoConnectTimer failed :event_new faield!");
        return;
    }
    int     sec = ms / 1000;          /// 秒
    int     us  = (ms % 1000) * 1000; /// 微妙
    timeval tv  = { sec, us };
    event_add(impl_->auto_connect_timer_event_, &tv);
}

void XComTask::AutoConnectTimerCB()
{
    std::cout << "." << std::flush;
    /// 如果正在连接，则等待，如果没有，则开始连接
    if (isConnected())
        return;
    if (!isConnecting())
        connect();
}
