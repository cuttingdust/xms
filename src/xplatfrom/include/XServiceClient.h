/**
 * @file   XServiceClient.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-25
 */

#ifndef XSERVICECLIENT_H
#define XSERVICECLIENT_H

#include "XPlatfrom_Global.h"
#include "XMsgEvent.h"

class XPLATFROM_EXPORT XServiceClient : public XMsgEvent
{
public:
    XServiceClient();
    ~XServiceClient() override;

public:
    /// \brief 将任务加入到线程池中，进行连接
    virtual void startConnect();

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};


#endif // XSERVICECLIENT_H
