//
//  CCTaskScheduler.cpp
//  cocos2d_libs
//
//  Created by Federico Spadoni on 29/01/15.
//
//

#include "TaskScheduler.h"

namespace foundation
{


//std::__thread_specific_ptr<WorkerThread> TaskScheduler::_workerThreadIndex;
thread_local WorkerThread* TaskScheduler::_workerThreadIndex = nullptr;
//pthread_key_t TaskScheduler::_workerThreadIndex;


TaskScheduler& TaskScheduler::getInstance()
{
    static TaskScheduler instance;
    
    return instance;
}

TaskScheduler::TaskScheduler()
{
    _isInitialized = false;
    _threadCount = 0;
    _isClosing = false;
    
    _readyForWork = false;
    
    _threads[0] = new WorkerThread( this );
    _threads[0]->attachToThisThread( this );
    
}

TaskScheduler::~TaskScheduler()
{
    if ( _isInitialized )
    {
        stop();
    }
    
//    pthread_key_delete(TaskScheduler::_workerThreadIndex);
}

unsigned TaskScheduler::GetHardwareThreadsCount()
{
    return std::thread::hardware_concurrency();
}


WorkerThread* TaskScheduler::getWorkerThread(const unsigned int index)
{
    WorkerThread* thread = _threads[index];
    if ( index >= _threadCount )
    {
        return thread = 0;
    }
    return thread;
}

bool TaskScheduler::start(const unsigned int NbThread )
{
    
    if ( _isInitialized )
    {
        stop();
    }
    
    //if ( !_isInitialized )
    {
        _isClosing		= false;
        _workersIdle		= false;
        _mainTaskStatus	= NULL;
        
        // only physicsal cores. no advantage from hyperthreading.
        _threadCount = GetHardwareThreadsCount();
        
        if ( NbThread > 0 && NbThread <= MAX_THREADS  )
        {
            _threadCount = NbThread;
        }
        
//        std::out("TaskScheduler: number thread %d ", _threadCount);
        //_thread[0] =  new WorkerThread( this ) ;
        //_thread[0]->attachToThisThread( this );
        
        /* start worker threads */
        for( unsigned int iThread=1; iThread<_threadCount; ++iThread)
        {
            //_thread[iThread] = boost::shared_ptr<WorkerThread>(new WorkerThread(this) );
            _threads[iThread] = new WorkerThread(this);
            _threads[iThread]->create_and_attach( this );
            _threads[iThread]->start( this );
        }
        
        _workerCount = _threadCount;
        _isInitialized = true;
        return true;
    }
    //else
    //{
    //	return false;
    //}
    
}



bool TaskScheduler::stop()
{
    unsigned iThread;
    
    _isClosing = true;
    
    if ( _isInitialized )
    {
        // wait for all
        WaitForWorkersToBeReady();
        wakeUpWorkers();
        
        for(iThread=1; iThread<_threadCount; ++iThread)
        {
            while (!_threads[iThread]->_finished)
            {
                //_thread[iThread]->join();
                //WorkerThread::release( _thread[iThread] );
                //_thread[iThread].reset();
            }
        }
        for(iThread=1; iThread<_threadCount; ++iThread)
        {
//            _threads[iThread]->release();
            _threads[iThread] = nullptr;
        }
        
        
        _isInitialized = false;
        _workerCount = 1;
    }
    
    
    return true;
}



void TaskScheduler::wakeUpWorkers()
{
    
    _workersIdle = false;
    
    {
//        std::lock_guard<std::mutex> lock(wakeUpMutex);
//        readyForWork = true;
    }
    
    _wakeUpEvent.notify_all();
    
}


void TaskScheduler::WaitForWorkersToBeReady()
{
    
    for(unsigned i=0; i<_threadCount-1; ++i)
    {}
    
    _workersIdle = true;
}




unsigned TaskScheduler::size()	const
{
    return _workerCount;
}



WorkerThread::WorkerThread(TaskScheduler* const& pScheduler)
: _taskScheduler(pScheduler)
//, mTaskMutex(ATOMIC_FLAG_INIT)
{
//    assert(pScheduler);
    
    _taskCount		= 0;
    _finished		= false;
    _currentStatus = NULL;
//    mTaskMutex.v_ = 0L;
}


WorkerThread::~WorkerThread()
{
    _thread.join();
}

bool WorkerThread::attachToThisThread(TaskScheduler* pScheduler)
{
    
    _taskCount		= 0;
    _finished		= false;
    
//    TaskScheduler::_workerThreadIndex->reset( this );
    TaskScheduler::_workerThreadIndex = this;
//    pthread_key_create(&TaskScheduler::_workerThreadIndex, nullptr);
//    pthread_setspecific(TaskScheduler::_workerThreadIndex, this);
    
    return true;
}



bool WorkerThread::start(TaskScheduler* const& taskScheduler)
{
//    assert(taskScheduler);
    _taskScheduler = taskScheduler;
    _currentStatus = NULL;
    
    return true;
}




std::thread* WorkerThread::create_and_attach( TaskScheduler* const & taskScheduler)
{
    
    _thread = std::thread(std::bind(&WorkerThread::run, this));

    return &_thread;
}


bool WorkerThread::releaseThread()
{
    
//    if ( _thread = 0 )
    {
        _thread.join();
        
        return true;
    }
    
    return false;
}


WorkerThread* WorkerThread::getCurrent()
{
//    return (WorkerThread*)pthread_getspecific( TaskScheduler::_workerThreadIndex );
    return TaskScheduler::_workerThreadIndex;
//    return TaskScheduler::_workerThreadIndex.get();
}


void WorkerThread::run(void)
{
    
    // Thread Local Storage
//    TaskScheduler::_workerThreadIndex.reset( this );
    TaskScheduler::_workerThreadIndex = this;
//    pthread_setspecific(TaskScheduler::_workerThreadIndex, this);
    
    // main loop
    for(;;)
    {
        Idle();
        
        if ( _taskScheduler->isClosing() )
            break;
        
        
        while ( !_taskScheduler->_mainTaskStatus->isBusy() )
        {
//            if ( !mTaskScheduler->mainTaskStatus->isBusy() )
//                break;
            
            doWork(0);
            
            
            if (_taskScheduler->isClosing() )
            break;
        }
        
        _taskScheduler->_mainTaskStatus = nullptr;
        
    }
    
    _finished = true;
    return;
}


std::thread::id WorkerThread::getId()
{
    return _thread.get_id();
}


int WorkerThread::GetWorkerIndex()
{
    return int(this - *_taskScheduler->_threads);
}


void WorkerThread::Idle()
{
    {
        std::unique_lock<std::mutex> lock( _taskScheduler->_wakeUpMutex );
        
//    book: Effective Modern C++ Scott Meyers pag. 283-284
//        mTaskScheduler->wakeUpEvent.wait(lock, [] {return mTaskScheduler->readyForWork;} );
        
//        while(!mTaskScheduler->readyForWork)
        {
            _taskScheduler->_wakeUpEvent.wait(lock);
            
        }
        
    }
    return;
}




void WorkerThread::doWork(Task::Status* status)
{
    
    do
    {
        Task*		pTask;
        Task::Status*	pPrevStatus = nullptr;
        
        while (popTask(&pTask))
        {
            // run
            pPrevStatus = _currentStatus;
            _currentStatus = pTask->getStatus();
            
            pTask->run(this);
            
            _currentStatus->markBusy(false);
            _currentStatus = pPrevStatus;
            
            if ( status && !status->isBusy() )
            return;
        }
        
        /* check if main work is finished */
        if (!_taskScheduler->_mainTaskStatus->isBusy())
            return;
        
    } while (stealTasks());
    
    
    return;
    
}


void WorkerThread::workUntilDone(Task::Status* status)
{
    //PROFILE_SYNC_PREPARE( this );
    
    while (status->isBusy())
    {
//        std::this_thread::yield();
        doWork(status);
    }
    
    //PROFILE_SYNC_CANCEL( this );
    
    if (_taskScheduler->_mainTaskStatus == status)
    {
        
        _taskScheduler->_mainTaskStatus = nullptr;
        

//        std::lock_guard<std::mutex> lock(mTaskScheduler->wakeUpMutex);
        
        _taskScheduler->_readyForWork = false;
        
        
//        {
//            std::unique_lock<std::mutex> lock(mTaskScheduler->wakeUpMutex);
//            cv.wait(lk, []{return processed;});
//            
//        }

//        // wait for the worker
//        {
//            std::unique_lock<std::mutex> lk(m);
//            cv.wait(lk, []{return processed;});
//        }
    }
}


bool WorkerThread::popTask(Task** outTask)
{
//    SpinLockMutex lock( &mTaskMutex );
    SpinLock::ScopedLock lock( _taskSpinLock );
    
    //
    if (_taskCount == 0)
        return false;
    
    Task* task = _tasks[_taskCount-1];
    
    /* Check if we can pop a partial-task (ex: one iteration of a loop) */
    if (task->partialPop(this, outTask))
    {
        task->getStatus()->markBusy(true);
        return true;
    }
    
    // pop from top of the pile
    *outTask = task;
    --_taskCount;
    return true;
}


bool WorkerThread::pushTask(Task* task)
{
    // if we're single threaded return false
    if ( _taskScheduler->getThreadCount()<2 )
        return false;
    
    /* if task pool is empty, try to spread subtasks across all threads */
    if (!_taskScheduler->_mainTaskStatus)
    {
        /* check we're indeed the main thread			*/
        /* (no worker can push job, no task is queued)	*/
//        CCASSERT( GetWorkerIndex()==0,  "no worker can push job, no task is queued");
        
        /* Ready ? */
        _taskScheduler->WaitForWorkersToBeReady();
        
        /* Set... */
        if (task->spread( _taskScheduler ))
        {	/* Go! Mark this task as the root task (see WorkUntilDone) */
            _taskScheduler->_mainTaskStatus = task->getStatus();
            _taskScheduler->wakeUpWorkers();
            return true;
        }
    }
    
    
    {
//        SpinLockMutex lock( &mTaskMutex );
        SpinLock::ScopedLock lock( _taskSpinLock );
        
        
        if (_taskCount >= Max_TasksPerThread )
            return false;
        
        
        task->getStatus()->markBusy(true);
        _tasks[_taskCount] = task;
        ++_taskCount;
    }
    
    
    if (!_taskScheduler->_mainTaskStatus)
    {
        _taskScheduler->_mainTaskStatus = task->getStatus();
        _taskScheduler->wakeUpWorkers();
    }
    
    return true;
}

bool WorkerThread::addTask(Task* task)
{
    if (pushTask(task))
        return true;
    
    
    task->run(this);
        return false;
}


bool WorkerThread::giveUpSomeWork(WorkerThread* idleThread)
{
//    SpinLockMutex lock;
    
//    try lock and remember to unlock at the end of the function
    if ( !_taskSpinLock.try_lock() )
    {
        return false;
    }
//    if ( !lock.try_lock( &mTaskMutex ) )
//        return false;
    
	
    if ( _taskCount == 0 )
    {
//    unlock
        _taskSpinLock.unlock();
        return false;
    }
    
//    SpinLockMutex	lockIdleThread( &idleThread->mTaskMutex );
    SpinLock::ScopedLock lock( idleThread->_taskSpinLock );
    
    if ( idleThread->_taskCount )
    {
        _taskSpinLock.unlock();
        return false;
    }
    
    
    /* if only one task remaining, try to split it */
    if (_taskCount==1)
    {
        Task* task;
        
        task = NULL;
        if (_tasks[0]->split( idleThread, &task ) )
        {
            task->getStatus()->markBusy(true);
            
            idleThread->_tasks[0] = task;
            idleThread->_taskCount = 1;
            
            _taskSpinLock.unlock();
            return true;
        }
    }
    
    unsigned int count = (_taskCount+1) /2;
    
    
    Task** p = idleThread->_tasks;
    
    unsigned int iTask;
    for( iTask=0; iTask< count; ++iTask)
    {
        *p++ = _tasks[iTask];
        _tasks[iTask] = NULL;
    }
    idleThread->_taskCount = count;
    
    
    for( p = _tasks; iTask<_taskCount; ++iTask)
    {
        *p++ = _tasks[iTask];
    }
    _taskCount -= count;
    
    
//    unlock
    _taskSpinLock.unlock();
    
    return true;
}


bool WorkerThread::stealTasks()
{
    
	int Offset = GetWorkerIndex();
    
    for( unsigned int iThread=0; iThread<_taskScheduler->getThreadCount(); ++iThread)
    {
        //WorkerThread*	pThread;
        
        WorkerThread* pThread = _taskScheduler->_threads[ (iThread+Offset) % _taskScheduler->getThreadCount() ];
       
        if ( pThread == this)
            continue;
        
        if ( pThread->giveUpSomeWork(this) )
            return true;
        
        if ( _taskCount > 0 )
            return true;
    }
    
    return false;
}



// called once by each thread used
// by the TaskScheduler
bool runThreadSpecificTask(WorkerThread* thread, const Task *task )
{
    
    
    //volatile long atomicCounter = TaskScheduler::getInstance().size();// mNbThread;
//    helper::system::atomic<int> atomicCounter( TaskScheduler::getInstance().size() );
    std::atomic<int> atomicCounter;
    atomicCounter = TaskScheduler::getInstance().size();
    
    std::mutex  InitThreadSpecificMutex;
    
    Task::Status status;
    
    const int nbThread = TaskScheduler::getInstance().size();
    
    for (int i=0; i<nbThread; ++i)
    {
        thread->addTask( new ThreadSpecificTask( &atomicCounter, &InitThreadSpecificMutex, &status ) );
    }
    
    
    thread->workUntilDone(&status);
    
    return true;
}


// called once by each thread used
// by the TaskScheduler
bool runThreadSpecificTask(const Task *task )
{
    return runThreadSpecificTask(WorkerThread::getCurrent(), task );
}


 } // namespace foundation
