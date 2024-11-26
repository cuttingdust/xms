#include "XTestClient.h"

#include <XTools.h>

#include <thread>


void XTestClient::readCB()
{
    LOGDEBUG("XTestClient::readCB");
}

void XTestClient::connectCB()
{
    LOGDEBUG("XTestClient::connectCB()");
}

bool XTestClient::autoConnect(int timeout_ms)
{
    LOGDEBUG("XTestClient::autoConnect");
    /// 1 ������
    if (isConnected())
        return true;
    /// 2 δ���� Ҳ����������
    if (!isConnecting())
    {
        /// ��ʼ����
        if (!connect())
            return false;
    }

    int count = timeout_ms / 10;
    /// ������
    for (int i = 0; i < count; i++)
    {
        if (isConnected())
            return true;
        if (!isConnecting())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return isConnected();
}
