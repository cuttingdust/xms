/**
 * @file   XMySSLClient.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-04-22
 */

#ifndef XMYSSLCLIENT_H
#define XMYSSLCLIENT_H

#include <XServiceClient.h>

class XMySSLClient : public XServiceClient
{
public:
    XMySSLClient();
    virtual ~XMySSLClient();

public:
    void connectCB() override;
};


#endif // XMYSSLCLIENT_H
