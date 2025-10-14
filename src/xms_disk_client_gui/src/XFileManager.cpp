#include "XFileManager.h"

#include <XMsgCom.pb.h>
#include <XDiskCom.pb.h>

class XFileManager::PImpl
{
public:
    PImpl(XFileManager *owenr);
    ~PImpl() = default;

public:
    XFileManager        *owenr_ = nullptr;
    std::string          root_  = "";
    static XFileManager *instance_;
    xmsg::XLoginRes      login_;
};
/// 静态成员的定义和初始化
XFileManager *XFileManager::PImpl::instance_ = nullptr;

XFileManager::PImpl::PImpl(XFileManager *owenr) : owenr_(owenr)
{
}

XFileManager *XFileManager::Instance()
{
    return PImpl::instance_;
}

XFileManager::XFileManager()
{
    impl_ = std::make_unique<XFileManager::PImpl>(this);
}

XFileManager::~XFileManager() = default;

auto XFileManager::setParent(XFileManager *parent) -> void
{
    PImpl::instance_ = parent;
}

auto XFileManager::setRoot(const std::string &root) -> void
{
    impl_->root_ = root;
}

auto XFileManager::setLogin(xmsg::XLoginRes login) -> void
{
    impl_->login_ = login;
}

auto XFileManager::getLogin() const -> xmsg::XLoginRes
{
    return impl_->login_;
}
