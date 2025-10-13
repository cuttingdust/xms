/**
 * @file   XDirService.cpp
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-13
 */

#ifndef XDIRSERVICE_CPP
#define XDIRSERVICE_CPP

#include <XService.h>

class XDirService : public XService
{
public:
    XDirService();
    ~XDirService() override;

public:
    auto createHandle() -> XServiceHandle * override;
};


#endif // XDIRSERVICE_CPP
