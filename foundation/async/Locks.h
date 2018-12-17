#ifndef Foundation_Locks_h__
#define Foundation_Locks_h__

#include <thread>
#include <atomic>

namespace async
{

    class SpinLock
    {
        enum
        {
            CACHE_LINE = 64
        };
        
    public:
        
        SpinLock()
        {}
        
        ~SpinLock()
        {
            unlock();
        }
        
        bool try_lock()
        {
            return !_flag.test_and_set( std::memory_order_acquire );
        }
        
        void lock()
        {
            while( _flag.test_and_set(std::memory_order_acquire) )
            {
                // cpu busy wait
                //std::this_thread::yield();
            }
        }
        
        void unlock()
        {
            _flag.clear( std::memory_order_release );
        }
        
    private:
        
        std::atomic_flag _flag = ATOMIC_FLAG_INIT;
        
        char _pad [CACHE_LINE - sizeof(std::atomic_flag)];
    };
    
    
    
    class ScopedLock
    {
    public:
        
        explicit ScopedLock( SpinLock & lock ): _spinlock( lock )
        {
            _spinlock.lock();
        }
        
        ~ScopedLock()
        {
            _spinlock.unlock();
        }
        
        ScopedLock( ScopedLock const & ) = delete;
        ScopedLock & operator=( ScopedLock const & ) = delete;
        
    private:
        
        SpinLock& _spinlock;
    };


} // namespace async


#endif // Foundation_Locks_h__
