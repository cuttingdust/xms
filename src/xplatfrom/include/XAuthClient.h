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

#include "XPlatfrom_Global.h"
#include <XServiceClient.h>

class XPLATFROM_EXPORT XAuthClient : public XServiceClient
{
public:
    static auto get() -> XAuthClient *;
    ~XAuthClient() override;

public:
    auto login(std::string username, std::string password) -> bool;

    /// \brief 登录
    /// \param username 用户名
    /// \param password 密码（明文),在函数中会经过md5_base64编码后发送
    auto loginReq(std::string username, std::string password) -> void;

    /// \brief 检查token 是否有效 更新本地的 login登录数据
    /// \param token
    auto checkTokenReq(std::string token) -> void;

    /// \brief 添加用户消息
    /// \param user
    auto addUserReq(xmsg::XAddUserReq *user) -> void;

    /// \brief 修改密码消息
    /// \param pass
    auto changePasswordReq(const xmsg::XChangePasswordReq *pass) -> void;

    /// \brief 检验登录
    /// \param username
    /// \param out_info
    /// \param timeout_ms 检测的超时时间，但如果连接异常则立刻返回
    /// 如果token快要过期，会自动发送更新请求
    ///
    /// \return 失败返回错误到 token 中
    auto getLoginInfo(std::string username, xmsg::XLoginRes *out_info, int timeout_ms = 200) -> bool;

    auto getLogin() -> xmsg::XLoginRes;

    /// \brief 当前登录的用户名
    /// \return
    auto cur_user_name() const -> std::string;

    static auto regMsgCallback() -> void;

private:
    XAuthClient();

    /// \brief 接收登录消息
    /// \param head
    /// \param msg
    auto loginRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    /// \brief 接收添加用户消息
    /// \param head
    /// \param msg
    auto addUserRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    /// \brief 接收修改密码消息
    /// \param head
    /// \param msg
    auto changePasswordRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto checkTokenRes(xmsg::XMsgHead *head, XMsg *msg) -> void;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XAUTHCLIENT_H
