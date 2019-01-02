#ifndef Foundation_TaskQueue_h__
#define Foundation_TaskQueue_h__


#include <atomic>
#include <cassert>
#include <vector>

namespace async
{


    /**
     * Circular buffer using std::vector
     * the initial size of the buffer must be a power of 2 for fast module computation
     */
    template<typename T>
    class CircularBuffer
    {
    public:
        
        CircularBuffer(std::size_t n)
        : _data(n)
        {
            assert(!(n == 0) && !(n & (n - 1)) && "n must be a power of 2");
        }
        
    public:
        std::size_t size() const
        {
            return _data.size();
        }
        
        T get(std::size_t index)
        {
            return _data[index & (size() - 1)];
        }
        
        void insert(std::size_t index, T elem)
        {
            _data[index & (size() - 1)] = elem;
        }
        
        // Growing the array returns a new circular_array object and keeps a linked list of all previous arrays.
        // This is done because other threads could still be accessing elements from the smaller arrays.
        CircularBuffer* resize(std::size_t top, std::size_t bottom)
        {
            CircularBuffer<T> *new_buffer = new CircularBuffer<T>(size() * 2);
            new_buffer->_previous.reset(this);
            for (std::size_t i = top; i != bottom; ++i)
            {
                new_buffer->insert(i, get(i));
            }
            return new_buffer;
        }
        
    private:
        std::vector<T> _data;
        
        // other threads could still be accessing elements from the smaller arrays.
        std::unique_ptr<CircularBuffer<T> > _previous;
    };
    

    
    /**
     * This is an implementation of 'Correct and Efficient Work-Stealing for Weak Memory Models' by Le et. al [2013]
     *
     * https://hal.inria.fr/hal-00802885
     */
    template<typename T>
    class LockFreeDeQueue
    {
    public:
        
        LockFreeDeQueue(std::size_t size)
        : _top(1)
        , _bottom(1)
        , _buffer(new CircularBuffer<T>(size)) {
        }
        
        
        ~LockFreeDeQueue()
        {
            delete _buffer.load(std::memory_order_relaxed);
        }
        
        std::uint64_t getCount() const
        {
            return _bottom.load(std::memory_order_relaxed) - _top.load(std::memory_order_relaxed);
        }
        
        void push(T elem)
        {
            std::uint64_t bottom = _bottom.load(std::memory_order_relaxed);
            std::uint64_t top = _top.load(std::memory_order_acquire);
            CircularBuffer<T>* buffer = _buffer.load(std::memory_order_relaxed);
            
            if (bottom - top > buffer->size() - 1) {
                /* Full queue. */
                buffer = buffer->resize(top, bottom);
                _buffer.store(buffer, std::memory_order_release);
            }
            buffer->insert(bottom, elem);
            
            std::atomic_thread_fence(std::memory_order_release);

            _bottom.store(bottom + 1, std::memory_order_relaxed);
        }
        
        bool pop(T* elem)
        {
            std::uint64_t bottom = _bottom.load(std::memory_order_relaxed) - 1;
            CircularBuffer<T>* buffer = _buffer.load(std::memory_order_relaxed);
            _bottom.store(bottom, std::memory_order_relaxed);
            
            std::atomic_thread_fence(std::memory_order_seq_cst);
            
            std::uint64_t top = _top.load(std::memory_order_relaxed);
            bool result = true;
            if (top <= bottom) {
                /* Non-empty queue. */
                *elem = buffer->get(bottom);
                if (top == bottom)
                {
                    /* Single last element in queue. */
                    if (!std::atomic_compare_exchange_strong_explicit(&_top, &top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
                    {
                        /* Failed race. */
                        result = false;
                    }
                    _bottom.store(bottom + 1, std::memory_order_relaxed);
                }
            } else {
                /* Empty queue. */
                result = false;
                _bottom.store(bottom + 1, std::memory_order_relaxed);
            }
            
            return result;
        }
        
        bool steal(T* elem)
        {
            std::uint64_t top = _top.load(std::memory_order_acquire);

            std::atomic_thread_fence(std::memory_order_seq_cst);
            
            std::uint64_t bottom = _bottom.load(std::memory_order_acquire);
            if (top < bottom) {
                /* Non-empty queue. */
                CircularBuffer<T>* buffer = _buffer.load(std::memory_order_consume);
                *elem = buffer->get(top);
                if (!std::atomic_compare_exchange_strong_explicit(&_top, &top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
                {
                    /* Failed race. */
                    return false;
                }
                
                return true;
            }
            
            return false;
        }
        
        
    private:
        
        // pad to avoid cache contention
        char _pad0[64];
        
        std::atomic<std::uint64_t> _top;
        // Cache line pad (128)
        char _pad1[64];
        
        std::atomic<std::uint64_t> _bottom;
        // Cache line pad (128)
        char _pad2[64];
        
        std::atomic<CircularBuffer<T>* > _buffer;
        // Cache line pad (128)
        char _pad3[64];
    };


} // namespace async



#endif // Foundation_TaskQueue_h__
