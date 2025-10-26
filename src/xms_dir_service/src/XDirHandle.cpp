#include "XDirHandle.h"
#include "XDiskCom.pb.h"

#include <XTools.h>

#ifdef _WIN32
#define DIR_ROOT "./server_root/"
#else
#define DIR_ROOT "/mnt/xms/"
#endif
#define FILE_INFO_NAME_PRE ".info_"
/// 10G
#define USER_SPACE 1073741824


static std::string GetUserPath(const xmsg::XMsgHead *head)
{
    if (!head)
    {
        return "";
    }
    std::string path = DIR_ROOT;
    path += head->username();
    path += "/";
    return path;
}

XDirHandle::XDirHandle()
{
}

XDirHandle::~XDirHandle() = default;

auto XDirHandle::getDirReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xdisk::XGetDirReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XDirHandle::GetDirReq failed!");
        return;
    }
    std::cout << req.DebugString();
    std::string path = GetUserPath(head);
    path += req.root();
    std::cout << "GetDirReq path = " << path << std::endl;
    auto                 files = XTools::GetDirList(path);
    xdisk::XFileInfoList file_list;
    for (auto file : files)
    {
        if (file.file_name == "." || file.file_name == "..")
        {
            continue;
        }
        /// 文件信息文件
        std::string pre = file.file_name.substr(0, strlen(FILE_INFO_NAME_PRE));
        if (pre == FILE_INFO_NAME_PRE)
        {
            continue;
        }
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
    xdisk::XGetDirReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XDirHandle::GetDirReq failed!");
        return;
    }
    std::string path = GetUserPath(head);
    path += req.root();
    path += "/";

    XTools::NewDir(path);

    xmsg::XMessageRes res;
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_NEW_DIR_RES));
    sendMsg(head, &res);
}

auto XDirHandle::deleteFileReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    xdisk::XFileInfo req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("XDirHandle::DeleteFileReq failed!");
        return;
    }
    std::string path = GetUserPath(head);
    path += req.filedir();
    path += "/";

    std::string info_path = path;
    path += req.filename();
    info_path += FILE_INFO_NAME_PRE;
    info_path += req.filename();
    /// 删除文件 如果是秒传文件需要删除引用次数
    XTools::DelFile(path);

    /// 删除信息文件
    XTools::DelFile(info_path);

    /// 判断是否删除成功，或者是否有次文件
    head->set_msgtype(static_cast<xmsg::MsgType>(xdisk::XFMT_DELETE_FILE_RES));

    xmsg::XMessageRes res;
    res.set_return_(xmsg::XMessageRes::XR_OK);
    res.set_msg("OK");

    sendMsg(head, &res);
}

auto XDirHandle::getDiskInfoReq(xmsg::XMsgHead *head, XMsg *msg) -> void
{
    std::string        path = GetUserPath(head);
    xdisk::XDiskInfo   res;
    unsigned long long avail = 0;
    unsigned long long total = 0;
    unsigned long long free  = 0;

    /// 此操作消耗资源，后期要优化
    long long dir_size = XTools::GetDirSize(path);
    res.set_dir_size(dir_size);

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
