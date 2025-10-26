/**
 * @file   XThread.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-05
 */

#ifndef XTHREAD_H
#define XTHREAD_H

#include <memory>
class XTask;

class XThread
{
public:
    XThread();
    virtual ~XThread();

public:
    /// \brief 安装线程，初始化event_base和管道监听事件用于激活
    /// \return
    auto setup() -> bool;

    /// \brief 启动线程
    auto start() -> void;

    auto notify(intptr_t fd, short which) -> void;

    /// \brief 添加任务
    /// \param t
    auto addTask(XTask *t) -> void;

    /// \brief 线程激活
    auto activate() -> void;

    auto setId(int id) -> void;

    auto getId() const -> int;

    /// \brief 线程入口函数
    auto threadFun() -> void;

    /// \brief 退出线程
    auto exit() -> void;

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};

#endif // XTHREAD_H
