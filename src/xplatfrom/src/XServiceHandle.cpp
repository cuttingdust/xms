#include "XServiceHandle.h"

class XServiceHandle::PImpl
{
public:
    PImpl(XServiceHandle *owenr);
    ~PImpl() = default;

public:
    XServiceHandle *owenr_         = nullptr;
    char            client_ip_[16] = { 0 };
    int             client_port_   = 0;
};

XServiceHandle::PImpl::PImpl(XServiceHandle *owenr) : owenr_(owenr)
{
}


XServiceHandle::XServiceHandle()
{
    impl_ = std::make_unique<PImpl>(this);
}

XServiceHandle::~XServiceHandle()
{
}

auto XServiceHandle::setClientIP(const char *ip) -> void
{
    if (!ip)
        return;
    strncpy(impl_->client_ip_, ip, sizeof(impl_->client_ip_));
}

auto XServiceHandle::clientIP() -> const char *
{
    return impl_->client_ip_;
}

void XServiceHandle::setClientPort(int port)
{
    impl_->client_port_ = port;
}

void XServiceHandle::close()
{
    XMsgEvent::close();
}
