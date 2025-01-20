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
    void saveConfig(xmsg::XMsgHead *head, XMsg *msg);

    void loadConfig(xmsg::XMsgHead *head, XMsg *msg);

    static void regMsgCallback();

    void loadAllConfig(xmsg::XMsgHead *head, XMsg *msg);

    void deleteConfig(xmsg::XMsgHead *head, XMsg *msg);
};


#endif // XCONFIGHANDLE_H
