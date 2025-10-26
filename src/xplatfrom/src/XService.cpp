#include "XService.h"

#include "XThreadPool.h"
#include "XTools.h"

#include <event2/listener.h>
#include <event2/bufferevent.h>

#ifdef _WIN32
#include <WinSock2.h>
#endif

static void SListenCB(struct evconnlistener *ev, evutil_socket_t sock, struct sockaddr *addr, int socklen, void *arg)
{
    LOGDEBUG("SListenCB");
    auto task = static_cast<XService *>(arg);
    task->listenCB(sock, addr, socklen);
}

class XService::PImpl
{
public:
    PImpl(XService *owenr);
    ~PImpl();

public:
    XService    *owenr_              = nullptr;
    XThreadPool *thread_listen_pool_ = nullptr; /// < 接收用户连接的线程池
    XThreadPool *thread_client_pool_ = nullptr; /// < 处理用户的数据的连接池
    int          server_port_        = 0;       /// < 服务器监听端口
    int          thread_count_       = 10;      /// < 客户数据处理的线程数量
    XSSL_CTX    *ssl_ctx_            = nullptr; /// < ssl上下文
};

XService::PImpl::PImpl(XService *owenr) : owenr_(owenr)
{
    thread_listen_pool_ = XThreadPoolFactory::create();
    thread_client_pool_ = XThreadPoolFactory::create();
}

XService::PImpl::~PImpl()
{
    if (thread_listen_pool_)
    {
        delete thread_listen_pool_;
        thread_listen_pool_ = nullptr;
    }

    if (thread_client_pool_)
    {
        delete thread_client_pool_;
        thread_client_pool_ = nullptr;
    }
}

XService::XService()
{
    impl_ = std::make_unique<PImpl>(this);
}

XService::~XService() = default;

auto XService::setServerPort(int port) -> void
{
    impl_->server_port_ = port;
}

auto XService::init() -> bool
{
    if (impl_->server_port_ <= 0)
    {
        LOGERROR("server port is not set!!!");
        return false;
    }

    /// 绑定端口
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(impl_->server_port_);
    auto evc       = evconnlistener_new_bind(base(), SListenCB, this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                             10, /// listen back
                                             reinterpret_cast<sockaddr *>(&sin), sizeof(sin));

    if (!evc)
    {
        std::stringstream ss;
        ss << "listen port " << impl_->server_port_ << " failed!!!" << std::endl;
        LOGERROR(ss.str());
        return false;
    }

    std::stringstream ss;
    ss << "listen port " << impl_->server_port_ << " success!!!" << std::endl;
    LOGINFO(ss.str());
    return true;
}

auto XService::listenCB(int client_socket, struct sockaddr *addr, int socketlen) -> void
{
    /// 创建客户端处理对象
    auto handle = createHandle();
    handle->setSocket(client_socket);
    handle->setSSLContent(this->getSSLContent());

    std::stringstream ss;
    char              ip[16] = { 0 };
    const auto        adder  = reinterpret_cast<sockaddr_in *>(addr);
    evutil_inet_ntop(AF_INET, &adder->sin_addr.s_addr, ip, sizeof(ip));
    int client_port = ntohs(adder->sin_port);
    ss << "accept client ip :" << ip << " port:" << client_port << std::endl;
    LOGINFO(ss.str());

    /// 任务加入到线程池
    handle->setClientIP(ip);
    handle->setClientPort(client_port);
    impl_->thread_client_pool_->dispatch(handle);
}

auto XService::start() -> bool
{
    impl_->thread_listen_pool_->init(1);
    impl_->thread_client_pool_->init(impl_->thread_count_);
    impl_->thread_listen_pool_->dispatch(this);
    return true;
}

auto XService::wait() -> void
{
    XThreadPool::wait();
}

auto XService::setSSLContent(XSSL_CTX *ctx) -> void
{
    impl_->ssl_ctx_ = ctx;
}

auto XService::getSSLContent() const -> XSSL_CTX *
{
    return impl_->ssl_ctx_;
}
