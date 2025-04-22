/**
 * @file   XMySSLService.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-04-22
 */

#ifndef XMYSSLSERVICE_H
#define XMYSSLSERVICE_H

#include <XService.h>

class XMySSLService : public XService
{
public:
    XMySSLService();
    ~XMySSLService() override;

public:
    XServiceHandle *createHandle() override;
};


#endif // XMYSSLSERVICE_H
