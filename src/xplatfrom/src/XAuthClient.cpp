#include "XAuthClient.h"

#include <utility>

#include "XTools.h"

class XAuthClient::PImpl
{
public:
    PImpl(XAuthClient *owenr);
    ~PImpl() = default;

public:
    XAuthClient *owenr_ = nullptr;

    std::map<std::string, xmsg::XLoginRes>
                login_map_; ///< 登录数据（包含token） key是用户名 同一个用户支持多个客户端登录
    std::mutex  logins_mutex_;
    std::string cur_user_name_; ///<  当前登录的用户名
};

XAuthClient::PImpl::PImpl(XAuthClient *owenr) : owenr_(owenr)
{
    owenr_->regMsgCallback();
    owenr_->setServiceName(AUTH_NAME);
}

auto XAuthClient::get() -> XAuthClient *
{
    static XAuthClient xc;
    return &xc;
}

XAuthClient::XAuthClient()
{
    impl_ = std::make_unique<PImpl>(this);
}

XAuthClient::~XAuthClient() = default;

auto XAuthClient::login(std::string username, std::string password) -> bool
{
    loginReq(std::move(username), std::move(password));

    auto res = getLogin();
    if (res.restype() == xmsg::XLoginRes::XRT_ERR)
    {
        return false;
    }

    return true;
}

auto XAuthClient::loginReq(std::string username, std::string password) -> void
{
    impl_->cur_user_name_ = username;

    xmsg::XLoginReq req;
    req.set_username(username);
    auto md5_pass = XTools::XMD5_base64(reinterpret_cast<unsigned char *>(password.data()), password.size());
    req.set_password(md5_pass);
    std::cout << req.DebugString();
    {
        XMUTEX(&impl_->logins_mutex_);
        impl_->login_map_.erase(username);
    }

    sendMsg(xmsg::MT_LOGIN_REQ, &req);
}

auto XAuthClient::checkTokenReq(std::string token) -> void
{
}

auto XAuthClient::addUserReq(xmsg::XAddUserReq *user) -> void
{
    if (!user)
    {
        return;
    }
    xmsg::XAddUserReq req;
    req.CopyFrom(*user);
    std::string pass = user->password();

    auto pass_md = XTools::XMD5_base64((unsigned char *)user->password().c_str(), user->password().size());
    user->set_password(pass_md);

    static int i = 0;
    i++;
    sendMsg(xmsg::MT_ADD_USER_REQ, user);

    std::cout << i << "XAuthClient::Get()->AddUserReq(&req);" << std::endl;
}

auto XAuthClient::changePasswordReq(const xmsg::XChangePasswordReq *pass) -> void
{
    sendMsg(xmsg::MT_CHANGE_PASSWORD_REQ, pass);
}

auto XAuthClient::getLoginInfo(std::string username, xmsg::XLoginRes *out_info, int timeout_ms) -> bool
{
    if (!out_info)
    {
        return false;
    }

    int count = timeout_ms / 10;
    if (count <= 0)
    {
        count = 1;
    }

    int i = 0;
    for (; i < count; i++)
    {
        impl_->logins_mutex_.lock();
        std::cout << impl_->login_map_.size() << ";" << std::flush;
        auto login_ptr = impl_->login_map_.find(username);
        if (login_ptr == impl_->login_map_.end())
        {
            impl_->logins_mutex_.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        auto login = login_ptr->second;
        impl_->logins_mutex_.unlock();
        if (login.restype() != xmsg::XLoginRes::XRT_OK)
        {
            return false;
        }
        out_info->CopyFrom(login);
        return true;
    }
    return false;
}

auto XAuthClient::getLogin() -> xmsg::XLoginRes
{
    xmsg::XLoginRes res;

    if (!getLoginInfo(impl_->cur_user_name_, &res, 3000))
    {
        res.set_restype(xmsg::XLoginRes::XRT_ERR);
        return res;
    }
    return res;
}

auto XAuthClient::cur_user_name() const -> std::string
{
    return impl_->cur_user_name_;
}

auto XAuthClient::regMsgCallback() -> void
{
    regCB(xmsg::MT_LOGIN_RES, static_cast<MsgCBFunc>(&XAuthClient::loginRes));
    regCB(xmsg::MT_ADD_USER_RES, static_cast<MsgCBFunc>(&XAuthClient::addUserRes));
    regCB(xmsg::MT_CHANGE_PASSWORD_RES, static_cast<MsgCBFunc>(&XAuthClient::changePasswordRes));
    regCB(xmsg::MT_CHECK_TOKEN_RES, static_cast<MsgCBFunc>(&XAuthClient::checkTokenRes));
}

auto XAuthClient::addUserRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("AddUser failed!ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_ERROR)
    {
        LOGDEBUG(res.msg());
        LOGDEBUG("AddUser failed!");
        return;
    }
    LOGDEBUG("AddUser success!");
    return;
}

auto XAuthClient::changePasswordRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xmsg::XMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("ChangePassword failed!ParseFromArray failed!");
        return;
    }
    if (res.return_() == xmsg::XMessageRes::XR_ERROR)
    {
        LOGDEBUG(res.msg());
        LOGDEBUG("ChangePasswordRes failed!");
        return;
    }
    LOGDEBUG("ChangePasswordRes success!");
    return;
}

auto XAuthClient::checkTokenRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
}

auto XAuthClient::loginRes(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    static int count;
    count++;
    std::cout << "LoginRes" << count << std::flush;
    xmsg::XLoginRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        return;
    }
    LOGINFO(res.DebugString());
    LOGINFO(head->DebugString());
    {
        std::cout << "begin XMUTEX(&logins_mutex_)" << std::endl;
        XMUTEX(&impl_->logins_mutex_);
        std::cout << "end XMUTEX(&logins_mutex_)" << std::endl;
        if (!res.username().empty())
        {
            impl_->login_map_[res.username()] = res;
            this->setLogin(&res);
        }
    }
}
