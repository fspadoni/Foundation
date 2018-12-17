#ifndef TaskScheduler_h__
#define TaskScheduler_h__

#include <PlatformDefine.h>
#include <async/Task.h>
#include <async/Locks.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <map>
#include <deque>
#include <string> 


namespace async
{

    class FoundationDLL TaskScheduler
    {

    public:

        virtual ~TaskScheduler();

        static TaskScheduler* create(const char* name = "");

        typedef std::function<TaskScheduler* ()> TaskSchedulerCreatorFunction;

        static bool registerScheduler(const char* name, std::function<TaskScheduler* ()> creatorFunc);

        static TaskScheduler* getInstance();

        static const std::string& getCurrentName()  { return _currentSchedulerName; }

        // interface
        virtual void init(const unsigned int nbThread = 0) = 0;

        virtual void stop(void) = 0;

        virtual unsigned int getThreadCount(void) const = 0;

        virtual const char* getCurrentThreadName() = 0;

        // queue task if there is space, and run it otherwise
        virtual bool addTask(Task* task) = 0;

        virtual void workUntilDone(Task::Status* status) = 0;

        virtual void* allocateTask(size_t size) = 0;

        virtual void releaseTask(Task*) = 0;

    protected:

        // factory map: registered schedulers: name, creation function
        static std::map<std::string, std::function<TaskScheduler*()> > _schedulers;

        // current instantiated scheduler
        static std::string _currentSchedulerName;
        static TaskScheduler * _currentScheduler;

        friend class Task;
    };




    FoundationDLL bool runThreadSpecificTask(const Task *pTask );


} // namespace async


#endif // TaskScheduler_std_h__
