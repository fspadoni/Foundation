//
//  Task.h
//
//  Created by Federico Spadoni on 29/01/15.
//
//

#ifndef __foundation_Task__
#define __foundation_Task__

#include "PlatformDefine.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace foundation
{

class WorkerThread;
class TaskScheduler;
class InternalForTask;

class FoundationDLL Task
{
public:
    
    
    // Task Status class definition
    class Status
    {
    public:
        Status();
        
        bool isBusy() const;
        
        
        
    private:
        
        void markBusy(bool busy);
        
        std::atomic<int> _busy;
        
        friend class WorkerThread;
        friend class InternalForTask;
    };
    
    
    
protected:
    
    Task(const Task::Status* status);
    
    
public:
    
    virtual ~Task();
    
    //struct TaskTag{};
    //typedef boost::singleton_pool<TaskTag, sizeof(*this)> memory_pool;
    
    
    virtual bool run(WorkerThread* thread) = 0;
    
    
    /* Keep half the work and put the other half in a new task	*/
    virtual bool split(WorkerThread* , Task** ) { return false; }
    
    /* returns a sub part of the task */
    virtual bool partialPop(WorkerThread* , Task** ) { return false; }
    
    /* share work across all threads (pool is idle) */
    virtual bool spread( TaskScheduler* const& ) { return false; }
    
    
private:
    
    Task(const Task& task) {}
    Task& operator= (const Task& task) {return *this;}
    
    
protected:
    
    inline Task::Status* getStatus(void) const;
    
    
    
    const Task::Status*	_status;
    
    friend class WorkerThread;
    
};


class FoundationDLL EmptyTask : public Task
{
    
public:
    
    EmptyTask(Task::Status* status)
    : Task(status)
    {
    }
    
    virtual ~EmptyTask()  {}
    
    virtual bool run(WorkerThread* thread) override { delete this;  return true; }
    
};


// This task is called once by each thread used by the TasScheduler
// this is useful to initialize the thread specific variables
class FoundationDLL ThreadSpecificTask : public Task
{
    
    public:
    
    //InitPerThreadDataTask(volatile long* atomicCounter, boost::mutex* mutex, TaskStatus* pStatus );
    ThreadSpecificTask(std::atomic<int>* atomicCounter, std::mutex* mutex, Task::Status* pStatus );
    
    virtual ~ThreadSpecificTask();
    
    virtual bool runThreadSpecific()  {return true;}
    
    virtual bool runCriticalThreadSpecific() {return true;}
    
    private:
    
    virtual bool run(WorkerThread* );
    
    //volatile long* mAtomicCounter;
    std::atomic<int>* mAtomicCounter;
    
    std::mutex*	 mThreadSpecificMutex;
    
};


} //namespace foundation


#include "Task.inl"


#endif /* defined(__foundation_Task__) */
