#include <async/Task.h>


#include <assert.h>
#include <thread>

namespace async
{

    static Task::TaskAllocator TheTaskAllocator;
    
    Task::TaskAllocator& Task::GetAllocator()
    {
        return TheTaskAllocator;
    }
    
    Task::Task(const Task::Status* status)
        : _status(status)
        , _id(0)
    {
    }

    Task::~Task()
    {
    }
    
    
    
    ThreadSpecificTask::ThreadSpecificTask(std::atomic<int>* atomicCounter, std::mutex* mutex, const Task::Status* status )
        : Task(status)
        , _atomicCounter(atomicCounter)
        , _threadSpecificMutex(mutex)
    {}

    ThreadSpecificTask::~ThreadSpecificTask()
    {
    }

    bool ThreadSpecificTask::run()
    {

        runThreadSpecific();

        {
            std::lock_guard<std::mutex> lock(*_threadSpecificMutex);
            runCriticalThreadSpecific();
        }

        _atomicCounter->fetch_sub(1, std::memory_order_acq_rel);

        while(_atomicCounter->load(std::memory_order_relaxed) > 0)
        {
            // yield while waiting
            std::this_thread::yield();
        }
        return false;
    }


} // namespace async
