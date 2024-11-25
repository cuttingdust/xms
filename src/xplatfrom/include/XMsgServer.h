/**
 * @file   XMsgServer.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-21
 */

#ifndef XMSGSERVER_H
#define XMSGSERVER_H

class XMsgServer
{
public:
    XMsgServer();
    virtual ~XMsgServer();

public:
    void initServer(int serverPort);
};


#endif // XMSGSERVER_H
