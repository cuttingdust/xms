/**
 * @file   XRegisterClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-17
 */

#ifndef XREGISTERCLIENT_H
#define XREGISTERCLIENT_H

#include "XPlatfrom_Global.h"
#include "XServiceClient.h"

class XPLATFROM_EXPORT XRegisterClient : public XServiceClient
{
public:
    static XRegisterClient *get()
    {
        /// TODO �ڲ�����ָ��������Ϊ
        static std::once_flag   s_flag;
        static XRegisterClient *r = nullptr;
        std::call_once(s_flag,
                       [&]()
                       {
                           if (!r)
                               r = new XRegisterClient;
                       });
        return r;
    }

private:
    XRegisterClient();
    ~XRegisterClient() override;

public:
    void connectCB() override;

public:
    /// \brief ��ע������ע����� �˺�������Ҫ��һ�����ã���������
    /// \param service_name  ΢��������
    /// \param port          ΢����ӿ�
    /// \param ip            ΢����IP �������NULL������ÿͻ������ӵ�ַ
    void registerServer(const char *service_name, int port, const char *ip);

    /// \brief ���շ����ע����Ӧ
    /// \param head
    /// \param msg
    void registerRes(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief �����л�ȡ΢�����б������
    /// \param service_name service_name == NULL ��ȡȫ��
    void getServiceReq(const char *service_name);

    /// \brief ��ȡ�����б����Ӧ
    /// \param head
    /// \param msg
    void getServiceRes(xmsg::XMsgHead *head, XMsg *msg);


    /// \brief   ��ȡ���еķ����б�����ԭ���ݣ�ÿ�������ϴεĸ�������
    /// \return  �˺����Ͳ���XServiceMap���ݵĺ�����һ���߳�
    xmsg::XServiceMap *getAllService() const;

    void regMsgCallback();

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XREGISTERCLIENT_H
