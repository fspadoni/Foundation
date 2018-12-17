#ifndef Foundation_Task_h__
#define Foundation_Task_h__

#include <PlatformDefine.h>

#include <atomic>
#include <mutex>


namespace async
{


    class FoundationDLL Task
    {
    public:

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
