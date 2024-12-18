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

#include "XServiceClient.h"

class XRegisterClient : public XServiceClient
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

    void registerRes(xmsg::XMsgHead *head, XMsg *msg);

    void regMsgCallback();

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XREGISTERCLIENT_H
