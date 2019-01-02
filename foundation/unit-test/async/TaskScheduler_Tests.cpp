#include <async/TaskSchedulerTestTasks.h>

#include <async/TaskScheduler.h>
#include <async/TaskSchedulerDefault.h>
#include <async/TaskSchedulerLockFree.h>
//#include <async/helper/testing/BaseTest.h>

#include <gtest/gtest.h> // googletest header file

#include <string>

//namespace
//{

    // compute the Fibonacci number for input N
    static int64_t Fibonacci(int64_t N, int nbThread, const bool useDynamicTaskAllocation, const std::string& schedulerName)
    {
        async::TaskScheduler* scheduler = async::TaskScheduler::create(schedulerName.c_str());
        scheduler->init(nbThread);
        
        async::Task::Status status;
        int64_t result = 0;
        
        if (useDynamicTaskAllocation)
        {
            FibonacciTask* task = new FibonacciTask(N, &result, useDynamicTaskAllocation, &status);
            scheduler->addTask(task);
        }
        else
        {
            FibonacciTask task(N, &result, useDynamicTaskAllocation, &status);
            scheduler->addTask(&task);
        }
        
        scheduler->workUntilDone(&status);
        
        scheduler->stop();
        return result;
    }
    
    
    // compute the sum of integers from 1 to N
    static int64_t IntSum1ToN(const int64_t N, int nbThread, const bool useDynamicTaskAllocation, const std::string& schedulerName)
    {
        async::TaskScheduler* scheduler = async::TaskScheduler::create(schedulerName.c_str());
        scheduler->init(nbThread);
        
        async::Task::Status status;
        int64_t result = 0;
        
        if (useDynamicTaskAllocation)
        {
            IntSumTask* task = new IntSumTask(1, N, &result, useDynamicTaskAllocation, &status);
            scheduler->addTask(task);
        }
        else
        {
            IntSumTask task(1, N, &result, useDynamicTaskAllocation, &status);
            scheduler->addTask(&task);
        }
        
        
        scheduler->workUntilDone(&status);
        
        scheduler->stop();
        return result;
    }
    
    
    
    // compute the Fibonacci single thread
    TEST(TaskSchedulerDefaultTests, FibonacciSingle )
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 1, false, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the Fibonacci multi thread
    TEST(TaskSchedulerDefaultTests, FibonacciMulti)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 0, false, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the sum of integers from 1 to N single thread
    TEST(TaskSchedulerDefaultTests, IntSumSingle)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);

        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 1, false, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, (N)*(N+1)/2);
        return;
    }

    // compute the sum of integers from 1 to N multi thread
    TEST(TaskSchedulerDefaultTests, IntSumMulti)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);
        
        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 0, false, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, (N)*(N + 1) / 2);
        return;
    }

    // compute the Fibonacci single thread
    TEST(TaskSchedulerLockFreeTests, FibonacciSingle )
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 1, false, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the Fibonacci multi thread
    TEST(TaskSchedulerLockFreeTests, FibonacciMulti)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 0, false, async::TaskSchedulerLockFree::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the sum of integers from 1 to N single thread
    TEST(TaskSchedulerLockFreeTests, IntSumSingle)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 1, false, async::TaskSchedulerLockFree::name());
        EXPECT_EQ(res, (N)*(N+1)/2);
        return;
    }

    // compute the sum of integers from 1 to N multi thread
    TEST(TaskSchedulerLockFreeTests, IntSumMulti)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 0, false, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, (N)*(N + 1) / 2);
        return;
    }


    // compute the Fibonacci single thread
    TEST(TaskSchedulerDefaultTests, FibonacciSingleDynamic )
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 1, true, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the Fibonacci multi thread
    TEST(TaskSchedulerDefaultTests, FibonacciMultiDynamic)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 0, true, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the sum of integers from 1 to N single thread
    TEST(TaskSchedulerDefaultTests, IntSumSingleDynamic)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);
        
        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 1, true, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, (N)*(N+1)/2);
        return;
    }

    // compute the sum of integers from 1 to N multi thread
    TEST(TaskSchedulerDefaultTests, IntSumMultiDynamic)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerDefault::name(), &async::TaskSchedulerDefault::create);
        
        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 0, true, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, (N)*(N + 1) / 2);
        return;
    }

    // compute the Fibonacci single thread
    TEST(TaskSchedulerLockFreeTests, FibonacciSingleDynamic )
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 1, true, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the Fibonacci multi thread
    TEST(TaskSchedulerLockFreeTests, FibonacciMultiDynamic)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        // tested with
        //  3 : 2
        //  6 : 8
        // 13 : 233
        // 23 : 28657
        // 27 : 196418
        // 35 : 9227465
        // 41 : 165580141
        // 43 : 433494437
        // 47 : 2971215073
        const int64_t res = Fibonacci(27, 0, true, async::TaskSchedulerLockFree::name());
        EXPECT_EQ(res, 196418);
        return;
    }

    // compute the sum of integers from 1 to N single thread
    TEST(TaskSchedulerLockFreeTests, IntSumSingleDynamic)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 1, true, async::TaskSchedulerLockFree::name());
        EXPECT_EQ(res, (N)*(N+1)/2);
        return;
    }

    // compute the sum of integers from 1 to N multi thread
    TEST(TaskSchedulerLockFreeTests, IntSumMultiDynamic)
    {
        async::TaskScheduler::registerScheduler(async::TaskSchedulerLockFree::name(), &async::TaskSchedulerLockFree::create);
        
        const int64_t N = 1 << 20;
        int64_t res = IntSum1ToN(N, 0, true, async::TaskSchedulerDefault::name());
        EXPECT_EQ(res, (N)*(N + 1) / 2);
        return;
    }

//} // namespace

