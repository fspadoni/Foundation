#include <async/Task.h>


//namespace
//{

    // compute recursively the Fibonacci number for input N  O(~1.6 exp(N))
    // this is implemented to test the task scheduler generating super lightweight tasks and not for performance
    class FibonacciTask : public async::Task
    {
    public:
        FibonacciTask(const int64_t N, int64_t* const sum, bool useDynamicTaskAllocation,  async::Task::Status* status)
        : Task(status)
        , _N(N)
        , _sum(sum)
        , _useDynamicTaskAllocation(useDynamicTaskAllocation)
        {}
        
        virtual ~FibonacciTask() { }
        
        virtual bool run() final;
        
    private:
        
        const int64_t _N;
        int64_t* const _sum;
        const bool _useDynamicTaskAllocation;
    };
    
    
    // compute recursively the sum of integers from first to last
    // this is implemented to test the task scheduler generating super lightweight tasks and not for performance
    class IntSumTask : public async::Task
    {
    public:
        IntSumTask(const int64_t first, const int64_t last, int64_t* const sum, bool useDynamicTaskAllocation, async::Task::Status* status)
        : Task(status)
        , _first(first)
        , _last(last)
        , _sum(sum)
        , _useDynamicTaskAllocation(useDynamicTaskAllocation)
        {}
        
        virtual ~IntSumTask() {}
        
        virtual bool run() final;
        
        
    private:
        
        const int64_t _first;
        const int64_t _last;
        int64_t* const _sum;
        const bool _useDynamicTaskAllocation;
    };
//} // namespace 

