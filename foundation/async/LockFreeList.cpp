#include <PlatformConfig.h>
#include <async/LockFreeList.h>
#include <cassert>
#include <vector>
#include <thread>

namespace async
{
    
//    static void ChangeMem(int32_t Delta)
//    {
//        static FThreadSafeCounter LockFreeListMem;
//        LockFreeListMem.Add(Delta);
//    }
//
//    void* LockFreeAllocLinks(SIZE_T AllocSize)
//    {
//        ChangeMem(AllocSize);
//        return std::malloc(AllocSize);
//    }
//    void LockFreeFreeLinks(SIZE_T AllocSize, void* Ptr)
//    {
//        ChangeMem(-int32(AllocSize));
//        std::free(Ptr);
//    }
    
    class LockFreeLinkAllocator_TLSCache
    {
        enum
        {
            NUM_PER_BUNDLE = 64,
        };
        
        typedef FLockFreeLinkPolicy::TLink TLink;
        typedef FLockFreeLinkPolicy::TLinkPtr TLinkPtr;
        
    public:
        
        LockFreeLinkAllocator_TLSCache()
        {
//            check(IsInGameThread());
//            TlsSlot = FPlatformTLS::AllocTlsSlot();
//            check(FPlatformTLS::IsValidTlsSlot(TlsSlot));
        }
        /** Destructor, leaks all of the memory **/
        ~LockFreeLinkAllocator_TLSCache()
        {
//            FPlatformTLS::FreeTlsSlot(TlsSlot);
//            TlsSlot = 0;
        }
        
        /**
         * Allocates a memory block of size SIZE.
         *
         * @return Pointer to the allocated memory.
         * @see Free
         */
        TLinkPtr Pop()
        {
            FThreadLocalCache& TLS = GetTLS();
            
            if (!TLS.PartialBundle)
            {
                if (TLS.FullBundle)
                {
                    TLS.PartialBundle = TLS.FullBundle;
                    TLS.FullBundle = 0;
                }
                else
                {
                    TLS.PartialBundle = GlobalFreeListBundles.Pop();
                    if (!TLS.PartialBundle)
                    {
                        int32_t FirstIndex = FLockFreeLinkPolicy::LinkAllocator.Alloc(NUM_PER_BUNDLE);
                        for (int32_t Index = 0; Index < NUM_PER_BUNDLE; Index++)
                        {
                            TLink* Event = FLockFreeLinkPolicy::IndexToLink(FirstIndex + Index);
                            Event->DoubleNext.Init();
                            Event->SingleNext = 0;
                            Event->Payload = (void*)(uint64_t)(TLS.PartialBundle);
                            TLS.PartialBundle = FLockFreeLinkPolicy::IndexToPtr(FirstIndex + Index);
                        }
                    }
                }
                TLS.NumPartial = NUM_PER_BUNDLE;
            }
            TLinkPtr Result = TLS.PartialBundle;
            TLink* ResultP = FLockFreeLinkPolicy::DerefLink(TLS.PartialBundle);
            TLS.PartialBundle = TLinkPtr(uint64_t(ResultP->Payload));
            TLS.NumPartial--;
//            checkLockFreePointerList(TLS.NumPartial >= 0 && ((!!TLS.NumPartial) == (!!TLS.PartialBundle)));
            ResultP->Payload = nullptr;
//            checkLockFreePointerList(!ResultP->DoubleNext.GetPtr() && !ResultP->SingleNext);
            return Result;
        }
        
        /**
         * Puts a memory block previously obtained from Allocate() back on the free list for future use.
         *
         * @param Item The item to free.
         * @see Allocate
         */
        void Push(TLinkPtr Item)
        {
            FThreadLocalCache& TLS = GetTLS();
            if (TLS.NumPartial >= NUM_PER_BUNDLE)
            {
                if (TLS.FullBundle)
                {
                    GlobalFreeListBundles.Push(TLS.FullBundle);
                    //TLS.FullBundle = nullptr;
                }
                TLS.FullBundle = TLS.PartialBundle;
                TLS.PartialBundle = 0;
                TLS.NumPartial = 0;
            }
            TLink* ItemP = FLockFreeLinkPolicy::DerefLink(Item);
            ItemP->DoubleNext.SetPtr(0);
            ItemP->SingleNext = 0;
            ItemP->Payload = (void*)uint64_t(TLS.PartialBundle);
            TLS.PartialBundle = Item;
            TLS.NumPartial++;
        }
        
    private:
        
        /** struct for the TLS cache. */
        struct FThreadLocalCache
        {
            TLinkPtr FullBundle;
            TLinkPtr PartialBundle;
            int32_t NumPartial;
            
            FThreadLocalCache()
            : FullBundle(0)
            , PartialBundle(0)
            , NumPartial(0)
            {
            }
        };
        
        FThreadLocalCache& GetTLS()
        {
//            checkSlow(FPlatformTLS::IsValidTlsSlot(TlsSlot));
//            FThreadLocalCache* TLS = (FThreadLocalCache*)FPlatformTLS::GetTlsValue(TlsSlot);
            static thread_local FThreadLocalCache* TLS = nullptr;
            if (!TLS)
            {
                TLS = new FThreadLocalCache();
//                FPlatformTLS::SetTlsValue(TlsSlot, TLS);
            }
            return *TLS;
        }
        
        /** Slot for TLS struct. */
//        uint32 TlsSlot;
        
        /** Lock free list of free memory blocks, these are all linked into a bundle of NUM_PER_BUNDLE. */
        FLockFreePointerListLIFORoot<PLATFORM_CACHE_LINE_SIZE> GlobalFreeListBundles;
    };
    
    static LockFreeLinkAllocator_TLSCache GLockFreeLinkAllocator;
    
    void FLockFreeLinkPolicy::FreeLockFreeLink(FLockFreeLinkPolicy::TLinkPtr Item)
    {
        GLockFreeLinkAllocator.Push(Item);
    }
    
    FLockFreeLinkPolicy::TLinkPtr FLockFreeLinkPolicy::AllocLockFreeLink()
    {
        FLockFreeLinkPolicy::TLinkPtr Result = GLockFreeLinkAllocator.Pop();
        // this can only really be a mem stomp
//        checkLockFreePointerList(Result && !FLockFreeLinkPolicy::DerefLink(Result)->DoubleNext.GetPtr() && !FLockFreeLinkPolicy::DerefLink(Result)->Payload && !FLockFreeLinkPolicy::DerefLink(Result)->SingleNext);
        return Result;
    }
    
    FLockFreeLinkPolicy::TAllocator FLockFreeLinkPolicy::LinkAllocator;

} // namespace async
