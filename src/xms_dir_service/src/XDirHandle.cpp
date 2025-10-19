#include "XDirHandle.h"
#include "XDiskCom.pb.h"

#include <XTools.h>

#ifdef _WIN32
#define DIR_ROOT "./server_root/"
#else
#define DIR_ROOT "/root/xms/";
#endif

/// 10G
#define USER_SPACE 1073741824

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
    path += head->username();
    // path += "root";
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

auto XDirHandle::newDirReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::string path = DIR_ROOT;
    path += head->username();
    //path += "root";
    path += "/";
    xdisk::XGetDirReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XDirHandle::GetDirReq failed!");
        return;
    }
    path += req.root();

    XTools::NewDir(path);

    xmsg::XMessageRes res;
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_NEW_DIR_RES));
    sendMsg(head, &res);
}

auto XDirHandle::deleteFileReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    /// 根目录 + 用户名 + 相对目录
    std::string path = DIR_ROOT;
    path += head->username();
    //path += "root";
    path += "/";

    xdisk::XFileInfo req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XDirHandle::DeleteFileReq failed!");
        return;
    }
    if (!req.filedir().empty())
    {
        path += req.filedir();
        path += "/";
    }

    path += req.filename();
    XTools::DelFile(path);

    xmsg::XMessageRes res;
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_DELETE_FILE_RES));
    sendMsg(head, &res);
}

auto XDirHandle::getDiskInfoReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    /// 根目录 + 用户名 + 相对目录
    std::string path = DIR_ROOT;
    path += head->username();
    auto             dir_size = XTools::GetDirSize(path);
    xdisk::XDiskInfo res;
    res.set_dir_size(dir_size);
    unsigned long long avail = 0;
    unsigned long long total = 0;
    unsigned long long free  = 0;

    if (head->username() == "root")
    {
        XTools::GetDiskSize(path.c_str(), &avail, &total, &free);
        res.set_avail(avail);
        res.set_free(free);
        res.set_total(total);
    }
    else
    {
        long long user_size = USER_SPACE;
        res.set_avail(user_size - dir_size);
        res.set_free(user_size - dir_size);
        res.set_total(user_size);
    }
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DISK_INFO_RES));
    sendMsg(head, &res);
}

auto XDirHandle::regMsgCallback() -> void
{
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DIR_REQ), static_cast<MsgCBFunc>(&XDirHandle::getDirReq));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_NEW_DIR_REQ), static_cast<MsgCBFunc>(&XDirHandle::newDirReq));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_DELETE_FILE_REQ), static_cast<MsgCBFunc>(&XDirHandle::deleteFileReq));
    regCB(static_cast<xmsg::MsgType>(xdisk::XFMT_GET_DISK_INFO_REQ),
          static_cast<MsgCBFunc>(&XDirHandle::getDiskInfoReq));
}
