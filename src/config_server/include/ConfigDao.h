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
    auto init(const char *ip, const char *user, const char *pass, const char *db_name, int port = 3306) -> bool;

    /// \brief
    /// \return
    auto install() -> bool;

    /// \brief 保存pb配置
    /// \param conf
    /// \return
    bool saveConfig(const xmsg::XConfig *conf);


    /// \brief 读取配置
    /// \param ip
    /// \param port
    /// \return
    xmsg::XConfig loadConfig(const char *ip, int port);

    /// \brief 读取分页的配置列表
    /// \param page 从 1开始
    /// \param page_count 每页数量
    /// \return
    xmsg::XConfigList loadAllConfig(unsigned int page, int page_count);

    /// \brief 删除指定的配置
    /// \param ip
    /// \param port
    /// \return
    bool deleteConfig(const char *ip, int port);

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // CONFIGDAO_H
