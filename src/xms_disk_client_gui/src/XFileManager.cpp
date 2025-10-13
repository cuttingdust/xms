#include "XFileManager.h"

class XFileManager::PImpl
{
public:
    PImpl(XFileManager *owenr);
    ~PImpl() = default;

public:
    XFileManager *owenr_ = nullptr;
};

XFileManager::PImpl::PImpl(XFileManager *owenr) : owenr_(owenr)
{
}

XFileManager::XFileManager()
{
    impl_ = std::make_unique<XFileManager::PImpl>(this);
}

XFileManager::~XFileManager() = default;
