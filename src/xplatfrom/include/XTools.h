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

#include "XPlatfrom_Global.h"

#include <string>
#include <iostream>
#include <mutex>

#define LOG(level, msg) std::cout << level << ":" << __FILE__ << ":" << __LINE__ << "\n" << msg << std::endl
#define LOGDEBUG(msg)   LOG("DEBUG", msg)
#define LOGINFO(msg)    LOG("INFO", msg)
#define LOGERROR(msg)   LOG("ERROR", msg)


class XPLATFROM_EXPORT XTools
{
public:
    static std::string getDirData(std::string path);

    static std::string XMD5_base64(const unsigned char *d, unsigned long n);

    static char *XMD5_base64(const unsigned char *d, unsigned long n, char *md);
};

class XPLATFROM_EXPORT XMutex final
{
public:
    explicit XMutex(std::mutex *mux)
    {
        mux_ = mux;
        mux_->lock();
    }
    ~XMutex()
    {
        mux_->unlock();
    }

private:
    std::mutex *mux_ = nullptr;
};

#endif // XTOOLS_H
