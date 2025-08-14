/**
 * @file   XRouteServer.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-11-28
 */

#ifndef XROUTESERVER_H
#define XROUTESERVER_H

#include <XService.h>

class XRouteServer : public XService
{
public:
    XRouteServer();
    ~XRouteServer() override;

public:
    XServiceHandle *createHandle() override;
};


#endif // XROUTESERVER_H
