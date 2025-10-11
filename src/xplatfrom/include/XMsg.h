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

//////////////////////////////////////////////////////////////////
#define API_GATEWAY_NAME        "gw"  /// API网关名称
#define API_GATEWAY_PORT        20010 /// API网关端口
#define API_GATEWAY_SSL_PORT    20011 /// API_SSL网关端口
#define API_GATEWAY_SERVER_NAME "xms_gateway_server"
//////////////////////////////////////////////////////////////////
#define REGISTER_PORT        20018 /// 注册中心端口
#define REGISTER_NAME        "reg"
#define REGISTER_SERVER_NAME "xms_register_server"

#define XLOG_NAME "xlog"
#define XLOG_PORT 20030

#define CONFIG_NAME "config" /// 配置中心名称
#define CONFIG_PORT 20019    /// 配置中心端口

#define AUTH_PORT 20020
#define AUTH_NAME "auth"

#define UPLOAD_PORT   20100
#define UPLOAD_NAME   "upload"
#define DOWNLOAD_PORT 20200
#define DOWNLOAD_NAME "download"
#define DIR_PORT      20300
#define DIR_NAME      "dir"

#define MAX_MSG_SIZE 8192 /// 头部消息的最大字节数

/// 所有的函数做内联
class XMsg
{
public:
    int           size     = 0;                        ///< 数据大小
    xmsg::MsgType type     = xmsg::MT_NONE_DO_NOT_USE; ///< 消息类型
    char         *data     = 0;                        ///< 数据存放（protobuf的序列化后的数据）
    int           recvSize = 0;                        ///< 已经接收的数据大小
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
        if (size < 0)
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
