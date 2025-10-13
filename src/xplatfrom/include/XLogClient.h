/**
 * @file   XLogClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-28
 */

#ifndef XLOGCLIENT_H
#define XLOGCLIENT_H

#include "XServiceClient.h"
#include "XPlatfrom_Global.h"

namespace xms
{
    XPLATFROM_EXPORT void XLog(xmsg::XLogLevel level, std::string msg, const char *filename, int line);
}

#define LOGDEBUG(msg) xms::XLog(xmsg::XLOG_DEBUG, msg, __FILE__, __LINE__)
#define LOGINFO(msg)  xms::XLog(xmsg::XLOG_INFO, msg, __FILE__, __LINE__)
#define LOGERROR(msg) xms::XLog(xmsg::XLOG_ERROR, msg, __FILE__, __LINE__)
#define LOGFATAL(msg) xms::XLog(xmsg::XLOG_FATAL, msg, __FILE__, __LINE__)

class XPLATFROM_EXPORT XLogClient : public XServiceClient
{
public:
    static auto get() -> XLogClient *;

private:
    XLogClient();
    ~XLogClient() override;

public:
    auto addLog(const xmsg::XAddLogReq *req) -> void;

    auto setLogLevel(const xmsg::XLogLevel &log_level) -> void;

    auto setOutFile(const std::string &filePath) -> void;

    auto timerCB() -> void override;

    auto startLog() -> bool;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XLOGCLIENT_H
