/**
 * @file   XComTask.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-14
 */

#ifndef XCOMTASK_H
#define XCOMTASK_H

#include "XPlatfrom_Global.h"
#include "XTask.h"
#include "XMsg.h"

#include <memory>
#include <string>

class XPLATFROM_EXPORT XComTask : public XTask
{
public:
    XComTask();
    virtual ~XComTask();

public:
    auto init() -> bool override;

    void setServerIp(const char* ip);

    void setServerPort(int port);

    void setServerRoot(const std::string path);

    void setIsRecvMsg(bool isRecvMsg);

public:
    virtual void eventCB(short events);

    virtual void connectCB();

    virtual void readCB() = 0;

    virtual int read(void* data, int size);

    virtual void writeCB();

    virtual bool write(const void* data, int size);

    /// \brief ����д��ص�
    virtual void beginWriteCB();

    virtual void close();

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};

#endif // XCOMTASK_H
