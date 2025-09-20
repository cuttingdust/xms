/**
 * @file   XServiceHandle.h
 * @brief
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-25
 */

#ifndef XSERVICEHANDLE_H
#define XSERVICEHANDLE_H

#include "XPlatfrom_Global.h"
#include "XMsgEvent.h"

class XPLATFROM_EXPORT XServiceHandle : public XMsgEvent
{
public:
    XServiceHandle();
    ~XServiceHandle() override;
};


#endif // XSERVICEHANDLE_H
