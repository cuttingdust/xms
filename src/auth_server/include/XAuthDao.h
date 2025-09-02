/**
 * @file   XAuthDao.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-01
 */

#ifndef XAUTHDAO_H
#define XAUTHDAO_H

#include <memory>

namespace xmsg
{
    class XLoginReq;
    class XLoginRes;
    class XAddUserReq;
} // namespace xmsg

class XAuthDao
{
public:
    static XAuthDao *get();
    virtual ~XAuthDao();

    /// 初始化数据库，准备好用户名密码
    auto init() -> bool;

    /// \brief 安装表
    /// \return
    auto install() -> bool;

    /// \brief 添加用户
    /// \param user
    /// \return
    auto addUser(xmsg::XAddUserReq *user) -> bool;

    /// \brief 登录
    /// \param user_req
    /// \param user_res
    /// \param timeout_sec
    /// \return
    auto login(const xmsg::XLoginReq *user_req, xmsg::XLoginRes *user_res, int timeout_sec) -> bool;

private:
    XAuthDao();

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XAUTHDAO_H
