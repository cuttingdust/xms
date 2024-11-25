#include "XThreadPool.h"

#include "XThread.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <signal.h>
#endif

#include <vector>
#include <thread>
#include <iostream>
#include <mutex>

static bool                   isExitAll = false;
static std::vector<XThread *> allThreads;
static std::mutex             allThreadsMutex;

class CThreadPool : public XThreadPool
{
public:
    auto init(int threadNum) -> void override
    {
        threadNum_  = threadNum;
        lastThread_ = -1;
        for (int i = 0; i < threadNum; ++i)
        {
            XThread *t = new XThread();
            t->setId(i + 1);
            std::cout << "create thread " << i << std::endl;
            /// �����߳�
            t->start();
            threads_.emplace_back(t);
            allThreadsMutex.lock();
            allThreads.emplace_back(t);
            allThreadsMutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    auto dispatch(XTask *task) -> void override
    {
        /// ��ѯ
        if (!task)
        {
            return;
        }

        int tid     = (lastThread_ + 1) % threadNum_;
        lastThread_ = tid;
        XThread *t  = threads_[tid];

        /// �������
        t->addTask(task);

        /// �����߳�
        t->activate();
    }

private:
    int                    threadNum_ = 0;
    std::vector<XThread *> threads_;
    int                    lastThread_ = -1;
};

auto XThreadPool::exitAllThread() -> void
{
    isExitAll = true;
    allThreadsMutex.lock();
    for (auto t : allThreads)
    {
        t->exit();
    }
    allThreadsMutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

auto XThreadPool::wait() -> void
{
    while (!isExitAll)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

auto XThreadPoolFactory::create() -> XThreadPool *
{
    static std::once_flag flag;
    std::call_once(flag,
                   []()
                   {
#ifdef _WIN32
                       /// ��ʼ��socket��
                       WSADATA wsa;
                       WSAStartup(MAKEWORD(2, 2), &wsa);
#else
                        ///ʹ�öϿ�����socket���ᷢ�����źţ���ɳ����˳�
                        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
                            return;
#endif
                   });

    return new CThreadPool();
}
