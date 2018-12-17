#include "TaskSchedulerLockFree.h"

#include "TaskSchedulerProfiler.h"

#include "InitTasks.h"

#include <sofa/helper/system/thread/CTime.h>

#include <string>  

namespace sofa
{

	namespace simulation
	{
        
        DEFINE_TASK_SCHEDULER_PROFILER(Push);
        DEFINE_TASK_SCHEDULER_PROFILER(Pop);
        DEFINE_TASK_SCHEDULER_PROFILER(Steal);
        
        static thread_local WorkerThreadLockFree* workerThreadIndex = nullptr;

        //std::map< std::thread::id, WorkerThreadLockFree*> TaskSchedulerLockFree::_threads;


		TaskSchedulerLockFree& TaskSchedulerLockFree::getInstance()
		{
			static TaskSchedulerLockFree instance;

			return instance;
		}

		TaskSchedulerLockFree::TaskSchedulerLockFree()
		{
			_isInitialized = false;
			_threadCount = 0;
			_isClosing = false;

            // init global static thread local var
            workerThreadIndex = new WorkerThreadLockFree(this, 0, "Main  ");
            _threads[std::this_thread::get_id()] = workerThreadIndex;// new WorkerThreadLockFree(this, 0, "Main  ");
		}

		TaskSchedulerLockFree::~TaskSchedulerLockFree()
		{
			if ( _isInitialized ) 
			{
				stop();
			}
		}

		unsigned TaskSchedulerLockFree::GetHardwareThreadsCount()
		{
			return std::thread::hardware_concurrency();
		}


		const WorkerThreadLockFree* TaskSchedulerLockFree::getWorkerThread(const std::thread::id id)
		{
			auto thread =_threads.find(id);
			if (thread == _threads.end() )
			{
				return nullptr;
			}
			return thread->second;
		}

        void TaskSchedulerLockFree::init(const unsigned int NbThread )
        {
            if ( _isInitialized )
            {
                if ( NbThread == _threadCount )
                {
                    return;
                }
                stop();
            }
            
            start(NbThread);

			// initialized per thread local data
			//initThreadLocalData();

        }
        
		void TaskSchedulerLockFree::start(const unsigned int NbThread )
		{
			stop();

            _isClosing = false;
            _workerThreadsIdle = false;
            _mainTaskStatus	= nullptr;
            
            // only physicsal cores. no advantage from hyperthreading.
            _threadCount = GetHardwareThreadsCount() / 2;
            
            if ( NbThread > 0 && NbThread <= MAX_THREADS  )
            {
                _threadCount = NbThread;
            }

            /* start worker threads */
            for( unsigned int i=1; i<_threadCount; ++i)
            {
				WorkerThreadLockFree* thread = new WorkerThreadLockFree(this, i);
				thread->create_and_attach(this);
				_threads[thread->getId()] = thread;
				thread->start(this);
            }
            
            _workerThreadCount = _threadCount;
            _isInitialized = true;

            return;
		}



		void TaskSchedulerLockFree::stop()
		{
			_isClosing = true;

			if ( _isInitialized ) 
			{
				// wait for all
				WaitForWorkersToBeReady();
				wakeUpWorkers();
                _isInitialized = false;

                // cpu busy wait
				//for(unsigned int i=1; i<_threadCount; ++i)
				//{
				//	while (!_threads[i]->isFinished())
				//	{
    //                    std::this_thread::yield();
				//	}
				//}
                
				// can't use C++11 for rqnge iterqtor because of std::unique_ptr
				//std::unordered_map< std::thread::id, WorkerThreadLockFree* >::const_iterator it = _threads.begin();
				//while (it != _threads.end())
				for (auto it : _threads)
				{
					// if this is the main thread continue
					if (std::this_thread::get_id() == it.first)
					{
						continue;
					}

					// cpu busy wait
					while (!it.second->isFinished())
					{
						std::this_thread::yield();
					}

					// free memory
					// cpu busy wait: thread.joint call
					delete it.second;
					it.second = nullptr;
				}

                _threadCount = 1;
				_workerThreadCount = 1;
                //_threads.resize(1);

				auto mainThreadIt = _threads.find(std::this_thread::get_id());
				WorkerThreadLockFree* mainThread = mainThreadIt->second;
				_threads.clear();
				_threads[std::this_thread::get_id()] = mainThread;
			}

			return;
		}

