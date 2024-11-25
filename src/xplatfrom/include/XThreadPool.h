/**
 * @file   XThreadPool.h
 * @brief  �̳߳� 
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-05
 */

#ifndef XTHREADPOOL_H
#define XTHREADPOOL_H

#include "XPlatfrom_Global.h"
#include <memory>

class XTask;

/// �ӿ��� �̳߳�
class XPLATFROM_EXPORT XThreadPool
{
public:
    /// \brief ��ʼ�������̲߳������߳�
    /// \param threadNum
    virtual auto init(int threadNum = 4) -> void = 0;

    /// \brief �ַ��߳�
    /// \param task
    virtual auto dispatch(XTask *task) -> void = 0;

    //////////////////////////////////////////

    /// \brief �˳������߳�
    static auto exitAllThread() -> void;

    /// \brief �����ȴ�exitAllThread
    static auto wait() -> void;
};

class XPLATFROM_EXPORT XThreadPoolFactory
{
public:
    /// \brief �����̳߳ض���
    static auto create() -> XThreadPool *;
};

#endif // XTHREADPOOL_H
