/**
 * @file   XDirServiceHandle.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-26
 */

#ifndef XDIRSERVICEHANDLE_H
#define XDIRSERVICEHANDLE_H
#include <XServiceHandle.h>

class XDirServiceHandle : public XServiceHandle
{
public:
    XDirServiceHandle();
    ~XDirServiceHandle() override;

    ///�����û���Ŀ¼����
    void dirReq(xmsg::XMsgHead *head, XMsg *msg);

    /// \brief ע����Ϣ�ص�����
    static void regMsgCallback();
};


#endif // XDIRSERVICEHANDLE_H
