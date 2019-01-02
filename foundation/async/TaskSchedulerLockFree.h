#ifndef Foundation_TaskSchedulerLockFree_h__
#define Foundation_TaskSchedulerLockFree_h__

#include <async/TaskScheduler.h>
#include <async/LockFreeDeQueue.h>
#include <async/TasksAllocator.h>

#include <thread>
#include <memory>
#include <atomic>
#include <map>

namespace async
{


    class TaskSchedulerLockFree;
    class WorkerThreadLockFree;
    class TasksAllocators;

    
    class WorkerThreadLockFree
    {
    public:
        
        WorkerThreadLockFree(TaskSchedulerLockFree* const& taskScheduler, const int index, const std::string& name = "Worker");
        
        ~WorkerThreadLockFree();
        
        static WorkerThreadLockFree* getCurrent();
        
        static WorkerThreadLockFree* getThread();
        
        // queue task if there is space, and run it otherwise
        bool addTask(Task* pTask);
        
        void workUntilDone(Task::Status* status);
        
        Task::Status* getCurrentStatus() const {return _currentStatus;}
        
        const char* getName() { return _name.c_str(); }
        
        const size_t getIndex() { return _index; }
        
        const std::thread::id getId();
        
        const LockFreeDeQueue<Task*>* getTasksQueue() {return &_tasks;}
        
        std::uint64_t getTaskCount()  {return _tasks.getCount(); }
        
        int GetWorkerIndex();
        
        void* allocate();
        
        void free(void* ptr);
        
        
    private:
        
        bool start(TaskSchedulerLockFree* const& taskScheduler);
        
        std::thread* create_and_attach( TaskSchedulerLockFree* const& taskScheduler);
        
        void runTask(Task* task);
        
        // queue task if there is space (or do nothing)
        bool pushTask(Task* pTask);
        
        // pop task from queue
        bool popTask(Task** ppTask);
        
        // steal and queue some task from another thread
        bool stealTask();
        
        bool stealTask(Task** task);
        
        // give an idle thread some work
        bool giveUpSomeWork(WorkerThreadLockFree* pIdleThread);
        
        bool giveUpSomeWork(Task** stolenTask);
        
        void doWork(Task::Status* status);
        
        // boost thread main loop
        void run(void);
        
        
        void Idle(void);
        
        bool attachToThisThread(TaskSchedulerLockFree* pScheduler);
        
        bool isFinished();
        
    private:
        
        enum
        {
            Max_TasksPerThread = 256
        };
        
        const std::string _name;
        
        const size_t _index;
        
        LockFreeDeQueue<Task*> _tasks;
        
        std::thread  _stdThread;
        
        Task::Status*    _currentStatus;
        
        TaskSchedulerLockFree*     _taskScheduler;
        
        TasksAllocator<256,64> _taskAllocator;
        
        // The following members may be accessed by _multiple_ threads at the same time:
        std::atomic<bool>    _finished;
        
        
        friend class TaskSchedulerLockFree;
    };
    
    
    
    
    class FoundationDLL TaskSchedulerLockFree : public TaskScheduler
    
    {
        enum
        {
            MAX_THREADS = 16,
            STACKSIZE = 64*1024 /* 64K */,
        };
        
        
    public:
        
        // interface
        virtual void init(const unsigned int nbThread = 0) override final;
        virtual void stop(void) override final;
        virtual unsigned int getThreadCount(void)  const override final { return _threadCount; }
        virtual const char* getCurrentThreadName() override final;
        // queue task if there is space, and run it otherwise
        virtual bool addTask(Task* task) override final;
        virtual void workUntilDone(Task::Status* status) override final;
        virtual void* allocateTask(size_t size) override final;
        virtual void releaseTask(Task*) override final;
        
    public:
        
        // factory methods: name, creator function
        static const char* name() { return "lockFree"; }// _name.c_str(); }
        
        static TaskSchedulerLockFree* create();
        
        static const bool isRegistered;
        
    public:
        
        
        bool isInitialized() { return _isInitialized; }
        
        bool isClosing(void) const { return _isClosing; }
        
        void    WaitForWorkersToBeReady();
        
        void    wakeUpWorkers();
        
        static unsigned GetHardwareThreadsCount();
        
        const WorkerThreadLockFree* getWorkerThread(const std::thread::id id);
        
        
    private:
        
        TaskSchedulerLockFree();
        
        TaskSchedulerLockFree(const TaskSchedulerLockFree&) {}
        
        ~TaskSchedulerLockFree();
        
        void start(unsigned int NbThread);
        
        // thread storage initialization
        //void initThreadLocalData();
        
    private:
        
        //static thread_local WorkerThreadLockFree* _workerThreadIndex;
        
        static const std::string _name;
        
        std::map< std::thread::id, WorkerThreadLockFree*> _threads;
        
        Task::Status*    _mainTaskStatus;
        
        std::mutex  _wakeUpMutex;
        
        std::condition_variable _wakeUpEvent;
        
        
        bool _isInitialized;
        
        unsigned _workerThreadCount;
        
        bool                        _workerThreadsIdle;
        
        bool _isClosing;
        
        unsigned                    _threadCount;
        
        friend class WorkerThreadLockFree;
    };

} // namespace async


#endif // Foundation_TaskSchedulerLockFree_h__
