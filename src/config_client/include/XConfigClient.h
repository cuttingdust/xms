/**
 * @file   XConfigClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-23
 */

#ifndef XCONFIGCLIENT_H
#define XCONFIGCLIENT_H

#include <XServiceClient.h>


class XConfigClient : public XServiceClient
{
public:
    static XConfigClient *get()
    {
        static XConfigClient instance;
        return &instance;
    }

private:
    XConfigClient();
    ~XConfigClient() override;

public:
    /// \brief ��������
    /// \param conf
    void sendConfig(xmsg::XConfig *conf);

    /// \brief ���յ��������õ���Ϣ
    /// \param head
    /// \param msg
    void sendConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief ��ȡ�������� IP���ΪNULL ��ȡ�����������ĵĵ�ַ
    /// \param ip
    /// \param port
    void loadConfig(const char *ip, int port);

    void loadConfigRes(xmsg::XMsgHead *head, XMsg *msg);

    static void regMsgCallback();

    void wait();
};


#endif // XCONFIGCLIENT_H
