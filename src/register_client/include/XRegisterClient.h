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
        static XRegisterClient r;
        return &r;
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

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XREGISTERCLIENT_H
