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

#include "XMsgType.pb.h"
#include "XMsg.h"
#include "XComTask.h"

#include <memory>

/// ������bufferevent�ӿڣ�ֱ�ӵ���XComTask�ķ�װ
class XMsgEvent : public XComTask
{
public:
    XMsgEvent();
    ~XMsgEvent() override;

public:
    /// \brief �������ݰ���
    /// 1 ��ȷ���յ���Ϣ  (������Ϣ��������)
    /// 2 ��Ϣ���ղ����� (�ȴ���һ�ν���)
    /// 3 ��Ϣ���ճ��� ���˳������ռ䣩
    /// \return  1 2 ����true 3����false
    auto recvMsg() -> bool;

    /// \brief ��ȡ���յ������ݰ�����������ͷ����Ϣ��,
    /// �ɵ���������XMsg
    /// \return ���û�����������ݰ�������NULL
    auto getMsg() const -> XMsg *;

    /// \brief ������Ϣ ����ͷ�����Զ�������
    /// \param msgType  ��Ϣ����
    /// \param msg      ��Ϣ����
    auto sendMsg(const xmsg::MsgType &msgType, const google::protobuf::Message *msg) -> void;

    /// \brief ����������Ϣͷ����Ϣ���ݣ����ڽ�����һ����Ϣ
    auto clear() -> void;

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};


#endif // XMSGEVENT_H