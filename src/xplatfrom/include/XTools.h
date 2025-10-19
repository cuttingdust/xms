/**
 * @file   XTools.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-14
 */

#ifndef XTOOLS_H
#define XTOOLS_H

#include "XDiskCom.pb.h"
#include "XPlatfrom_Global.h"
#include "XLogClient.h"

#include <string>
#include <iostream>
#include <mutex>

namespace xmsg
{
    class XMsgHead;
}

class XMsg;

// #define LOG(level, msg) std::cout << level << ":" << __FILE__ << ":" << __LINE__ << "\n" << msg << std::endl
// #define LOGDEBUG(msg)   LOG("DEBUG", msg)
// #define LOGINFO(msg)    LOG("INFO", msg)
// #define LOGERROR(msg)   LOG("ERROR", msg)


struct XPLATFROM_EXPORT XToolFileInfo
{
    std::string file_name  = "";
    long long   file_size  = 0;
    bool        is_dir     = false;
    long long   time_write = 0;
    std::string time_str   = ""; /// 2020-03-15 20:00:15
};

class XPLATFROM_EXPORT XTools
{
public:
    static auto GetDirData(const std::string &path) -> std::string;

    static auto GetDirSize(const std::string &path) -> long long;

    static auto XMD5_base64(const unsigned char *d, unsigned long n) -> std::string;

    static auto XMD5_base64(const unsigned char *d, unsigned long n, char *md) -> char *;

    /// \brief
    ///
    /// %a 星期几的简写
    /// %A 星期几的全称
    /// %b 月分的简写
    /// %B 月份的全称
    /// %c 标准的日期的时间串
    /// %C 年份的后两位数字
    /// %d 十进制表示的每月的第几天
    /// %D 月/天/年
    /// %e 在两字符域中，十进制表示的每月的第几天
    /// %F 年-月-日
    /// %g 年份的后两位数字，使用基于周的年
    /// %G 年分，使用基于周的年
    /// %h 简写的月份名
    /// %H 24小时制的小时
    /// %I 12小时制的小时
    /// %j 十进制表示的每年的第几天
    /// %m 十进制表示的月份
    /// %M 十时制表示的分钟数
    /// %n 新行符
    /// %p 本地的AM或PM的等价显示
    /// %r 12小时的时间
    /// %R 显示小时和分钟：hh:mm
    /// %S 十进制的秒数
    /// %t 水平制表符
    /// %T 显示时分秒：hh:mm:ss
    /// %u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
    /// %U 第年的第几周，把星期日做为第一天（值从0到53）
    /// %V 每年的第几周，使用基于周的年
    /// %w 十进制表示的星期几（值从0到6，星期天为0）
    /// %W 每年的第几周，把星期一做为第一天（值从0到53）
    /// %x 标准的日期串
    /// %X 标准的时间串
    /// %y 不带世纪的十进制年份（值从0到99）
    /// %Y 带世纪部分的十制年份
    /// %z，%Z 时区名称，如果不能得到时区名称则返回空字符。
    /// %% 百分号下面的程序则显示当前的完整日期：
    ///
    /// \param timestamp
    /// \param fmt
    /// \return
    static auto XGetTime(int timestamp, std::string fmt = "%F %T") -> std::string;

    static auto PrintMsg(xmsg::XMsgHead *head, XMsg *msg);

public:
    static auto GetDirList(const std::string &path) -> std::list<XToolFileInfo>;

    static auto XGetIconFilename(const std::string &filename, bool is_dir) -> std::string;

    static auto GetSizeString(long long size) -> std::string;

    static auto NewDir(const std::string &path) -> void;

    static auto DelFile(const std::string &path) -> void;

    static auto GetDiskSize(const char *dir, unsigned long long *avail, unsigned long long *total,
                            unsigned long long *free) -> bool;

public:
    /// \brief
    /// windows C:\Windows\System32\drivers\etc
    /// Linux /etc/hosts
    /// 127.0.0.1 xms_gateway_server
    /// 127.0.0.1 xms_register_server
    /// \param host_name
    /// \return
    static auto XGetHostByName(const std::string &host_name) -> std::string;

    static auto XGetPortByName(const std::string &host_name) -> int;

    static auto XGetNameByPort(unsigned short port) -> const char *;
};

class XPLATFROM_EXPORT XMutex final
{
public:
    explicit XMutex(std::mutex *mux);
    ~XMutex();

private:
    std::mutex *mux_ = nullptr;
};

#endif // XTOOLS_H
