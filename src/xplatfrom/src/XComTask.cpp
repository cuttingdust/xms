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

static auto SReadCb(struct bufferevent *bev, void *ctx) -> void
{
    auto task = static_cast<XComTask *>(ctx);
    task->readCB();
}

static auto SWriteCb(struct bufferevent *bev, void *ctx) -> void
{
    auto task = static_cast<XComTask *>(ctx);
    task->writeCB();
}

static auto SEventCb(struct bufferevent *bev, short events, void *ctx) -> void
{
    auto task = static_cast<XComTask *>(ctx);
    task->eventCB(events);
}

static auto STimerCB(evutil_socket_t s, short w, void *ctx) -> void
{
    auto task = static_cast<XComTask *>(ctx);
    task->timerCB();
}

static auto SAutoConnectTimerCB(evutil_socket_t s, short w, void *ctx) -> void
{
    auto task = static_cast<XComTask *>(ctx);
    task->autoConnectTimerCB();
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
    XComTask           *owenr_ = nullptr;
    struct bufferevent *bev_   = nullptr;
    std::string         serverPath_;
    char                server_ip_[16] = { 0 };
    int                 server_port_   = -1;
    char                buffer_[1024]  = { 0 };
    XMsg                msg_;
    bool                isRecvMsg      = true;  ///< 是否接受消息
    bool                isAutoDelete   = true;  ///< 是否自动删除
    bool                isAutoConnect  = false; ///< 是否自动连接
    char                client_ip_[16] = { 0 };
    int                 client_port_   = -1;
    char                local_ip_[16]  = { 0 }; ///< 本地微服务的ip
    int                 local_port_    = 0;     ///< 本地微服务的端口

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
    mtx_ = new std::mutex;
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
    sin.sin_port   = htons(impl_->server_port_);
    evutil_inet_pton(AF_INET, impl_->server_ip_, &sin.sin_addr.s_addr);
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
    if (impl_->server_ip_[0] == '\0')
    {
        return true;
    }

    setAutoConnectTimer(3000); /// 3秒自动连接一次

    return connect();
}

auto XComTask::setServerIp(const char *ip) -> void
{
    if (!ip)
    {
        return;
    }

    strncpy(impl_->server_ip_, ip, sizeof(impl_->server_ip_));
}

auto XComTask::getServerIP() const -> const char *
{
    return impl_->server_ip_;
}

auto XComTask::setServerPort(int port) -> void
{
    impl_->server_port_ = port;
}

auto XComTask::getServerPort() const -> int
{
    return impl_->server_port_;
}

auto XComTask::setClientIP(const char *ip) -> void
{
    if (!ip)
    {
        return;
    }
    strncpy(impl_->client_ip_, ip, sizeof(impl_->client_ip_));
}

auto XComTask::getClientIP() const -> char *
{
    return impl_->client_ip_;
}

auto XComTask::setClientPort(int port) -> void
{
    impl_->client_port_ = port;
}

auto XComTask::getClientPort() const -> int
{
    return impl_->client_port_;
}

auto XComTask::setLocalIP(const char *ip) -> void
{
    /// _CRT_SECURE_NO_WARNINGS
    if (!ip)
    {
        return;
    }
    strncpy(impl_->local_ip_, ip, sizeof(impl_->local_ip_));
}

auto XComTask::getLocalIP() const -> char *
{
    return impl_->local_ip_;
}

auto XComTask::setLocalPort(int port) -> void
{
    impl_->local_port_ = port;
}

auto XComTask::getLocalPort() const -> int
{
    return impl_->local_port_;
}

auto XComTask::setServerRoot(const std::string path) -> void
{
    impl_->serverPath_ = path;
}

auto XComTask::setIsRecvMsg(bool isRecvMsg) -> void
{
    impl_->isRecvMsg = isRecvMsg;
}

auto XComTask::setAutoDelete(bool bAuto) -> void
{
    impl_->isAutoDelete = bAuto;
}

auto XComTask::setAutoConnect(bool bAuto) -> void
{
    impl_->isAutoConnect = bAuto;
    if (bAuto)
    {
        impl_->isAutoDelete = false;
    }
}

auto XComTask::waitConnected(int timeout_sec) -> bool
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

