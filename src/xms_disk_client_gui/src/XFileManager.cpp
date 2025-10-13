#include "XFileManager.h"

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
