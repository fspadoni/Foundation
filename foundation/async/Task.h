#ifndef Foundation_Task_h__
#define Foundation_Task_h__

#include <PlatformDefine.h>

#include <atomic>
#include <mutex>

#include <async/LockFreeAllocator.h>

namespace async
{


    class FoundationDLL Task
    {
    public:
        // Allocator for small tasks.
        enum
        {
            /** Total size in bytes for a small task that will use the custom allocator **/
            SMALL_TASK_SIZE = 256            
        };
        typedef TLockFreeFixedSizeAllocator_TLSCache<SMALL_TASK_SIZE, PLATFORM_CACHE_LINE_SIZE> TaskAllocator;
//        typedef TLockFreeFixedSizeAllocator<SMALL_TASK_SIZE, PLATFORM_CACHE_LINE_SIZE> TaskAllocator;

        // Task Status class definition
        class Status
        {
        public:
            Status() : _busy(0) {}

            bool isBusy() const
            {
                return (_busy.load(std::memory_order_relaxed) > 0);
            }

            int setBusy(bool busy)
            {
                if (busy)
                {
                    return _busy.fetch_add(1, std::memory_order_relaxed);
                }
                else
                {
                    return _busy.fetch_sub(1, std::memory_order_relaxed);
                }
            }

        private:
            std::atomic<int> _busy;
        };


        static void* operator new(std::size_t sz)
        {
            if (sz > SMALL_TASK_SIZE)
            {
                return ::operator new(sz);
            }
            else
            {
                return Task::GetAllocator().Allocate();
            }
        }
        
        static void operator delete(void* ptr, std::size_t sz)
        {
            if (sz > SMALL_TASK_SIZE)
            {
                ::operator delete(ptr);
            }
            else
            {
                return Task::GetAllocator().Free(ptr);
            }
        }
            
            
        Task(const Task::Status* status = nullptr);

        virtual ~Task();

    public:

        virtual bool run() = 0;


        // remove from this interface
    public:

        inline Task::Status* getStatus(void) const
        {
            return const_cast<Task::Status*>(_status);
        }

        /** Singleton to retrieve the small task allocator **/
        static TaskAllocator& GetAllocator();
        
    protected:

        const Task::Status*    _status;

    public:
        int _id;
    };




    // This task is called once by each thread used by the TasScheduler
    // this is useful to initialize the thread specific variables
    class FoundationDLL ThreadSpecificTask : public Task
    {

    public:

        ThreadSpecificTask(std::atomic<int>* atomicCounter, std::mutex* mutex, const Task::Status* status);

        virtual ~ThreadSpecificTask();

        virtual bool run() final;


    private:

        virtual bool runThreadSpecific() { return true; }

        virtual bool runCriticalThreadSpecific() { return true; }


        std::atomic<int>* _atomicCounter;
        std::mutex*     _threadSpecificMutex;
    };



} // namespace async



#endif // Foundation_Task_h__
