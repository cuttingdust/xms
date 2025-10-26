/**
 * @file   XConfigHandle.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-23
 */

#ifndef XCONFIGHANDLE_H
#define XCONFIGHANDLE_H

#include <XServiceHandle.h>

class XConfigHandle : public XServiceHandle
{
public:
    XConfigHandle();
    ~XConfigHandle() override;

public:
    auto saveConfig(xmsg::XMsgHead *head, XMsg *msg) -> void;

    auto loadConfig(xmsg::XMsgHead *head, XMsg *msg) -> void;

    static auto regMsgCallback() -> void;

    /// \brief 下载全部配置（有分页）
    /// \param head
    /// \param msg
    auto loadAllConfig(xmsg::XMsgHead *head, XMsg *msg) -> void;

    /// \brief 删除配置
    /// \param head
    /// \param msg
    auto deleteConfig(xmsg::XMsgHead *head, XMsg *msg) -> void;
};


#endif // XCONFIGHANDLE_H
