/**
 * @file   XLogServer.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2025-09-28
 */

#ifndef XLOGSERVER_H
#define XLOGSERVER_H

#include <XService.h>

class XLogServer : public XService
{
public:
    XLogServer();
    ~XLogServer() override;

public:
    auto main(int argc, char *argv[]) -> void;
    auto createHandle() -> XServiceHandle * override;
};


#endif // XLOGSERVER_H
