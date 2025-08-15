/**
 * @file   XThreadPool.h
 * @brief  线程池 
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

/// 接口类 线程池
class XPLATFROM_EXPORT XThreadPool
{
public:
    XThreadPool();
    virtual ~XThreadPool();

public:
    /// \brief 初始化所有线程并启动线程
    /// \param threadNum
    virtual auto init(int threadNum = 4) -> void = 0;

    /// \brief 分发线程
    /// \param task
    virtual auto dispatch(XTask *task) -> void = 0;

    //////////////////////////////////////////

    /// \brief 退出所有线程
    static auto exitAllThread() -> void;

    /// \brief 阻塞等待exitAllThread
    static auto wait() -> void;
};

class XPLATFROM_EXPORT XThreadPoolFactory
{
public:
    /// \brief 创建线程池对象
    static auto create() -> XThreadPool *;
};

#endif // XTHREADPOOL_H
