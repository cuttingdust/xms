/**
 * @file   XAuthServer.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-01
 */

#ifndef XAUTHSERVER_H
#define XAUTHSERVER_H

#include <XService.h>

class XAuthServer : public XService
{
public:
    XAuthServer();
    ~XAuthServer() override;

public:
    auto createHandle() -> XServiceHandle * override;
};


#endif // XAUTHSERVER_H
