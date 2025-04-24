/**
 * @file   XMsg.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-22
 */

#ifndef XMSG_H
#define XMSG_H
#include "XMsgType.pb.h"

#include <cstring>
#define MAX_MSG_SIZE         8192     /// ͷ����Ϣ������ֽ���
#define API_GATEWAY_NAME     "gw"     /// API��������
#define API_GATEWAY_PORT     20010    /// API���ض˿�
#define API_GATEWAY_SSL_PORT 20011    /// API_SSL���ض˿�
#define REGISTER_PORT        20018    /// ע�����Ķ˿�
#define CONFIG_NAME          "config" /// ������������
#define CONFIG_PORT          20019    /// �������Ķ˿�

/// ���еĺ���������
class XMsg
{
public:
    int           size     = 0;                        ///< ���ݴ�С
    xmsg::MsgType type     = xmsg::MT_NONE_DO_NOT_USE; ///< ��Ϣ����
    char         *data     = 0;                        ///< ���ݴ�ţ�protobuf�����л�������ݣ�
    int           recvSize = 0;                        ///< �Ѿ����յ����ݴ�С
public:
    bool alloc(int s)
    {
        if (s <= 0 || s > MAX_MSG_SIZE)
            return false;
        if (data)
            delete data;
        data = new char[s];
        if (!data)
            return false;
        this->size     = s;
        this->recvSize = 0;
        return true;
    }

    /// �ж������Ƿ�������
    bool recved()
    {
        if (size <= 0)
            return false;
        return (recvSize == size);
    }

    void clear()
    {
        delete data;
        memset(this, 0, sizeof(XMsg));
    }
};

#endif // XMSG_H
