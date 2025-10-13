#include "XDirHandle.h"
#include "XDiskCom.pb.h"

#include <XTools.h>

#ifdef _WIN32
#define DIR_ROOT "./server_root/"
#else
#define DIR_ROOT "/root/xms/";
#endif

XDirHandle::XDirHandle()
{
}

XDirHandle::~XDirHandle()
{
}

auto XDirHandle::getDirReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    /// 根目录 + 用户名 + 相对目录
    std::string path = DIR_ROOT;
    //path += head->username();
    path += "root";
    path += "/";
    xdisk::XGetDirReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XDirHandle::GetDirReq failed!");
        return;
    }
    path += req.root();
    auto                 files = XTools::GetDirList(path);
    xdisk::XFileInfoList file_list;
    for (auto file : files)
    {
        if (file.file_name == "." || file.file_name == "..")
            continue;
        auto info = file_list.add_files();
        info->set_filename(file.file_name);
        info->set_filesize(file.file_size);
        info->set_filetime(file.time_str);
        info->set_is_dir(file.is_dir);
    }
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_RES));
    sendMsg(head, &file_list);
}

auto XDirHandle::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_REQ), static_cast<MsgCBFunc>(&XDirHandle::getDirReq));
}
