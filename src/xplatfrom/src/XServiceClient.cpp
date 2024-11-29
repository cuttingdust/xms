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
    impl_ = std::make_shared<PImpl>(this);
}

XServiceClient::~XServiceClient() = default;

void XServiceClient::startConnect()
{
    impl_->thread_pool_->init(1);
    impl_->thread_pool_->dispatch(this);
}
