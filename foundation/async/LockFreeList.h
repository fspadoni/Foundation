#ifndef Foundation_LockFreeList_h__
#define Foundation_LockFreeList_h__


#include <atomic>
#include <cassert>
#include <vector>
#include <thread>

namespace async
{
    
    static inline bool IsAligned( const volatile void* Ptr, const uint32_t Alignment = sizeof(void*) )
    {
        return !(sizeof(Ptr) & (Alignment - 1));
    }
    
//    void LockFreeTagCounterHasOverflowed();
//    void LockFreeLinksExhausted(uint32 TotalNum);
//    void* LockFreeAllocLinks(SIZE_T AllocSize);
//    void LockFreeFreeLinks(SIZE_T AllocSize, void* Ptr);
    
#define MAX_LOCK_FREE_LINKS_AS_BITS (26)
#define MAX_LOCK_FREE_LINKS (1 << 26)
    
    
    template<int TPaddingForCacheContention>
    struct FPaddingForCacheContention
    {
        std::uint8_t PadToAvoidContention[TPaddingForCacheContention];
    };
    
    template<>
    struct FPaddingForCacheContention<0>
    {
    };
    
    template<class T, unsigned int MaxTotalItems, unsigned int ItemsPerPage>
    class TLockFreeAllocOnceIndexedAllocator
    {
        enum
        {
            MaxBlocks = (MaxTotalItems + ItemsPerPage - 1) / ItemsPerPage
        };
    public:
        
        TLockFreeAllocOnceIndexedAllocator()
        {
//            NextIndex.Increment(); // skip the null ptr
            ++NextIndex;
            for (uint32_t Index = 0; Index < MaxBlocks; Index++)
            {
                Pages[Index] = nullptr;
            }
        }
        
        uint32_t Alloc(uint32_t Count = 1)
        {
//            uint32_t FirstItem = NextIndex.Add(Count);
            uint32_t FirstItem = NextIndex.fetch_add(Count, std::memory_order_relaxed);
            if (FirstItem + Count > MaxTotalItems)
            {
//                LockFreeLinksExhausted(MaxTotalItems);
            }
            for (uint32_t CurrentItem = FirstItem; CurrentItem < FirstItem + Count; CurrentItem++)
            {
                new (GetRawItem(CurrentItem)) T();
            }
            return FirstItem;
        }
        T* GetItem(uint32_t Index)
        {
            if (!Index)
            {
                return nullptr;
            }
            uint32_t BlockIndex = Index / ItemsPerPage;
            uint32_t SubIndex = Index % ItemsPerPage;
//            checkLockFreePointerList(Index < (uint32_t)NextIndex.GetValue() && Index < MaxTotalItems && BlockIndex < MaxBlocks && Pages[BlockIndex]);
            return Pages[BlockIndex] + SubIndex;
        }
        
    private:
        void* GetRawItem(uint32_t Index)
        {
            uint32_t BlockIndex = Index / ItemsPerPage;
            uint32_t SubIndex = Index % ItemsPerPage;
//            checkLockFreePointerList(Index && Index < (uint32_t)NextIndex.GetValue() && Index < MaxTotalItems && BlockIndex < MaxBlocks);
            T* currentVal = Pages[BlockIndex].load(std::memory_order_acquire);
            if (!currentVal)
            {
                T* NewBlock = (T*)malloc(ItemsPerPage * sizeof(T));
//                checkLockFreePointerList(IsAligned(NewBlock, alignof(T)));
//                if (FPlatformAtomics::InterlockedCompareExchangePointer((void**)&Pages[BlockIndex], NewBlock, nullptr) != nullptr)
//                std::atomic<T*>* temp = &Pages[BlockIndex];
                if ( !std::atomic_compare_exchange_weak<T*>( &Pages[BlockIndex], &currentVal, NewBlock) )
                {
                    // we lost discard block
//                    checkLockFreePointerList(Pages[BlockIndex] && Pages[BlockIndex] != NewBlock);
//                    LockFreeFreeLinks(ItemsPerPage * sizeof(T), NewBlock);
                    free(NewBlock);
                }
                else
                {
//                    checkLockFreePointerList(Pages[BlockIndex]);
                }
            }
            return (void*)(Pages[BlockIndex] + SubIndex);
        }
        
