#include "meridian/common/thread_pool.hpp"
#include "meridian/common/semaphore.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <functional>
#include <string>

using namespace ::std;
using namespace ::testing;
using namespace ::meridian::common;

namespace {

void f1(string* log, Semaphore* sem, int n) {
    log->append(to_string(n));
    log->push_back(',');
    sem->signal();
}

} // namespace

TEST(ThreadPoolTest, Thread1_Item1) {
    string log;
    Semaphore sem(0);

    ThreadPool tp(1);
    tp.start();
    tp.push(bind(f1, &log, &sem, 1));
    sem.wait();
    tp.stop();

    EXPECT_STREQ(log.c_str(), "1,");
}

TEST(ThreadPoolTest, Thread1_Item10) {
    string log;
    Semaphore sem(0);
    const int n = 10;

    ThreadPool tp(1);
    tp.start();
    for(int i = 0; i < n; ++i) {
        tp.push(bind(f1, &log, &sem, i));
    }
    for(int i = 0; i < n; ++i) {
        sem.wait();
    }
    tp.stop();

    EXPECT_STREQ(log.c_str(), "0,1,2,3,4,5,6,7,8,9,");
}

namespace {

void f2(Semaphore* sem, mutex* m, vector<int>* vs, int n)
{
    {
        lock_guard<mutex> g(*m);
        vs->push_back(n);
    }
    sem->signal();
}

} // namespace

TEST(ThreadPoolTest, Thread10_Item100) {
    Semaphore sem(0);
    const int threads = 10;
    const int items = 100;
    mutex m;
    vector<int> vs;

    ThreadPool tp(threads);
    tp.start();
    for(int i = 0; i < items; ++i) {
        tp.push(bind(f2, &sem, &m, &vs, i));
    }
    for(int i = 0; i < items; ++i) {
        sem.wait();
    }
    tp.stop();

    EXPECT_EQ((int) vs.size(), items);
    sort(vs.begin(), vs.end());
    for(int i = 0; i < items; ++i) {
        EXPECT_EQ(vs[i], i);
    }
}

