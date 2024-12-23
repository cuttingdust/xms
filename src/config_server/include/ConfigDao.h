/**
 * @file   ConfigDao.h
 * @brief  配置中心
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-19
 */

#ifndef CONFIGDAO_H
#define CONFIGDAO_H

#include "XMsgCom.pb.h"

#include <memory>

class ConfigDao
{
public:
    static ConfigDao *get()
    {
        static ConfigDao dao;
        return &dao;
    }

private:
    ConfigDao();
    virtual ~ConfigDao();

public:
    /// \brief 初始化数据库
    /// \param ip
    /// \param user
    /// \param pass
    /// \param db_name
    /// \param port
    /// \return
    bool init(const char *ip, const char *user, const char *pass, const char *db_name, int port = 3306);

    /// \brief
    /// \return
    bool install();

    /// \brief 保存pb配置
    /// \param conf
    /// \return
    bool saveConfig(const xmsg::XConfig *conf);

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // CONFIGDAO_H