        uint8_t PadToAvoidContention0[PLATFORM_CACHE_LINE_SIZE];
//        FThreadSafeCounter NextIndex;
        std::atomic<int32_t> NextIndex;
        uint8_t PadToAvoidContention1[PLATFORM_CACHE_LINE_SIZE];
        std::atomic<T*> Pages[MaxBlocks];
        uint8_t PadToAvoidContention2[PLATFORM_CACHE_LINE_SIZE];
    };
    
    
#define MAX_TagBitsValue (uint64_t(1) << (64 - MAX_LOCK_FREE_LINKS_AS_BITS))
    struct FIndexedLockFreeLink;
    
    
//    MS_ALIGN(8)
    struct FIndexedPointer
    {
        // no constructor, intentionally. We need to keep the ABA double counter in tact
        
        // This should only be used for FIndexedPointer's with no outstanding concurrency.
        // Not recycled links, for example.
        void Init()
        {
            static_assert(((MAX_LOCK_FREE_LINKS - 1) & MAX_LOCK_FREE_LINKS) == 0, "MAX_LOCK_FREE_LINKS must be a power of two");
            Ptrs = 0;
        }
        
        void SetAll(uint32_t Ptr, uint64_t CounterAndState)
        {
//            checkLockFreePointerList(Ptr < MAX_LOCK_FREE_LINKS && CounterAndState < (uint64_t(1) << (64 - MAX_LOCK_FREE_LINKS_AS_BITS)));
            Ptrs = (uint64_t(Ptr) | (CounterAndState << MAX_LOCK_FREE_LINKS_AS_BITS));
        }
        
        uint32_t GetPtr() const
        {
            return uint32_t(Ptrs & (MAX_LOCK_FREE_LINKS - 1));
        }
        
        void SetPtr(uint32_t To)
        {
            SetAll(To, GetCounterAndState());
        }
        
        uint64_t GetCounterAndState() const
        {
            return (Ptrs >> MAX_LOCK_FREE_LINKS_AS_BITS);
        }
        
        void SetCounterAndState(uint64_t To)
        {
            SetAll(GetPtr(), To);
        }
        
