/**
 * @file   XLogDao.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-26
 */

#ifndef XLOGDAO_H
#define XLOGDAO_H

#include <memory>

namespace xmsg
{
    class XAddLogReq;
}

class XLogDao
{
public:
    static auto get() -> XLogDao *
    {
        static XLogDao xLog;
        return &xLog;
    }

private:
    XLogDao();
    virtual ~XLogDao();

public:
    auto init() -> bool;

    auto install() -> bool;

    auto addLog(const xmsg::XAddLogReq *req) -> bool;

private:
    class PImpl;
    std::unique_ptr<PImpl> impl_;
};


#endif // XLOGDAO_H
