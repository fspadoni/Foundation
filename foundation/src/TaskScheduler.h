//
//  CCTaskScheduler.h
//
//  Created by Federico Spadoni on 29/01/15.
//
//

#ifndef foundation_TaskScheduler__
#define foundation_TaskScheduler__

#include "platformDefine.h"

#include "Task.h"

#include <thread>
#include <atomic>
#include <condition_variable>

#include <pthread.h>


namespace foundation
{

class TaskScheduler;
class WorkerThread;




class SpinLockMutex
{
    public:
    
    SpinLockMutex()
    : _mutex(0)
    {
    }
    
    SpinLockMutex(std::atomic_flag* mutex, bool bLock=true)
    : _mutex(mutex)
    {
        if (bLock)
        {
            lock();
        }
    }

    ~SpinLockMutex()
    {
        unlock();
    }
    
    bool try_lock(std::atomic_flag* mutex)
    {
        if (mutex->test_and_set(std::memory_order_seq_cst))
        {
            return false;
        }
        
        _mutex = mutex;
        return true;
    }
    
    void lock()
    {
        while(_mutex->test_and_set(std::memory_order_seq_cst))
        {
//            std::this_thread::yield();
        }
    }
    
    void unlock()
    {
        if (_mutex)
            _mutex->clear(std::memory_order_seq_cst);
    }
    
private:
    
    std::atomic_flag* _mutex;
};




class SpinLock
{
    
    std::atomic_flag _v;
    
public:
    
    SpinLock() : _v(ATOMIC_FLAG_INIT)
    {}
    
    ~SpinLock()
    {
        unlock();
    }

    
public:
    
    bool try_lock()
    {
        return !_v.test_and_set( std::memory_order_seq_cst );
    }
    
    void lock()
    {
        while( _v.test_and_set(std::memory_order_seq_cst) )
        {
            std::this_thread::yield();
        }
    }
    
    void unlock()
    {
        _v .clear( std::memory_order_seq_cst );
    }
    
public:
    
    class ScopedLock
    {
    private:
        
        SpinLock & _sp;
        
        ScopedLock( ScopedLock const & );
        ScopedLock & operator=( ScopedLock const & );
        
    public:
        
        explicit ScopedLock( SpinLock & sp ): _sp( sp )
        {
            sp.lock();
        }
        
        ~ScopedLock()
        {
            _sp.unlock();
        }
    };
};



class FoundationDLL WorkerThread
{
public:
    
    WorkerThread(TaskScheduler* const& taskScheduler);
    
    ~WorkerThread();
    
    static WorkerThread* getCurrent();
    
    // queue task if there is space, and run it otherwise
    bool addTask(Task* pTask);
    
    void workUntilDone(Task::Status* status);
    
    Task::Status* getCurrentStatus() const {return _currentStatus;}
    
//    std::atomic_flag* getTaskMutex() const {return &_TaskMutex;}
    
    SpinLock& getTaskLock()  {return _taskSpinLock;}
    
    std::thread::id getId();
    
    Task** getTasks() {return _tasks;}
    
    unsigned int& getTaskCount()  {return _taskCount; }
    
    int GetWorkerIndex();
    
private:
    
    bool start(TaskScheduler* const& taskScheduler);
    
    
    std::thread* create_and_attach( TaskScheduler* const& taskScheduler);
    
    bool releaseThread();
    
    
    
    // queue task if there is space (or do nothing)
    bool pushTask(Task* pTask);
    
    // pop task from queue
    bool popTask(Task** ppTask);
    
    // steal and queue some task from another thread
    bool stealTasks();
    
    // give an idle thread some work
    bool giveUpSomeWork(WorkerThread* pIdleThread);
    
    
    void doWork(Task::Status* status);
    
    // boost thread main loop
    void run(void);
    
    
    //void	ThreadProc(void);
    void	Idle(void);
    
    bool attachToThisThread(TaskScheduler* pScheduler);
    
    
    
    
    
private:
    
    enum
    {
        Max_TasksPerThread = 256
    };
    
    
    
    
//    mutable std::atomic_flag		_TaskMutex;
    SpinLock _taskSpinLock;
    
    Task*		_tasks[Max_TasksPerThread];
    unsigned int	_taskCount;
    Task::Status*	_currentStatus;
    
    
    TaskScheduler*     _taskScheduler;
    std::thread  _thread;
    
    // The following members may be accessed by _multiple_ threads at the same time:
    bool	_finished;
    
    
    friend class TaskScheduler;
    
};




class FoundationDLL TaskScheduler

{
    enum
    {
        MAX_THREADS = 16,
        STACKSIZE = 64*1024 /* 64K */
    };
    
public:
    
    static TaskScheduler& getInstance();
    
    
    bool start(const unsigned int NbThread = 0);
    
    bool stop(void);
    
    bool isClosing(void) const { return _isClosing; }
    
    unsigned int getThreadCount(void) const { return _threadCount; }
    
    
    void	WaitForWorkersToBeReady();
    
    void	wakeUpWorkers();
    
    static unsigned GetHardwareThreadsCount();
    
    unsigned size()	const;
    
    WorkerThread* getWorkerThread(const unsigned int index);
    
    
private:
    
//    static std::__thread_specific_ptr<WorkerThread>	mWorkerThreadIndex;
    static thread_local WorkerThread* _workerThreadIndex;
//    static __thread WorkerThread* mWorkerThreadIndex;
//    static pthread_key_t mWorkerThreadIndex;
    
    //boost::thread_group _Threads;
    WorkerThread* 	_threads[MAX_THREADS];
    
    
    // The following members may be accessed by _multiple_ threads at the same time:
    Task::Status*	_mainTaskStatus;
    static Task::Status _initStatus;
    
    bool _readyForWork;
    
    std::mutex  _wakeUpMutex;
    
    std::condition_variable _wakeUpEvent;
    //boost::condition_variable sleepEvent;
    
    
    
private:
    
    TaskScheduler();
    
    TaskScheduler(const TaskScheduler& ) {}
    
    ~TaskScheduler();
    
    bool _isInitialized;
    // The following members may be accessed by _multiple_ threads at the same time:
    unsigned _workerCount;
    unsigned _targetWorkerCount;
    unsigned _activeWorkerCount;
    
    
    bool			_workersIdle;
    
    
    bool            _isClosing;
    
    unsigned		_threadCount;
    
    
    
    friend class WorkerThread;
};		





FoundationDLL bool runThreadSpecificTask(WorkerThread* pThread, const Task *pTask );

FoundationDLL bool runThreadSpecificTask(const Task *pTask );

} // namespace foundation

foundation_TaskScheduler__

#endif /* defined(foundation_TaskScheduler__) */