auto XComTask::autoConnect(int timeout_sec) -> bool
{
    /// 如果正在连接，则等待，如果没有，则开始连接
    if (isConnected())
    {
        return true;
    }

    if (!isConnecting())
    {
        connect();
    }

    return waitConnected(timeout_sec);
}

auto XComTask::setSSLContent(XSSL_CTX *ctx) -> void
{
    impl_->ssl_ctx_ = ctx;
}

auto XComTask::getSSLContent() const -> XSSL_CTX *
{
    return impl_->ssl_ctx_;
}

auto XComTask::setReadTimeMs(int ms) -> void
{
    impl_->read_timeout_ms_ = ms;
}

auto XComTask::setTimeMs(int ms) -> void
{
    impl_->timer_ms_ = ms;
}

auto XComTask::eventCB(short events) -> void
{
    std::stringstream ss;
    ss << "SEventCB:" << events;

    if (events & BEV_EVENT_CONNECTED)
    {
        // std::cout << "BEV_EVENT_CONNECTED" << std::endl;
        std::stringstream ss;
        ss << "connnect server " << impl_->server_ip_ << ":" << impl_->server_port_ << " success!";
        LOGINFO(ss.str());

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
        /// 获取本地地址
        int sock = bufferevent_getfd(impl_->bev_);
        /*if (sock > 0)
        {
            sockaddr_in sin;
            memset(&sin, 0, sizeof(sin));
            int len = sizeof(sin);
            getsockname(sock, (sockaddr*)&sin, &len);
            char buf[16] = { 0 };
            evutil_inet_ntop(AF_INET, &sin.sin_addr.s_addr, buf, sizeof(buf));
            cout << "client ip is " << buf << endl;
        }*/

        connectCB();
    }

    /// 退出要处理缓冲内容
    if (events & BEV_EVENT_ERROR)
    {
        auto ssl = bufferevent_openssl_get_ssl(impl_->bev_);
        if (ssl)
        {
            XSSL xssl;
            xssl.set_ssl(ssl);
            xssl.printCert();
        }

        std::cout << "BEV_EVENT_ERROR" << std::endl;
        int sock = bufferevent_getfd(impl_->bev_);
        int err  = evutil_socket_geterror(sock);
        //LOGDEBUG(server_ip());
        //std::stringstream log;
        ss << getServerIP() << ":" << getServerPort() << " ";
        ss << getLocalIP() << evutil_socket_error_to_string(err);
        //LOGDEBUG(log.str().c_str());
        LOGINFO(ss.str());

        close();
    }

    if (events & BEV_EVENT_TIMEOUT)
    {
        ss << "BEV_EVENT_TIMEOUT";
        LOGINFO(ss.str());
        close();
    }

    if (events & BEV_EVENT_EOF)
    {
        std::cout << "BEV_EVENT_EOF" << std::endl;
        LOGINFO(ss.str());
        close();
    }
}

auto XComTask::connectCB() -> void
{
    std::cout << "XComTask::connectCB" << std::endl;
}

auto XComTask::read(void *data, int size) -> int
{
    if (!impl_->bev_)
    {
        LOGERROR("bev not set");
        return 0;
    }
    int re = bufferevent_read(impl_->bev_, data, size);
    return re;
}

auto XComTask::writeCB() -> void
{
    std::cout << "XComTask::writeCB" << std::endl;
}

auto XComTask::write(const void *data, int size) -> bool
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

auto XComTask::beginWriteCB() -> void
{
    if (!impl_->bev_)
        return;

    bufferevent_trigger(impl_->bev_, EV_WRITE, 0);
}

auto XComTask::close() -> void
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

auto XComTask::clearTimer() -> void
{
    if (impl_->auto_connect_timer_event_)
        event_free(impl_->auto_connect_timer_event_);
    impl_->auto_connect_timer_event_ = nullptr;

    if (impl_->timer_event_)
        event_free(impl_->timer_event_);
    impl_->timer_event_ = nullptr;
}

auto XComTask::setTimer(int ms) -> void
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

auto XComTask::timerCB() -> void
{
    std::cout << "XComTask::timerCB" << std::endl;
}

auto XComTask::setAutoConnectTimer(int ms) -> void
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

auto XComTask::autoConnectTimerCB() -> void
{
    /// 如果正在连接，则等待，如果没有，则开始连接
    if (isConnected())
    {
        return;
    }

    if (!isConnecting())
    {
        connect();
        std::cout << "." << std::flush;
    }
}
