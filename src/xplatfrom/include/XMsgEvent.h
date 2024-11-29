/**
 * @file   XMsgEvent.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-22
 */

#ifndef XMSGEVENT_H
#define XMSGEVENT_H

#include "XPlatfrom_Global.h"

#include "XMsgType.pb.h"
#include "XMsg.h"
#include "XComTask.h"
#include "XMsgCom.pb.h"

#include <memory>

/// ������bufferevent�ӿڣ�ֱ�ӵ���XComTask�ķ�װ
class XPLATFROM_EXPORT XMsgEvent : public XComTask
{
public:
    XMsgEvent();
    ~XMsgEvent() override;
    typedef void (XMsgEvent::*MsgCBFunc)(xmsg::XMsgHead *head, XMsg *msg);

public:
    /// \brief ������Ϣ �ַ���Ϣ
    void readCB() override;

    virtual void readCB(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief �����Ϣ����Ļص�������������Ϣ���ͷַ� ,ͬһ������ֻ����һ���ص�����
    /// \param type ��Ϣ����
    /// \param func ��Ϣ�ص�����
    static void regCB(const xmsg::MsgType &type, MsgCBFunc func);

public:
    /// \brief �������ݰ���
    /// 1 ��ȷ���յ���Ϣ  (������Ϣ������)
    /// 2 ��Ϣ���ղ����� (�ȴ���һ�ν���)
    /// 3 ��Ϣ���ճ��� ���˳�����ռ䣩
    /// \return  1 2 ����true 3����false
    auto recvMsg() -> bool;

    /// \brief ��ȡ���յ������ݰ�����������ͷ����Ϣ��,
    /// �ɵ���������XMsg
    /// \return ���û�����������ݰ�������NULL
    auto getMsg() const -> XMsg *;

    virtual auto sendMsg(xmsg::XMsgHead *head, XMsg *msg) -> bool;

    virtual auto sendMsg(xmsg::XMsgHead *head, const google::protobuf::Message *msg) -> bool;

    /// \brief ������Ϣ ����ͷ�����Զ�������
    /// \param msgType  ��Ϣ����
    /// \param msg      ��Ϣ����
    virtual auto sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg) -> bool;


    /// \brief ��������Ϣͷ����Ϣ���ݣ����ڽ�����һ����Ϣ
    auto clear() -> void;

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};


#endif // XMSGEVENT_H
