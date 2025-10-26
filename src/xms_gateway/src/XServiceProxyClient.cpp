#include "XServiceProxyClient.h"

#include "XAuthProxy.h"

#include <XTools.h>

#include <mutex>

class XServiceProxyClient::PImpl
{
public:
    PImpl(XServiceProxyClient *owenr);
    ~PImpl() = default;

public:
    XServiceProxyClient *owenr_ = nullptr;

    /// 消息转发的对象，一个proxy对应多个XMsgEven
    /// 用指针的值作为索引，要兼容64位
    std::map<long long, XMsgEvent *> callback_task_;

    std::mutex callback_task_mutex_;

    bool is_find_ = false;
};

XServiceProxyClient::PImpl::PImpl(XServiceProxyClient *owenr) : owenr_(owenr)
{
}


auto XServiceProxyClient::create(std::string service_name) -> XServiceProxyClient *
{
    if (service_name == AUTH_NAME)
    {
        return new XAuthProxy;
    }
    return new XServiceProxyClient;
}

XServiceProxyClient::XServiceProxyClient()
{
    impl_ = std::make_unique<PImpl>(this);
}

XServiceProxyClient::~XServiceProxyClient() = default;

auto XServiceProxyClient::setFindFlag(bool bFind) -> void
{
    impl_->is_find_ = bFind;
}

auto XServiceProxyClient::isFind() const -> bool
{
    return impl_->is_find_;
}

auto XServiceProxyClient::sendMsg(xmsg::XMsgHead *head, XMsg *msg, XMsgEvent *ev) -> bool
{
    regEvent(ev);
    head->set_msgid(reinterpret_cast<long long>(ev));
    return XMsgEvent::sendMsg(head, msg);
}

auto XServiceProxyClient::regEvent(XMsgEvent *ev) -> void
{
    XMutex mux(&impl_->callback_task_mutex_);
    impl_->callback_task_[reinterpret_cast<long long>(ev)] = ev;
}

auto XServiceProxyClient::readCB(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    if (!head || !msg)
    {
        return;
    }

    std::cout << "***************************************" << std::endl;
    std::cout << head->DebugString();
    XMutex mux(&impl_->callback_task_mutex_);
    /// 转发给XRouteHandle
    /// 每个XServiceProxyClient对象可能管理多个XRouterHandle
    auto router = impl_->callback_task_.find(head->msgid());
    if (router == impl_->callback_task_.end())
    {
        LOGDEBUG("callback_task_ can't find");
        return;
    }

    /// TODO 多线程问题？？
    router->second->sendMsg(head, msg);
}

auto XServiceProxyClient::delEvent(XMsgEvent *ev) -> void
{
    XMutex mux(&impl_->callback_task_mutex_);
    impl_->callback_task_.erase(reinterpret_cast<long long>(ev));
}
