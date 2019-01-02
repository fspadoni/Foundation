#ifndef TaskSchedulerProfiler_h__
#define TaskSchedulerProfiler_h__


#include <chrono>
#include <iostream>     // std::cout
#include <iterator>     // std::ostream_iterator
#include <vector>       // std::vector
#include <algorithm>    // std::copy
#include <sstream>

//#define ENABLE_TASK_SCHEDULER_PROFILER 1     // Comment this line to disable the profiler


namespace async
{


#if ENABLE_TASK_SCHEDULER_PROFILER

    //------------------------------------------------------------------
    // A class for local variables created on the stack by the API_PROFILER macro:
    //------------------------------------------------------------------
    class TaskSchedulerProfiler
    {
    public:
        //------------------------------------------------------------------
        // A structure for each thread to store information about an API:
        //------------------------------------------------------------------
        struct ThreadInfo
        {
            long long accumulator;  // total time spent in target module since the last report
            long long hitCount;     // number of times the target module was called since last report
            const char *name;       // the name of the target module
            std::vector<long long> timeIntervals;
            std::chrono::time_point<std::chrono::high_resolution_clock> lastReportTime;
        };
        
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
        ThreadInfo *m_threadInfo;
        
        //static float s_ooFrequency;      // 1.0 divided by QueryPerformanceFrequency()
        const long long s_reportInterval = 1000;   // length of time between reports
        void flush(long long end);
        
    public:
        
        inline TaskSchedulerProfiler(ThreadInfo *threadInfo)
        {
            m_start = std::chrono::high_resolution_clock::now();
            m_threadInfo = threadInfo;
        }
        
        inline ~TaskSchedulerProfiler()
        {
            auto end = std::chrono::high_resolution_clock::now();
            long long timeInterval = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count();
    //        m_threadInfo->timeIntervals.push_back(timeInterval);
            m_threadInfo->accumulator += timeInterval;
            m_threadInfo->hitCount++;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(end - m_threadInfo->lastReportTime).count() > s_reportInterval)
            {
                flush(end);
            }
        }
        
        
        //------------------------------------------------------------------
        // Flush is called at the rate determined by APIProfiler_ReportIntervalSecs
        //------------------------------------------------------------------
        void flush(std::chrono::time_point<std::chrono::high_resolution_clock> end);
        
    };

//----------------------
// Profiler is enabled
//----------------------
#define DECLARE_TASK_SCHEDULER_PROFILER(name) \
extern thread_local  TaskSchedulerProfiler::ThreadInfo __TaskSchedulerProfiler_##name;

#define DEFINE_TASK_SCHEDULER_PROFILER(name) \
thread_local TaskSchedulerProfiler::ThreadInfo __TaskSchedulerProfiler_##name = { 0, 0, #name };

#define TOKENPASTE2(x, y) x ## y
#define TOKENPASTE(x, y) TOKENPASTE2(x, y)
#define TASK_SCHEDULER_PROFILER(name) \
TaskSchedulerProfiler TOKENPASTE(__TaskSchedulerProfiler_##name, __LINE__)(&__TaskSchedulerProfiler_##name)

#else

//----------------------
// Profiler is disabled
//----------------------
#define DECLARE_TASK_SCHEDULER_PROFILER(name)
#define DEFINE_TASK_SCHEDULER_PROFILER(name)
#define TASK_SCHEDULER_PROFILER(name)

#endif


//
} // namespace async


#endif // TaskSchedulerProfiler_h__
