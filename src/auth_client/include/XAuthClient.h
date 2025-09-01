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
    void LoginReq(std::string username, std::string password);
};


#endif // XAUTHCLIENT_H