        void AdvanceCounterAndState(const FIndexedPointer &From, uint64_t TABAInc)
        {
            SetCounterAndState(From.GetCounterAndState() + TABAInc);
            if (GetCounterAndState() < From.GetCounterAndState())
            {
                // this is not expected to be a problem and it is not expected to happen very often. When it does happen, we will sleep as an extra precaution.
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        
        template<uint64_t TABAInc>
        uint64_t GetState() const
        {
            return GetCounterAndState() & (TABAInc - 1);
        }
        
        template<uint64_t TABAInc>
        void SetState(uint64_t Value)
        {
            assert(Value < TABAInc);
//            checkLockFreePointerList(Value < TABAInc);
            SetCounterAndState((GetCounterAndState() & ~(TABAInc - 1)) | Value);
        }
        
        void AtomicRead(const FIndexedPointer& Other)
        {
            assert(IsAligned(&Ptrs, 8) && IsAligned(&Other.Ptrs, 8));
//            checkLockFreePointerList(IsAligned(&Ptrs, 8) && IsAligned(&Other.Ptrs, 8));
//            Ptrs = uint64_t(FPlatformAtomics::AtomicRead64((volatile const int64_t*)&Other.Ptrs));
            std::atomic_exchange_explicit<uint64_t>( &Ptrs, Other.Ptrs, std::memory_order_relaxed);
//            TestCriticalStall();
        }
        
        bool InterlockedCompareExchange(const FIndexedPointer& Exchange, const FIndexedPointer& Comparand)
        {
//            TestCriticalStall();
//            return uint64_t(FPlatformAtomics::InterlockedCompareExchange((volatile int64_t*)&Ptrs, Exchange.Ptrs, Comparand.Ptrs)) == Comparand.Ptrs;
            return std::atomic_compare_exchange_strong_explicit<uint64_t>(&Ptrs, (uint64_t*)&Exchange.Ptrs,  Comparand.Ptrs, std::memory_order_relaxed, std::memory_order_release);
            
//            return Ptrs.compare_exchange_strong(&Exchange.Ptrs,  Comparand.Ptrs, std::memory_order_relaxed, std::memory_order_release);
            
        }
        
        bool operator==(const FIndexedPointer& Other) const
        {
            return Ptrs == Other.Ptrs;
        }
        bool operator!=(const FIndexedPointer& Other) const
        {
            return Ptrs != Other.Ptrs;
        }
        
    private:
        std::atomic<uint64_t> Ptrs;
        
    }
//    GCC_ALIGN(8)
    ;
    
    struct FIndexedLockFreeLink
    {
        FIndexedPointer DoubleNext;
        void *Payload;
        uint32_t SingleNext;
    };
    
    // there is a version of this code that uses 128 bit atomics to avoid the indirection, that is why we have this policy class at all.
    struct FLockFreeLinkPolicy
    {
        enum
        {
            MAX_BITS_IN_TLinkPtr = MAX_LOCK_FREE_LINKS_AS_BITS
        };
        typedef FIndexedPointer TDoublePtr;
        typedef FIndexedLockFreeLink TLink;
        typedef uint32_t TLinkPtr;
        typedef TLockFreeAllocOnceIndexedAllocator<async::FIndexedLockFreeLink, MAX_LOCK_FREE_LINKS, 16384> TAllocator;
        
        static FIndexedLockFreeLink* DerefLink(uint32_t Ptr)
        {
            return LinkAllocator.GetItem(Ptr);
        }
        static FIndexedLockFreeLink* IndexToLink(uint32_t Index)
        {
            return LinkAllocator.GetItem(Index);
        }
        static uint32_t IndexToPtr(uint32_t Index)
        {
            return Index;
        }
        
        static uint32_t AllocLockFreeLink();
        static void FreeLockFreeLink(uint32_t Item);
        static TAllocator LinkAllocator;
    };
    
  
    template<int TPaddingForCacheContention, uint64_t TABAInc = 1>
    class FLockFreePointerListLIFORoot
    {
        typedef FLockFreeLinkPolicy::TDoublePtr TDoublePtr;
        typedef FLockFreeLinkPolicy::TLink TLink;
        typedef FLockFreeLinkPolicy::TLinkPtr TLinkPtr;
        
    public:
        FLockFreePointerListLIFORoot()
        {
            // We want to make sure we have quite a lot of extra counter values to avoid the ABA problem. This could probably be relaxed, but eventually it will be dangerous.
            // The question is "how many queue operations can a thread starve for".
            static_assert(MAX_TagBitsValue / TABAInc >= (1 << 23), "risk of ABA problem");
            static_assert((TABAInc & (TABAInc - 1)) == 0, "must be power of two");
            Reset();
        }
        
        void Reset()
        {
            Head.Init();
        }
        
        void Push(TLinkPtr Item)
        {
            while (true)
            {
                TDoublePtr LocalHead;
                LocalHead.AtomicRead(Head);
                TDoublePtr NewHead;
                NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
                NewHead.SetPtr(Item);
                FLockFreeLinkPolicy::DerefLink(Item)->SingleNext = LocalHead.GetPtr();
                if (Head.InterlockedCompareExchange(LocalHead, NewHead))
                {
                    break;
                }
            }
        }
        
        TLinkPtr Pop()
        {
            TLinkPtr Item = 0;
            while (true)
            {
                TDoublePtr LocalHead;
                LocalHead.AtomicRead(Head);
                Item = LocalHead.GetPtr();
                if (!Item)
                {
                    break;
                }
                TDoublePtr NewHead;
                NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
                TLink* ItemP = FLockFreeLinkPolicy::DerefLink(Item);
                NewHead.SetPtr(ItemP->SingleNext);
                if (Head.InterlockedCompareExchange(LocalHead, NewHead))
                {
                    ItemP->SingleNext = 0;
                    break;
                }
            }
            return Item;
        }
        
        TLinkPtr PopAll()
        {
            TLinkPtr Item = 0;
            while (true)
            {
                TDoublePtr LocalHead;
                LocalHead.AtomicRead(Head);
                Item = LocalHead.GetPtr();
                if (!Item)
                {
                    break;
                }
                TDoublePtr NewHead;
                NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
                NewHead.SetPtr(0);
                if (Head.InterlockedCompareExchange(LocalHead, NewHead))
                {
                    break;
                }
            }
            return Item;
        }
        
        bool IsEmpty() const
        {
            return !Head.GetPtr();
        }
        
        uint64_t GetState() const
        {
            TDoublePtr LocalHead;
            LocalHead.AtomicRead(Head);
            return LocalHead.GetState<TABAInc>();
        }
        
    private:
        
        FPaddingForCacheContention<TPaddingForCacheContention> PadToAvoidContention1;
        TDoublePtr Head;
        FPaddingForCacheContention<TPaddingForCacheContention> PadToAvoidContention2;
    };

    
    template<class T, int TPaddingForCacheContention, uint64_t TABAInc = 1>
    class FLockFreePointerListLIFOBase
    {
        typedef FLockFreeLinkPolicy::TDoublePtr TDoublePtr;
        typedef FLockFreeLinkPolicy::TLink TLink;
        typedef FLockFreeLinkPolicy::TLinkPtr TLinkPtr;
    public:
        void Reset()
        {
            RootList.Reset();
        }
        
        void Push(T* InPayload)
        {
            TLinkPtr Item = FLockFreeLinkPolicy::AllocLockFreeLink();
            FLockFreeLinkPolicy::DerefLink(Item)->Payload = InPayload;
            RootList.Push(Item);
        }
        
        T* Pop()
        {
            TLinkPtr Item = RootList.Pop();
            T* Result = nullptr;
            if (Item)
            {
                Result = (T*)FLockFreeLinkPolicy::DerefLink(Item)->Payload;
                FLockFreeLinkPolicy::FreeLockFreeLink(Item);
            }
            return Result;
        }
        
        void PopAll(std::vector<T*>& OutArray)
        {
            TLinkPtr Links = RootList.PopAll();
            while (Links)
            {
                TLink* LinksP = FLockFreeLinkPolicy::DerefLink(Links);
                OutArray.Add((T*)LinksP->Payload);
                TLinkPtr Del = Links;
                Links = LinksP->SingleNext;
                FLockFreeLinkPolicy::FreeLockFreeLink(Del);
            }
        }
        
        bool IsEmpty() const
        {
            return RootList.IsEmpty();
        }
        
        uint64_t GetState() const
        {
            return RootList.GetState();
        }
        
    private:
        
        FLockFreePointerListLIFORoot<TPaddingForCacheContention, TABAInc> RootList;
    };
    
    
    template<class T, int TPaddingForCacheContention, uint64_t TABAInc = 1>
    class FLockFreePointerFIFOBase
    {
        typedef FLockFreeLinkPolicy::TDoublePtr TDoublePtr;
        typedef FLockFreeLinkPolicy::TLink TLink;
        typedef FLockFreeLinkPolicy::TLinkPtr TLinkPtr;
    public:
        
        FLockFreePointerFIFOBase()
        {
            // We want to make sure we have quite a lot of extra counter values to avoid the ABA problem. This could probably be relaxed, but eventually it will be dangerous.
            // The question is "how many queue operations can a thread starve for".
            static_assert(TABAInc <= 65536, "risk of ABA problem");
            static_assert((TABAInc & (TABAInc - 1)) == 0, "must be power of two");
            
            Head.Init();
            Tail.Init();
            TLinkPtr Stub = FLockFreeLinkPolicy::AllocLockFreeLink();
            Head.SetPtr(Stub);
            Tail.SetPtr(Stub);
        }
        
        void Push(T* InPayload)
        {
            TLinkPtr Item = FLockFreeLinkPolicy::AllocLockFreeLink();
            FLockFreeLinkPolicy::DerefLink(Item)->Payload = InPayload;
            TDoublePtr LocalTail;
            while (true)
            {
                LocalTail.AtomicRead(Tail);
                TLink* LoadTailP = FLockFreeLinkPolicy::DerefLink(LocalTail.GetPtr());
                TDoublePtr LocalNext;
                LocalNext.AtomicRead(LoadTailP->DoubleNext);
                TDoublePtr TestLocalTail;
                TestLocalTail.AtomicRead(Tail);
                if (TestLocalTail == LocalTail)
                {
                    if (LocalNext.GetPtr())
                    {
                        TDoublePtr NewTail;
                        NewTail.AdvanceCounterAndState(LocalTail, TABAInc);
                        NewTail.SetPtr(LocalNext.GetPtr());
                        Tail.InterlockedCompareExchange(LocalTail, NewTail);
                    }
                    else
                    {
                        TDoublePtr NewNext;
                        NewNext.AdvanceCounterAndState(LocalNext, 1);
                        NewNext.SetPtr(Item);
                        if (LoadTailP->DoubleNext.InterlockedCompareExchange(LocalNext, NewNext))
                        {
                            break;
                        }
                    }
                }
            }
            {
//                TestCriticalStall();
                TDoublePtr NewTail;
                NewTail.AdvanceCounterAndState(LocalTail, TABAInc);
                NewTail.SetPtr(Item);
                Tail.InterlockedCompareExchange(LocalTail, NewTail);
            }
        }
        
        T* Pop()
        {
            T* Result = nullptr;
            TDoublePtr LocalHead;
            while (true)
            {
                LocalHead.AtomicRead(Head);
                TDoublePtr LocalTail;
                LocalTail.AtomicRead(Tail);
                TDoublePtr LocalNext;
                LocalNext.AtomicRead(FLockFreeLinkPolicy::DerefLink(LocalHead.GetPtr())->DoubleNext);
                TDoublePtr LocalHeadTest;
                LocalHeadTest.AtomicRead(Head);
                if (LocalHead == LocalHeadTest)
                {
                    if (LocalHead.GetPtr() == LocalTail.GetPtr())
                    {
                        if (!LocalNext.GetPtr())
                        {
                            return nullptr;
                        }
//                        TestCriticalStall();
                        TDoublePtr NewTail;
                        NewTail.AdvanceCounterAndState(LocalTail, TABAInc);
                        NewTail.SetPtr(LocalNext.GetPtr());
                        Tail.InterlockedCompareExchange(LocalTail, NewTail);
                    }
                    else
                    {
                        Result = (T*)FLockFreeLinkPolicy::DerefLink(LocalNext.GetPtr())->Payload;
                        TDoublePtr NewHead;
                        NewHead.AdvanceCounterAndState(LocalHead, TABAInc);
                        NewHead.SetPtr(LocalNext.GetPtr());
                        if (Head.InterlockedCompareExchange(LocalHead, NewHead))
                        {
                            break;
                        }
                    }
                }
            }
            FLockFreeLinkPolicy::FreeLockFreeLink(LocalHead.GetPtr());
            return Result;
        }
        
        void PopAll(std::vector<T*>& OutArray)
        {
            while (T* Item = Pop())
            {
                OutArray.Add(Item);
            }
        }
        
        
        bool IsEmpty() const
        {
            TDoublePtr LocalHead;
            LocalHead.AtomicRead(Head);
            TDoublePtr LocalNext;
            LocalNext.AtomicRead(FLockFreeLinkPolicy::DerefLink(LocalHead.GetPtr())->DoubleNext);
            return !LocalNext.GetPtr();
        }
        
    private:
        
        FPaddingForCacheContention<TPaddingForCacheContention> PadToAvoidContention1;
        TDoublePtr Head;
        FPaddingForCacheContention<TPaddingForCacheContention> PadToAvoidContention2;
        TDoublePtr Tail;
        FPaddingForCacheContention<TPaddingForCacheContention> PadToAvoidContention3;
    };
    
    
    template<class T>
    class TLockFreePointerListLIFO : public FLockFreePointerListLIFOBase<T, 0>
    {
        
    };
    
    template<class T, int TPaddingForCacheContention>
    class TLockFreePointerListUnordered : public FLockFreePointerListLIFOBase<T, TPaddingForCacheContention>
    {
        
    };
    
   
    

} // namespace async



#endif // Foundation_LockFreeList_h__
