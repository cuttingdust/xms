#include "XServiceClient.h"
#include "XThreadPool.h"
#include "XTools.h"

#include <mutex>

class XServiceClient::PImpl
{
public:
    PImpl(XServiceClient *owenr);
    ~PImpl();

public:
    XServiceClient  *owenr_       = nullptr;
    XThreadPool     *thread_pool_ = nullptr;
    std::string      service_name_;
    xmsg::XLoginRes *login_ = nullptr;
    std::mutex       login_mutex_;
};

XServiceClient::PImpl::PImpl(XServiceClient *owenr) : owenr_(owenr)
{
    thread_pool_ = XThreadPoolFactory::create();
}

XServiceClient::PImpl::~PImpl()
{
    // delete thread_pool_;
    // thread_pool_ = nullptr;
}


XServiceClient::XServiceClient()
{
    impl_ = std::make_unique<PImpl>(this);
}

XServiceClient::~XServiceClient() = default;

auto XServiceClient::setLogin(xmsg::XLoginRes *login) -> void
{
    XMutex mux(&impl_->login_mutex_);
    if (!impl_->login_)
    {
        impl_->login_ = new xmsg::XLoginRes;
    }
    impl_->login_->CopyFrom(*login);
    LOGINFO(impl_->login_->DebugString());
}

auto XServiceClient::startConnect() -> void
{
    impl_->thread_pool_->init(1);
    impl_->thread_pool_->dispatch(this);
    setAutoDelete(false);
}

auto XServiceClient::setServiceName(const std::string &serviceName) -> void
{
    impl_->service_name_ = serviceName;
}

auto XServiceClient::getServiceName() const -> std::string
{
    return impl_->service_name_;
}

auto XServiceClient::setHead(xmsg::XMsgHead *head) -> xmsg::XMsgHead *
{
    if (!head)
    {
        return nullptr;
    }

    if (impl_->service_name_.empty())
    {
        LOGDEBUG("service name is empty!");
    }
    else if (head->servername().empty())
    {
        head->set_servername(impl_->service_name_);
    }
    XMutex mux(&impl_->login_mutex_);
    if (!impl_->login_)
        return head;
    head->set_token(impl_->login_->token());
    head->set_username(impl_->login_->username());
    head->set_rolename(impl_->login_->rolename());
    return head;
}

auto XServiceClient::sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg) -> bool
{
    xmsg::XMsgHead head;
    head.set_msgtype(msgType);

    this->setHead(&head);
    return XMsgEvent::sendMsg(&head, msg);
}

auto XServiceClient::sendMsg(xmsg::XMsgHead *head, const google::protobuf::Message *msg) -> bool
{
    return XMsgEvent::sendMsg(head, msg);
}
