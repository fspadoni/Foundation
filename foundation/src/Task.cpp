//
//  Task.cpp
//
//  Created by Federico Spadoni on 29/01/15.
//
//

#include "Task.h"


namespace foundation
{


Task::Task(const Task::Status* status)
: _status(status)
{
}

Task::~Task()
{
}





//InitPerThreadDataTask::InitPerThreadDataTask(volatile long* atomicCounter, boost::mutex* mutex, TaskStatus* pStatus )
ThreadSpecificTask::ThreadSpecificTask(std::atomic<int>* atomicCounter, std::mutex* mutex, Task::Status* status )
: Task(status)
, mAtomicCounter(atomicCounter)
, mThreadSpecificMutex(mutex)
{}

ThreadSpecificTask::~ThreadSpecificTask()
{
    //mAtomicCounter;
}

bool ThreadSpecificTask::run(WorkerThread* )
{
    
    runThreadSpecific();
    
    
    {
        std::lock_guard<std::mutex> lock(*mThreadSpecificMutex);
        
        runCriticalThreadSpecific();
        
    }
    
    //BOOST_INTERLOCKED_DECREMENT( mAtomicCounter );
    //BOOST_COMPILER_FENCE;
    
    --(*mAtomicCounter);
    
    
    while(mAtomicCounter->operator int() > 0)
    {
        // yield while waiting
        std::this_thread::yield();
    }  
    return NULL;  
}


} // namespace foundation
    
