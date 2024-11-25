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
struct evutil_socket_t;

class XThread
{
public:
    XThread();
    virtual ~XThread();

public:
    /// \brief ��װ�̣߳���ʼ��event_base�͹ܵ������¼����ڼ���
    /// \return
    auto setup() -> bool;

    /// \brief �����߳�
    auto start() -> void;

    auto notify(intptr_t fd, short which) -> void;

    /// \brief �������
    /// \param t
    auto addTask(XTask *t) -> void;

    /// \brief �̼߳���
    auto activate() -> void;

    auto setId(int id) -> void;

    auto getId() const -> int;

    /// \brief �߳���ں���
    auto threadFun() -> void;

    /// \brief �˳��߳�
    auto exit() -> void;

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};

#endif // XTHREAD_H
