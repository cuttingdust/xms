/**
 * @file   XTestClient.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-25
 */

#ifndef XTESTCLIENT_H
#define XTESTCLIENT_H

#include "XServiceClient.h"

class XTestClient : public XServiceClient
{
public:
    void readCB() override;
    void connectCB() override;
};

#endif // XTESTCLIENT_H
