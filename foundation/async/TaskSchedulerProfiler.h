/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2017 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef TaskSchedulerProfiler_h__
#define TaskSchedulerProfiler_h__

#include <Performance/config.h>

#include <chrono>
#include <iostream>     // std::cout
#include <iterator>     // std::ostream_iterator
#include <vector>       // std::vector
#include <algorithm>    // std::copy
#include <sstream>

//#define ENABLE_TASK_SCHEDULER_PROFILER 1     // Comment this line to disable the profiler


namespace sofa
{
    namespace simulation
    {
//        class WorkerThreadLockFree;
    }
}


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


//    } // namespace simulation
//
//} // namespace sofa


#endif // TaskSchedulerProfiler_h__
