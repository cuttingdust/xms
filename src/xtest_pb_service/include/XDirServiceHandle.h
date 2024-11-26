/**
 * @file   XDirServiceHandle.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-26
 */

#ifndef XDIRSERVICEHANDLE_H
#define XDIRSERVICEHANDLE_H
#include <XServiceHandle.h>

class XDirServiceHandle : public XServiceHandle
{
public:
    XDirServiceHandle();
    ~XDirServiceHandle() override;

public:
    void readCB() override;
};


#endif // XDIRSERVICEHANDLE_H
