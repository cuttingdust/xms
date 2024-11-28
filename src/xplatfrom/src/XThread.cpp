#include "XThread.h"

#include "XTask.h"

#include <event2/util.h>
#include <event2/event.h>

#include <iostream>
#include <thread>
#include <list>
#include <mutex>

#ifndef _WIN32
#include <unistd.h>
#endif

/// 激活线程任务的回调函数
static void notify_cb(evutil_socket_t fd, short which, void *arg)
{
    auto *t = static_cast<XThread *>(arg);
    t->notify(fd, which);
}

class XThread::PImpl
{
public:
    PImpl(XThread *owenr);
    ~PImpl() = default;

public:
    XThread           *owenr_          = nullptr;
    int                notify_send_fd_ = 0;
    int                id_             = 0;
    struct event_base *base_           = nullptr;
    std::list<XTask *> tasks_;
    std::mutex         tasks_mutex_;
    bool               is_exit_ = false;
};

XThread::PImpl::PImpl(XThread *owenr) : owenr_(owenr)
{
}


XThread::XThread()
{
    impl_ = std::make_shared<PImpl>(this);
}

XThread::~XThread() = default;

/// 安装线程，初始化event_base和管道监听事件用于激活
auto XThread::setup() -> bool
{
    std::cout << "XThread::setup" << std::endl;

    /// windows 用配对socket
    /// linux   用管道
#ifdef _WIN32
    /// 创建一个sockerpair 可以互相通信 fd[0]读 fd[1]写
    evutil_socket_t fds[2];
    if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
    {
        std::cout << "evutil_socketpair failed!" << std::endl;
        return false;
    }

    /// 设置成非阻塞
    evutil_make_socket_nonblocking(fds[0]);
    evutil_make_socket_nonblocking(fds[1]);

#else
    /// 创建的管道不能用send recv读取 read write
    int fds[2];
    if (pipe(fds))
    {
        std::cerr << "pipe failed!" << std::endl;
        return false;
    }

#endif

    /// 读取绑定到event事件中，写入要保存
    impl_->notify_send_fd_ = fds[1];

    /// 创建libevent上下文（无锁）
    event_config *ev_conf = event_config_new();
    event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
    impl_->base_ = event_base_new_with_config(ev_conf);
    event_config_free(ev_conf);
    if (!impl_->base_)
    {
        std::cerr << "event_base_new_with_config failed in thread!" << std::endl;
        return false;
    }

    /// 添加管道监听事件
    event *ev = event_new(impl_->base_, fds[0], EV_READ | EV_PERSIST, notify_cb, this);
    event_add(ev, 0);

    return true;
}

auto XThread::start() -> void
{
    setup();

    std::cout << "XThread::start" << std::endl;

    /// 启动线程
    std::thread t(&XThread::threadFun, this);

    /// 断开与主线程的联系
    t.detach();
}

auto XThread::notify(intptr_t fd, short which) -> void
{
    /// 水平触发 只要有数据就会触发
    char buf[2] = { 0 };
#ifdef _WIN32
    int re = ::recv(fd, buf, 1, 0);
#else
    int re = ::read(fd, buf, 1);
#endif
    if (re <= 0)
        return;

    std::cout << impl_->id_ << " thread :" << buf << std::endl;

    XTask *task = nullptr;
    /// 取出任务 并初始化任务
    impl_->tasks_mutex_.lock();
    if (impl_->tasks_.empty())
    {
        impl_->tasks_mutex_.unlock();
        return;
    }
    task = impl_->tasks_.front();
    impl_->tasks_.pop_front();
    impl_->tasks_mutex_.unlock();
    task->init();
}

auto XThread::addTask(XTask *t) -> void
{
    std::cout << "XThread::addTask" << std::endl;
    if (!t)
    {
        return;
    }
    t->set_base(impl_->base_);

    impl_->tasks_mutex_.lock();
    impl_->tasks_.push_back(t);
    impl_->tasks_mutex_.unlock();
}

/// 激活线程
auto XThread::activate() -> void
{
#ifdef _WIN32
    int re = ::send(impl_->notify_send_fd_, "c", 1, 0);
#else
    int re = ::write(impl_->notify_send_fd_, "c", 1);
#endif
    if (re <= 0)
    {
        std::cerr << "XThread::activate failed!" << std::endl;
    }
}

auto XThread::setId(int id) -> void
{
    impl_->id_ = id;
}

auto XThread::getId() const -> int
{
    return impl_->id_;
}

auto XThread::threadFun() -> void
{
    std::cout << impl_->id_ << " XThread::threadFun "
              << "begin" << std::endl;

    if (!impl_->base_)
    {
        std::cerr << "event_base_new_with_config failed in thread!" << std::endl;
        return;
    }

    /// 设置为不阻塞分发消息
    while (!impl_->is_exit_)
    {
        /// 一次处理多条消息
        event_base_loop(impl_->base_, EVLOOP_NONBLOCK);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // event_base_dispatch(impl_->base_);
    event_base_free(impl_->base_);
    std::cout << impl_->id_ << " XThread::threadFun "
              << "end" << std::endl;
}

auto XThread::exit() -> void
{
    impl_->is_exit_ = true;
}