		void TaskSchedulerLockFree::wakeUpWorkers()
		{
			{
				std::lock_guard<std::mutex> guard(_wakeUpMutex);
				_workerThreadsIdle = false;
			}
			_wakeUpEvent.notify_all();
		}

		void TaskSchedulerLockFree::WaitForWorkersToBeReady()
		{
			_workerThreadsIdle = true;
		}


		unsigned TaskSchedulerLockFree::size()	const
		{
			return _workerThreadCount;
		}

		



		WorkerThreadLockFree::WorkerThreadLockFree(TaskSchedulerLockFree* const& pScheduler, const int index, const std::string& name)
        : _tasks(Max_TasksPerThread)
        , _index(index)
        , _name(name + std::to_string(index))
        , _taskScheduler(pScheduler)
        , _taskAllocator(this)
		{
			assert(pScheduler);
			_finished.store(false);
			_currentStatus = nullptr;
		}


		WorkerThreadLockFree::~WorkerThreadLockFree()
		{
            if (_stdThread.joinable())
            {
                _stdThread.join();
            }
		}		

        void* WorkerThreadLockFree::allocate()
        {
            void* mem = _taskAllocator.allocate();
            return mem;
        }
        
        void WorkerThreadLockFree::free(void* ptr)
        {
            TasksAllocator<256,16>::MemoryPool*  mempool = reinterpret_cast<TasksAllocator<256,16>::MemoryPool*>(ptr);
            WorkerThreadLockFree* workerThread = const_cast<WorkerThreadLockFree*>(mempool->_threadPtr);
            workerThread->_taskAllocator.free(ptr);
        }
        
        bool WorkerThreadLockFree::attachToThisThread(TaskSchedulerLockFree* /*pScheduler*/)
		{
			_finished.store(false);
            //TaskSchedulerLockFree::workerThreadIndex = this;
			return true;
		}

        bool WorkerThreadLockFree::isFinished()
        {
            return _finished.load();
        }

		bool WorkerThreadLockFree::start(TaskSchedulerLockFree* const& taskScheduler)
		{
			assert(taskScheduler);
			_taskScheduler = taskScheduler;
			_currentStatus = nullptr;

			return  true;
		}

        std::thread* WorkerThreadLockFree::create_and_attach( TaskSchedulerLockFree* const & taskScheduler)
        {
            _stdThread = std::thread(std::bind(&WorkerThreadLockFree::run, this));
            return &_stdThread;
        }

		WorkerThreadLockFree* WorkerThreadLockFree::getCurrent()
		{
            return workerThreadIndex;
            //auto thread = TaskSchedulerLockFree::_threads.find(std::this_thread::get_id());
            //if (thread == TaskSchedulerLockFree::_threads.end())
            //{
            //    return nullptr;
            //}
            //return thread->second;
		}


		void WorkerThreadLockFree::run(void)
		{

			// Thread Local Storage 
            workerThreadIndex = this;
            //TaskSchedulerLockFree::_threads[std::this_thread::get_id()] = this;

			// main loop
            while ( !_taskScheduler->isClosing() )
			{
				Idle();
                
                while ( _taskScheduler->_mainTaskStatus != nullptr)
				{
				
					doWork(0);

				
					if (_taskScheduler->isClosing() ) 
						break;
				}
			}

			_finished.store(true);

			return;
		}

        const std::thread::id WorkerThreadLockFree::getId()
        {
            return _stdThread.get_id();
        }
        
        void WorkerThreadLockFree::Idle()
        {
			std::unique_lock<std::mutex> lock(_taskScheduler->_wakeUpMutex);
			if (!_taskScheduler->_workerThreadsIdle)
			{
				return;
			}
			// cpu free wait
			_taskScheduler->_wakeUpEvent.wait(lock);
            return;
        }

		void WorkerThreadLockFree::doWork(TaskLockFree::Status* status)
		{

            for (;;)// do
            {
                TaskLockFree* task;

                while (popTask(&task))
                {
                    // run
                    runTask(task);


                    if (status && !status->isBusy())
                        return;
                }

                /* check if main work is finished */
                if (_taskScheduler->_mainTaskStatus == nullptr)
                    return;

                if (!stealTask(&task))
                    return;

                // run stolen task
                runTask(task);

            } //;;while (stealTasks());	

		
			return;

		}


		void WorkerThreadLockFree::workUntilDone(TaskLockFree::Status* status)
		{
			while (status->isBusy())
			{
				doWork(status);
			}

			if (_taskScheduler->_mainTaskStatus == status)
			{
				_taskScheduler->_mainTaskStatus = nullptr;
			}
		}

