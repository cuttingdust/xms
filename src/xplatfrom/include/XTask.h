/**
 * @file   XTask.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-12
 */

#ifndef XTASK_H
#define XTASK_H

class XTask
{
public:
    virtual auto init() -> bool = 0;

    auto base() const -> struct event_base *
    {
        return base_;
    }
    auto set_base(struct event_base *base) -> void
    {
        this->base_ = base;
    }

    auto sock() const -> int
    {
        return sock_;
    }
    auto set_sock(int sock) -> void
    {
        this->sock_ = sock;
    }

    int thread_id() const
    {
        return thread_id_;
    }

    void set_thread_id(int thread_id)
    {
        this->thread_id_ = thread_id;
    }

private:
    struct event_base *base_      = 0;
    int                sock_      = 0;
    int                thread_id_ = 0;
};

#endif // XTASK_H
