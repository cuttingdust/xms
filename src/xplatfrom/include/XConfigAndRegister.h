/**
 * @file   XConfigAndRegister.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-10-25
 */

#ifndef XCONFIGANDREGISTER_H
#define XCONFIGANDREGISTER_H

#include "XPlatfrom_Global.h"

namespace google::protobuf
{
    class Message;
}

class XPLATFROM_EXPORT XConfigAndRegister
{
public:
    static auto init(const char *service_name, const char *service_ip, int service_port, const char *register_ip,
                     int register_port, google::protobuf::Message *conf_message) -> bool;
};


#endif // XCONFIGANDREGISTER_H
