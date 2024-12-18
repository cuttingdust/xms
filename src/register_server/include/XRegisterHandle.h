/**
 * @file   XRegisterHandle.h
 * @brief  ����ע�����ĵĿͻ��� ��Ӧһ������
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-17
 */

#ifndef XREGISTERHANDLE_H
#define XREGISTERHANDLE_H

#include "XServiceHandle.h"

class XRegisterHandle : public XServiceHandle
{
public:
    /// \brief ���շ����ע������
    /// \param head
    /// \param msg
    void registerReq(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief ���շ���ķ�������
    /// \param head
    /// \param msg
    void getServiceReq(xmsg::XMsgHead *head, XMsg *msg);

    static void regMsgCallback();
};


#endif // XREGISTERHANDLE_H
