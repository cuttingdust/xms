/**
 * @file   XTestClient.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-25
 */

#ifndef XTESTCLIENT_H
#define XTESTCLIENT_H

#include "XServiceClient.h"

class XTestClient : public XServiceClient
{
public:
    void connectCB() override;

public:
    /// \brief
    /// \param path ����ĸ�Ŀ¼
    /// \return �Ƿ�����ɹ�������֤Ŀ¼��ȡ
    bool getDir(const std::string& path);

    /// \brief Ŀ¼������Ӧ
    /// \param head
    /// \param msg
    void dirRes(xmsg::XMsgHead* head, XMsg* msg);

    static void regMsgCallback();

    /// \brief ������ӣ��Զ�����,����ʧ�����̷��أ����������̷���
    /// \param timeout_ms ��ʱʱ��
    /// \return ���ӳɹ�����true
    bool autoConnect(int timeout_ms);
};

#endif // XTESTCLIENT_H
