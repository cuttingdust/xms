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
    /// \brief ��ʼ��bufferevent���ͻ��˽�������
    /// \return
    auto init() -> bool override;

    /// \brief ��ʼ���ӷ����������ó�Ա server_ip_ server_port_
    /// �����Զ�����
    /// \return
    auto connect() const -> bool;

    // protected:
    /// \brief �Ƿ�������
    /// \return
    auto isConnected() const -> bool;

    /// \brief �Ƿ���������
    /// \return
    auto isConnecting() const -> bool;

public:
    /// \brief ���÷�����ip
    /// \param ip
    void        setServerIp(const char* ip);
    const char* getServerIp() const;

    /// \brief ���÷������˿�
    /// \param port
    void setServerPort(int port);
    int  getServerPort() const;

    void setServerRoot(const std::string path);

    void setIsRecvMsg(bool isRecvMsg);

    void setAutoDelete(bool bAuto);

    /// \brief �ȴ����ӳɹ�
    /// \param timeout_sec ���ȴ�ʱ��
    bool waitConnected(int timeout_sec);

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
