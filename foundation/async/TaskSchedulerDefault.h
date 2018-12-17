#ifndef TaskSchedulerDefault_h__
#define TaskSchedulerDefault_h__

#include <async/TaskScheduler.h>

#include <atomic>

// default
#include <thread>
#include <condition_variable>
#include <memory>
#include <map>
#include <deque>
#include <string> 
#include <mutex>


// workerthread
#include <async/Locks.h>


namespace async  {

//#define ENABLE_TASK_SCHEDULER_PROFILER 1     // Comment this line to disable the profiler

#if ENABLE_TASK_SCHEDULER_PROFILER

#include "TaskSchedulerProfiler.h"

#else
    //----------------------
    // Profiler is disabled
    //----------------------
#define DECLARE_TASK_SCHEDULER_PROFILER(name)
#define DEFINE_TASK_SCHEDULER_PROFILER(name)
#define TASK_SCHEDULER_PROFILER(name)

#endif


    class TaskSchedulerDefault;
    class WorkerThread;


    class WorkerThread
    {
    public:

        WorkerThread(TaskSchedulerDefault* const& taskScheduler, const int index, const std::string& name = "Worker");

        ~WorkerThread();

        static WorkerThread* getCurrent();

        // queue task if there is space, and run it otherwise
        bool addTask(Task* pTask);

        void workUntilDone(Task::Status* status);

        Task::Status* getCurrentStatus() const { return _currentStatus; }

        const char* getName() { return _name.c_str(); }

        const size_t getIndex() { return _index; }

        const std::thread::id getId();

        const std::deque<Task*>* getTasksQueue() { return &_tasks; }

        std::uint64_t getTaskCount() { return _tasks.size(); }

        int GetWorkerIndex();

        void* allocate();

        void free(void* ptr);


    private:

        bool start(TaskSchedulerDefault* const& taskScheduler);

        std::thread* create_and_attach(TaskSchedulerDefault* const& taskScheduler);

        void runTask(Task* task);

        // queue task if there is space (or do nothing)
        bool pushTask(Task* pTask);

        // pop task from queue
        bool popTask(Task** ppTask);

        // steal and queue some task from another thread
        bool stealTask(Task** task);

        void doWork(Task::Status* status);

        // boost thread main loop
        void run(void);

        //void    ThreadProc(void);
        void    Idle(void);

        bool isFinished();

    private:

        enum
        {
            Max_TasksPerThread = 256
        };

        const std::string _name;

        const size_t _index;

        async::SpinLock _taskMutex;

        std::deque<Task*> _tasks;

        std::thread  _stdThread;

        Task::Status*    _currentStatus;

        TaskSchedulerDefault*     _taskScheduler;

        // The following members may be accessed by _multiple_ threads at the same time:
        volatile bool    _finished;

        friend class TaskSchedulerDefault;
    };



    class FoundationDLL TaskSchedulerDefault : public TaskScheduler
    {
        enum
        {
            MAX_THREADS = 16,
            STACKSIZE = 64 * 1024 /* 64K */,
        };

    public:

        // interface
        virtual void init(const unsigned int nbThread = 0) final;
        virtual void stop(void) final;
        virtual unsigned int getThreadCount(void)  const final { return _threadCount; }
        virtual const char* getCurrentThreadName() final;
        // queue task if there is space, and run it otherwise
        virtual bool addTask(Task* task) final;
        virtual void workUntilDone(Task::Status* status) final;
        virtual void* allocateTask(size_t size) final;
        virtual void releaseTask(Task*) final;

    public:

        // factory methods: name, creator function
        static const char* name() { return "_default"; }

        static TaskSchedulerDefault* create();

        static const bool isRegistered;

    private:

        bool isInitialized() { return _isInitialized; }

        bool isClosing(void) const { return _isClosing; }

        void    WaitForWorkersToBeReady();

        void    wakeUpWorkers();

        static unsigned GetHardwareThreadsCount();

        WorkerThread* getCurrentThread();

        const WorkerThread* getWorkerThread(const std::thread::id id);


    private:

        static const std::string _name;

        // TO DO: replace with thread_specific_ptr. clang 3.5 doesn't support C++ 11 thread_local vars on Mac
//        static thread_local WorkerThread* _workerThreadIndex;
        static std::map< std::thread::id, WorkerThread*> _threads;

        Task::Status*    _mainTaskStatus;

        std::mutex  _wakeUpMutex;

        std::condition_variable _wakeUpEvent;

    private:

        TaskSchedulerDefault();

        TaskSchedulerDefault(const TaskSchedulerDefault&) {}

        virtual ~TaskSchedulerDefault();

        void start(unsigned int NbThread);

        bool _isInitialized;

        unsigned _workerThreadCount;

        bool _workerThreadsIdle;

        bool _isClosing;

        unsigned _threadCount;


        friend class WorkerThread;
    };


} // namespace async


#endif // TaskSchedulerDefault_h__
