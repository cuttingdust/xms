#include "XServiceClient.h"
#include "XThreadPool.h"

class XServiceClient::PImpl
{
public:
    PImpl(XServiceClient *owenr);
    ~PImpl();

public:
    XServiceClient *owenr_       = nullptr;
    XThreadPool    *thread_pool_ = nullptr;
    std::string     service_name_;
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
