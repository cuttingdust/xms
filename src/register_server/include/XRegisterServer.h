/**
 * @file   XRegisterServer.h
 * @brief  ע�����ķ����
 *
 * @details   
 *
 * @author 31667
 * @date   2024-12-17
 */

#ifndef XREGISTERSERVER_H
#define XREGISTERSERVER_H

#include <XService.h>

class XRegisterServer : public XService
{
public:
    /// ���ݲ��� ��ʼ��������Ҫ�ȵ���
    void main(int argc, char *argv[]);

    /// �ȴ��߳��˳�
    void wait();

    XServiceHandle *createHandle() override;
};


#endif // XREGISTERSERVER_H
