#include <Performance/src/TaskSchedulerProfiler.h>

#if defined(ENABLE_TASK_SCHEDULER_PROFILER)


#include <Performance/src/TaskSchedulerLockFree.h>

void TaskSchedulerProfiler::flush(std::chrono::time_point<std::chrono::high_resolution_clock> end)
{
    // Avoid garbage timing on first call by initializing a new interval:
    auto count = m_threadInfo->lastReportTime.time_since_epoch().count();
    if (count == 0)
    {
        m_threadInfo->lastReportTime = m_start;
        return;
    }
    
    // Enough time has elapsed. Print statistics to console:
    float interval = std::chrono::duration_cast<std::chrono::microseconds>(end - m_threadInfo->lastReportTime).count();
    float measured = m_threadInfo->accumulator * 0.001;
    
    std::stringstream sstream;
    sstream << "Thread " << sofa::simulation::WorkerThreadLockFree::getCurrent()->getName()
    << " time spent in " << m_threadInfo->name << ": " << measured/m_threadInfo->hitCount << " [microsec] avarage   "
    << measured << " / " << interval << " us " << 100.f * measured / interval << "%  " << m_threadInfo->hitCount << "x\n";
    
//    for (auto iter : m_threadInfo->timeIntervals)
//    {
//        sstream << 0.001f * iter << ", ";
//    }
    sstream << "\n";
    
    std::cout << sstream.str();
    
    // Reset statistics and begin next timing interval:
    m_threadInfo->timeIntervals.clear();
    m_threadInfo->lastReportTime = end;
    m_threadInfo->accumulator = 0;
    m_threadInfo->hitCount = 0;
}


#endif // ENABLE_TASK_SCHEDULER_PROFILER
