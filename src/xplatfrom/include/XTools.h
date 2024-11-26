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

#define LOG(level, msg) (std::cout << level << ":" << __FILE__ << ":" << __LINE__ << "\n" << msg << std::endl)
#define LOGDEBUG(msg)   LOG("DEBUG", msg)
#define LOGINFO(msg)    LOG("INFO", msg)
#define LOGERROR(msg)   LOG("ERROR", msg)


class XPLATFROM_EXPORT XTools
{
public:
    static std::string getDirData(std::string path);
};

#endif // XTOOLS_H
