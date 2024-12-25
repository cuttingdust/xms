/**
 * @file   ConfigDao.h
 * @brief  ��������
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
    /// \brief ��ʼ�����ݿ�
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

    /// \brief ����pb����
    /// \param conf
    /// \return
    bool saveConfig(const xmsg::XConfig *conf);


    /// \brief ��ȡ����
    /// \param ip
    /// \param port
    /// \return
    xmsg::XConfig loadConfig(const char *ip, int port);

    /// \brief ��ȡ��ҳ�������б�
    /// \param page �� 1��ʼ
    /// \param page_count ÿҳ����
    /// \return
    xmsg::XConfigList loadAllConfig(unsigned int page, int page_count);

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // CONFIGDAO_H
