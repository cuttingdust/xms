/**
 * @file   XConfigServer.h
 * @brief  
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-23
 */

#ifndef XCONFIGSERVER_H
#define XCONFIGSERVER_H

#include <XService.h>

class XConfigServer : public XService
{
public:
    XConfigServer();
    ~XConfigServer() override;

public:
    void            main(int argc, char *argv[]);
    XServiceHandle *createHandle() override;
    void            wait();
};


#endif // XCONFIGSERVER_H
