#include "TaskSchedulerTestTasks.h"

#include <async/TaskScheduler.h>


//namespace
//{

    
    bool FibonacciTask::run()
    {
        if (_N < 2)
        {
            *_sum = _N;
            return false;
        }
        
        Task::Status status;
        
        int64_t x, y;
        
        async::TaskScheduler* scheduler = async::TaskScheduler::getInstance();
        
        if (_useDynamicTaskAllocation)
        {
            FibonacciTask* task0 = new FibonacciTask(_N - 1, &x, _useDynamicTaskAllocation, &status);
            FibonacciTask* task1 = new FibonacciTask(_N - 2, &y, _useDynamicTaskAllocation, &status);
            
            scheduler->addTask(task0);
            scheduler->addTask(task1);
            scheduler->workUntilDone(&status);
        }
        else
        {
            FibonacciTask task0(_N - 1, &x, _useDynamicTaskAllocation, &status);
            FibonacciTask task1(_N - 2, &y, _useDynamicTaskAllocation, &status);
            
            scheduler->addTask(&task0);
            scheduler->addTask(&task1);
            scheduler->workUntilDone(&status);
        }
        
        // Do the sum
        *_sum = x + y;
        
        if (_useDynamicTaskAllocation)
            return true;
        
        return false;
    }
    
    
    
    bool IntSumTask::run()
    {
        const int64_t count = _last - _first;
        if (count < 1)
        {
            *_sum = _first;
            return false;
        }
        
        const int64_t mid = _first + (count / 2);
        
        Task::Status status;
        
        int64_t x, y;
        
        async::TaskScheduler* scheduler = async::TaskScheduler::getInstance();
        
        if (_useDynamicTaskAllocation)
        {
            IntSumTask* task0 = new IntSumTask(_first, mid, &x, _useDynamicTaskAllocation, &status);
            IntSumTask* task1 = new IntSumTask(mid+1, _last, &y, _useDynamicTaskAllocation, &status);
            
            scheduler->addTask(task0);
            scheduler->addTask(task1);
            scheduler->workUntilDone(&status);
        }
        else
        {
            IntSumTask task0(_first, mid, &x, _useDynamicTaskAllocation, &status);
            IntSumTask task1(mid+1, _last, &y, _useDynamicTaskAllocation, &status);
            
            scheduler->addTask(&task0);
            scheduler->addTask(&task1);
            scheduler->workUntilDone(&status);
        }
        
        
        
        // Do the sum
        *_sum = x + y;
        
        if (_useDynamicTaskAllocation)
            return true;
        
        return false;
    }
    
    
//} // namespace 

