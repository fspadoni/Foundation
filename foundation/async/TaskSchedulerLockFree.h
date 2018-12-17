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
#ifndef TaskSchedulerPOC_h__
#define TaskSchedulerPOC_h__

#include <Performance/config.h>

#include "TasksLockFree.h"
#include "LockFreeDeQueue.h"
#include "TasksAllocator.h"

#include <thread>
#include <memory>
#include <atomic>
#include <map>
//#include <vector>

namespace sofa
{

	namespace simulation
	{

		class TaskSchedulerLockFree;
		class WorkerThreadLockFree;
        class TasksAllocators;

        
		class SOFA_PERFORMANCE_API WorkerThreadLockFree
		{
		public:

			WorkerThreadLockFree(TaskSchedulerLockFree* const& taskScheduler, const int index, const std::string& name = "Worker");

			~WorkerThreadLockFree();

			static WorkerThreadLockFree* getCurrent();
            
            static WorkerThreadLockFree* getThread();

			// queue task if there is space, and run it otherwise
			bool addTask(TaskLockFree* pTask);

			void workUntilDone(TaskLockFree::Status* status);

			TaskLockFree::Status* getCurrentStatus() const {return _currentStatus;}

            const char* getName() { return _name.c_str(); }

            const size_t getIndex() { return _index; }

            const std::thread::id getId();
            
            const LockFreeDeQueue<TaskLockFree*>* getTasksQueue() {return &_tasks;}
            
			std::uint64_t getTaskCount()  {return _tasks.getCount(); }
            
            int GetWorkerIndex();
            
            void* allocate();
            
            void free(void* ptr);
            
            
		private:

			bool start(TaskSchedulerLockFree* const& taskScheduler);

			std::thread* create_and_attach( TaskSchedulerLockFree* const& taskScheduler);

            void runTask(TaskLockFree* task);

			// queue task if there is space (or do nothing)
			bool pushTask(TaskLockFree* pTask);

			// pop task from queue
			bool popTask(TaskLockFree** ppTask);
			
			// steal and queue some task from another thread 
			bool stealTask();

            bool stealTask(TaskLockFree** task);

			// give an idle thread some work
			bool giveUpSomeWork(WorkerThreadLockFree* pIdleThread);
			
            bool giveUpSomeWork(TaskLockFree** stolenTask);

			void doWork(TaskLockFree::Status* status);

			// boost thread main loop
			void run(void);


			//void	ThreadProc(void);
			void	Idle(void);

			bool attachToThisThread(TaskSchedulerLockFree* pScheduler);

            bool isFinished();
            
		private:

			enum 
			{
				Max_TasksPerThread = 256
			};

            const std::string _name;

            const size_t _index;

            LockFreeDeQueue<TaskLockFree*> _tasks;      

            std::thread  _stdThread;
            
			TaskLockFree::Status*	_currentStatus;

			TaskSchedulerLockFree*     _taskScheduler;
			
            TasksAllocator<256,64> _taskAllocator;
            
			// The following members may be accessed by _multiple_ threads at the same time:
			std::atomic<bool>	_finished;


			friend class TaskSchedulerLockFree;
		};




		class SOFA_PERFORMANCE_API TaskSchedulerLockFree

		{
			enum
			{
				MAX_THREADS = 16,
				STACKSIZE = 64*1024 /* 64K */,
			};

		public:
			
			static TaskSchedulerLockFree& getInstance();
			
            void init(const unsigned int NbThread = 0);
            
            bool isInitialized() { return _isInitialized; }

			bool isClosing(void) const { return _isClosing; }

			unsigned int getThreadCount(void) const { return _threadCount; }

			void	WaitForWorkersToBeReady();

			void	wakeUpWorkers();

			static unsigned GetHardwareThreadsCount();

			unsigned size()	const;

			const WorkerThreadLockFree* getWorkerThread(const std::thread::id id);
		
			void stop(void);
            void start(unsigned int NbThread);

		private:
			
            //static thread_local WorkerThreadLockFree* _workerThreadIndex;

			std::map< std::thread::id, WorkerThreadLockFree*> _threads;

			TaskLockFree::Status*	_mainTaskStatus;

			std::mutex  _wakeUpMutex;

			std::condition_variable _wakeUpEvent;

		private:

			TaskSchedulerLockFree();
			
			TaskSchedulerLockFree(const TaskSchedulerLockFree& ) {}

			~TaskSchedulerLockFree();


			



			bool _isInitialized;
            
			unsigned _workerThreadCount;

			bool						_workerThreadsIdle;

			bool _isClosing;

			unsigned					_threadCount;

			friend class WorkerThreadLockFree;
		};		



		

		SOFA_PERFORMANCE_API bool runThreadSpecificTask(WorkerThreadLockFree* pThread, const TaskLockFree *pTask );

		SOFA_PERFORMANCE_API bool runThreadSpecificTask(const TaskLockFree *pTask );





	} // namespace simulation

} // namespace sofa


#endif // TaskSchedulerPOC_h__
