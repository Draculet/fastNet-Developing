#include "Thread.h"
#include "Eventloop.h"
#include "Mutex.h"
#include <memory>

using namespace std;
using namespace net;

class ThreadLoop
{
    public:
    ThreadLoop():
        loop_(new Eventloop()),
        thread_(new Thread(&Eventloop::runInThread, &loop_),
        mutex_(),
        cond_(mutex_),
        state_(kRun)
    {

    }
    
    ~ThreadLoop()
    {
        //TODO 填充细节处理,日志输出
    }

    void start()
    {
        state_ = kRun;
        thread_.start();
        MutexGuard mutex(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait();
        }
    }

    /* 
        11.10
        将EventLoop中的runInThread移到此处,方便直接用锁
    */
    void runInThread()
    {
        Eventloop loop;
        /*
            FIXME

            此处初始化Eventloop,tid_初始化时确定,并且不会改变
            后面跨线程访问是否会有问题,是否应该加锁?
         */
        {
            MutexGuard mutex(mutex_);
            loop_ = &loop;
            cond_.notify();
        }
        loop.loop();
        MutexGuard mutex(mutex_);
        loop_ = nullptr;
        state_ = kQuit;
    }

    private:
    Eventloop *loop_;
    unique_ptr<Thread> thread_;
    Mutex mutex_;
    Condition cond_;
    enum {kRun, kQuit} state_;
}