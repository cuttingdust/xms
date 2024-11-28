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
#include "XPlatfrom_Global.h"
#include "XMsgType.pb.h"
#include "XMsgCom.pb.h"

#include <cstring>
#define MAX_MSG_SIZE 8192 /// 头部消息的最大字节数


/// 所有的函数做内联
class XMsg
{
public:
    int           size     = 0;                     ///< 数据大小
    xmsg::MsgType type     = xmsg::NONE_DO_NOT_USE; ///< 消息类型
    char         *data     = 0;                     ///< 数据存放（protobuf的序列化后的数据）
    int           recvSize = 0;                     ///< 已经接收的数据大小
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

    /// 判断数据是否接收完成
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
