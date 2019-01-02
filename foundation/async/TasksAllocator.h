#ifndef Foundation_TasksAllocator_h__
#define Foundation_TasksAllocator_h__

#include <atomic>

#include <async/Locks.h>

namespace async
{

    class WorkerThreadLockFree;
    
//        class TaskMemoryChunk
//        {
//
//        };
    
    template<size_t TASK_SIZE, size_t TASK_COUNT>
    class TasksAllocator
    {
        enum
        {
//                Task_MAX_SIZE = 128, // bytes
//                Task_Buffer_Size = 64 // tasks
        };
        
//            class MemoryBlock
//            {
//                MemoryBlock* _nextBlock;
//                unsigned int _threadIndex;
//                char _data[TASK_SIZE-sizeof(unsigned int)-sizeof(MemoryBlock*)];
//            };
        
//            std::set< std::array<MemoryBlock, TASK_COUNT> > _buffers;
    public:
        
        TasksAllocator(WorkerThreadLockFree* threadPtr);
        
        ~TasksAllocator();

        void* allocate();
        
        void free(void* ptr);
        
    
        // Private types
        struct MemoryPool
        {
            char _data[TASK_SIZE-sizeof(WorkerThreadLockFree*)-sizeof(MemoryPool*)];
            const WorkerThreadLockFree* _threadPtr;
            MemoryPool* _next = nullptr;
        };
        
    private:
        
        struct MemoryPoolHead
        {
            uintptr_t _aba_counter = 0;
            MemoryPool* _pool = nullptr;
            
            MemoryPoolHead()
                : _aba_counter(0)
                , _pool(nullptr)
            { }
        };
        
        struct MemoryPoolBlock
        {
            char* _buffer = nullptr;
            MemoryPoolBlock* _next = nullptr;
            
            MemoryPoolBlock()
            {
                _buffer = nullptr;
                _next = nullptr;
                
            }
            ~MemoryPoolBlock()
            {
                operator delete(_buffer);
                
            }
        };
        
        const WorkerThreadLockFree* _threadPtr;
        
        std::uint64_t _max_size = 0;
        MemoryPool* _last_pool = nullptr;
        MemoryPoolBlock* _memoryPoolBlock_head = nullptr;
        
        // pad to avoid cache contention
        char _pad0[64];
        std::atomic<MemoryPoolHead> _free;
        char _pad1[64];
        
        SpinLock _mutex;
        
        void allocateMemoryBlock();
        
    };
    

    template<size_t TASK_SIZE, size_t TASK_COUNT>
    TasksAllocator<TASK_SIZE, TASK_COUNT>::TasksAllocator(WorkerThreadLockFree* threadPtr)
    : _threadPtr(threadPtr)
    , _free( MemoryPoolHead() )
    {
        
    }
    
    template<size_t TASK_SIZE, size_t TASK_COUNT>
    TasksAllocator<TASK_SIZE, TASK_COUNT>::~TasksAllocator()
    {
        MemoryPoolBlock* current = _memoryPoolBlock_head;
        MemoryPoolBlock* next = nullptr;
        while (current != nullptr)
        {
            next = current->_next;
            delete current;
            current = next;
        }
    }
    
    template<size_t TASK_SIZE, size_t TASK_COUNT>
    void* TasksAllocator<TASK_SIZE, TASK_COUNT>::allocate()
    {
        MemoryPoolHead current = _free.load(std::memory_order_acquire);
        
        while(true)
        {
            if ( nullptr == current._pool )
            {
                allocateMemoryBlock();
                current = _free.load(std::memory_order_acquire);
            }
            
            MemoryPoolHead next;
            next._aba_counter = current._aba_counter + 1;
            next._pool = current._pool->_next;
            
            if (std::atomic_compare_exchange_weak_explicit(&_free, &current, next, std::memory_order_seq_cst, std::memory_order_relaxed) )
            {
                break;
            }
        }
        
        return current._pool;
    }
    
    template<size_t TASK_SIZE, size_t TASK_COUNT>
    void TasksAllocator<TASK_SIZE, TASK_COUNT>::free(void* ptr)
    {
        MemoryPool* mempool = reinterpret_cast<MemoryPool*>(ptr);
        
        MemoryPoolHead next;
        
        MemoryPoolHead current = _free.load(std::memory_order_acquire);
        while(true)
        {
            mempool->_next = current._pool;
            next._aba_counter = current._aba_counter + 1;
            next._pool = mempool;
            
            if (std::atomic_compare_exchange_weak_explicit(&_free, &current, next, std::memory_order_seq_cst, std::memory_order_relaxed))
            {
                break;
            }
        }
    }
    
    
    template<size_t TASK_SIZE, size_t TASK_COUNT>
    void TasksAllocator<TASK_SIZE, TASK_COUNT>::allocateMemoryBlock()
    {
//            ScopedLock lock( _mutex );
        
        // After coming out of the lock, if the condition that got us here is now false, we can safely return
        // and do nothing.  This means another thread beat us to the allocation.  If we don't do this, we could
        // potentially allocate an entire block_size of memory that would never get used.
//            if (_free.load()._pool != nullptr) { return; }
        
//            MemoryPoolHead current = _free.load(std::memory_order_acquire);
        
        MemoryPoolBlock* new_memoryBlock = new MemoryPoolBlock();
        new_memoryBlock->_next = _memoryPoolBlock_head;
        _memoryPoolBlock_head = new_memoryBlock;
        
        new_memoryBlock->_buffer = reinterpret_cast<char*>(operator new(TASK_COUNT * sizeof(MemoryPool)));
        
        // Pad block body to satisfy the alignment requirements for elements
        char* body = new_memoryBlock->_buffer + sizeof(MemoryPool*);
        size_t body_padding = ((alignof(MemoryPool) - reinterpret_cast<uintptr_t>(body)) % alignof(MemoryPool));
        char* start = body + body_padding;
        char* end = (new_memoryBlock->_buffer + (TASK_COUNT * sizeof(MemoryPool)));
        
        // Update the old last slot's next ptr to point to the first slot of the new block
        if (_last_pool)
            _last_pool->_next = reinterpret_cast<MemoryPool*>(start);
        
        // We'll never get exactly the number of objects requested, but it should be close.
        for (; (start + sizeof(MemoryPool)) < end; start += sizeof(MemoryPool))
        {
            reinterpret_cast<MemoryPool*>(start)->_threadPtr = _threadPtr;
            reinterpret_cast<MemoryPool*>(start)->_next = reinterpret_cast<MemoryPool*>(start + sizeof(MemoryPool));
            ++_max_size;
        }
        
        // "start" should now point to one byte past the end of the last slot.  Subtract the size of slot_t from it to
        // get a pointer to the beginning of the last slot.
        _last_pool = reinterpret_cast<MemoryPool*>(start - sizeof(MemoryPool));
        _last_pool->_next = nullptr;
        
        // Update the head of the free list to point to the start of the new block
        MemoryPoolHead first;
        first._aba_counter = 0;
        first._pool = reinterpret_cast<MemoryPool*>(body + body_padding);
        
        _free.store(first);
    }


} // namespace async

#include "TasksAllocator.inl"

#endif // Foundation_TasksAllocator_h__
