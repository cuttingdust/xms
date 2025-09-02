/**
 * @file   XAuthClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-01
 */

#ifndef XAUTHCLIENT_H
#define XAUTHCLIENT_H

#include <XServiceClient.h>

class XAuthClient : public XServiceClient
{
public:
    XAuthClient();
    ~XAuthClient() override;

public:
    /// \brief 登录
    /// \param username 用户名
    /// \param password 密码（明文),在函数中会经过md5_base64编码后发送
    auto loginReq(std::string username, std::string password) -> void;

    auto addUserReq(xmsg::XAddUserReq *user) -> void;

    auto getLoginInfo(std::string username, xmsg::XLoginRes *out_info, int timeout_ms) -> bool;

    static auto regMsgCallback() -> void;

private:
    void addUserRes(xmsg::XMsgHead *head, XMsg *msg);

    void loginRes(xmsg::XMsgHead *head, XMsg *msg);

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XAUTHCLIENT_H