        void WorkerThreadLockFree::runTask(TaskLockFree* task)
        {
            TaskLockFree::Status* prevStatus = _currentStatus;
            _currentStatus = task->getStatus();

            {
                TaskLockFree::Memory ret = task->run(this);
                if (ret == TaskLockFree::Memory::Dynamic)
                {
                    // pooled memory: call destructor and free
                    task->~TaskLockFree();
                    delete task;
                }
            }

            _currentStatus->markBusy(false);
            _currentStatus = prevStatus;
        }

		bool WorkerThreadLockFree::popTask(TaskLockFree** task)
		{
            TASK_SCHEDULER_PROFILER(Pop);
            
            return _tasks.pop(task);
//            if (!_tasks.pop(task) )
//            {
//                return false;
//            }
//            return true;
		}


		bool WorkerThreadLockFree::pushTask(TaskLockFree* task)
		{
            // if we're single threaded return false
            if ( _taskScheduler->getThreadCount()<2 )
            {
                return false;
            }
            
            task->getStatus()->markBusy(true);
            {
                TASK_SCHEDULER_PROFILER(Push);
                _tasks.push(task);
            }
            
            if (!_taskScheduler->_mainTaskStatus)
            {
                _taskScheduler->_mainTaskStatus = task->getStatus();
                _taskScheduler->wakeUpWorkers();
            }
            
            return true;
		}

		bool WorkerThreadLockFree::addTask(TaskLockFree* task)
		{
			if (pushTask(task))
            {
                return true;
            }
			
            // we are single thread: run the task
            {
                //TASK_SCHEDULER_PROFILER(RunTask);
                runTask(task);
            }
            
			return false;
		}


		bool WorkerThreadLockFree::giveUpSomeWork(WorkerThreadLockFree* idleThread)
		{
            TaskLockFree* stealedTask = nullptr;
			//_stolenTask = nullptr;
			if (_tasks.steal(&stealedTask))// &idleThread->_stolenTask) )
            {
                idleThread->_tasks.push(stealedTask);
                return true;
            }
            return false;
		}

        bool WorkerThreadLockFree::giveUpSomeWork(TaskLockFree** stolenTask)
        {
            TASK_SCHEDULER_PROFILER(Steal);
            return _tasks.steal(stolenTask);
        }

		bool WorkerThreadLockFree::stealTask()
		{
            //int Offset = GetWorkerIndex();
            
			// can't use C++11 for rqnge iterqtor because of std::unique_ptr
			//std::unordered_map< std::thread::id, std::unique_ptr<WorkerThreadLockFree> >::const_iterator it = TaskSchedulerLockFree::_threads.begin();
			//while (it != TaskSchedulerLockFree::_threads.end())
			for (auto it : _taskScheduler->_threads)
			{
				// if this is the main thread continue
				if (std::this_thread::get_id() == it.first)
				{
					continue;
				}

				if (it.second->giveUpSomeWork(this))
				{
					return true;
				}
			}

			return false;
		}

        bool WorkerThreadLockFree::stealTask(TaskLockFree** task)
        {
            for (auto it : _taskScheduler->_threads)
            {
                // if this is the main thread continue
                if (std::this_thread::get_id() == it.first)
                {
                    continue;
                }
                
                if (it.second->giveUpSomeWork(task))
                {
                    return true;
                }
            }
            return false;
        }
		
		// called once by each thread used
		// by the TaskScheduler
		bool runThreadSpecificTask(WorkerThreadLockFree* thread, const TaskLockFree * /*task*/ )
		{
            std::atomic<int> atomicCounter;
            atomicCounter = TaskSchedulerLockFree::getInstance().size();
            
            std::mutex  InitThreadSpecificMutex;
            
            TaskLockFree::Status status;
            
            const int nbThread = TaskSchedulerLockFree::getInstance().size();
            
            for (int i=0; i<nbThread; ++i)
            {
                thread->addTask( new ThreadSpecificTaskLockFree( &atomicCounter, &InitThreadSpecificMutex, &status ) );
            }
            
            thread->workUntilDone(&status);

			return true;
		}


		// called once by each thread used
		// by the TaskScheduler
		bool runThreadSpecificTask(const TaskLockFree *task )
		{
			return runThreadSpecificTask(WorkerThreadLockFree::getCurrent(), task );
		}




	} // namespace simulation

} // namespace sofa
