#include "XDirServiceHandle.h"
#include <XTools.h>

XDirServiceHandle::XDirServiceHandle() = default;

XDirServiceHandle::~XDirServiceHandle() = default;

void XDirServiceHandle::dirReq(xmsg::XMsgHead *head, XMsg *msg)
{
    LOGDEBUG("XDirServiceHandle::dirReq");
    if (!head || !msg || !msg->data || msg->size <= 0)
    {
        LOGDEBUG("XDirServiceHandle::dirReq msg error!!!");
        return;
    }

    /// ���ܿͻ���ָ�� �����л�
    xmsg::XDirReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("req.ParseFromArray failed!");
        return;
    }
    std::cout << "client req path = " << req.path() << std::endl;

    /// ��Ӧ�ͻ��� ͷ����Ϣ����������·��
    xmsg::XDirRes res;

    //////////���Դ���/////////////
    for (int i = 0; i < 10; ++i)
    {
        /// ��������
        static int count = 0;
        ++count;
        std::stringstream ss;
        ss << "filename" << count << "_" << i;
        auto dir = res.add_dirs();
        dir->set_filename(ss.str());
        dir->set_filesize((i + 1) * 1024);
    }
    head->set_msgtype(xmsg::MsgType::MT_DIR_RES);
    sendMsg(head, &res);
}

void XDirServiceHandle::regMsgCallback()
{
    regCB(xmsg::MsgType::MT_DIR_REQ, static_cast<MsgCBFunc>(&XDirServiceHandle::dirReq));
}
