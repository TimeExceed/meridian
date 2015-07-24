#include "thread_pool.hpp"
#include "meridian/common/assert.hpp"
#include <algorithm>

using namespace ::std;

namespace meridian {
namespace common {

namespace {

class Exit
{};

void exitter()
{
    throw Exit();
}

} // namespace

ThreadPool::Worker::Worker()
  : notifier(new Semaphore)
{}

ThreadPool::ThreadPool(int64_t threads)
  : mStop(false)
{
    for(int64_t i = 0; i < threads; ++i) {
        mWorkers.push_back(move(Worker()));
    }
}

ThreadPool::~ThreadPool()
{
    MERIDIAN_ASSERT(mStop)
        .Msg("a thread pool must be stopped before its destruction");
}

void ThreadPool::start()
{
    lock_guard<mutex> g(mMutex);
    for(int64_t i = mWorkers.size() - 1; i >= 0; --i) {
        mWorkers[i].thread = move(thread(
                bind(&ThreadPool::worker, this, &mWorkers[i])));
        mWaitingList.push_back(&mWorkers[i]);
    }
    mDispatcher = move(thread(bind(&ThreadPool::dispatcher, this)));
}

void ThreadPool::stop()
{
    {
        lock_guard<mutex> g(mMutex);
        for(int64_t i = mWorkers.size(); i > 0; --i) {
            mToDoList.push_back(unique_ptr<WorkItem>(new WorkItem(exitter)));
        }
        mStop = true;
    }
    mDispatcherNotifier.signal();
    mDispatcher.join();
    for(int64_t i = mWorkers.size() - 1; i >= 0; --i) {
        mWorkers[i].thread.join();
    }
}

void ThreadPool::worker(Worker* w)
{
    try {
        for(;;) {
            w->notifier->wait();
            unique_ptr<WorkItem> wi = move(w->workItem);
            if (wi) {
                (*wi)();
            }
            {
                lock_guard<mutex> g(mMutex);
                mWaitingList.push_back(w);
            }
            mDispatcherNotifier.signal();
        }
    }
    catch(const Exit&) {
        mDispatcherNotifier.signal();
    }
}

void ThreadPool::dispatcher()
{
    for(;;) {
        mDispatcherNotifier.wait();
        lock_guard<mutex> g(mMutex);
        if (mStop && mToDoList.empty()) {
            break;
        }
        if (!mToDoList.empty() && !mWaitingList.empty()) {
            unique_ptr<WorkItem> wi = move(mToDoList.front());
            mToDoList.pop_front();
            Worker* w = mWaitingList.front();
            mWaitingList.pop_front();
            MERIDIAN_ASSERT(w->workItem.get() == NULL);
            w->workItem = move(wi);
            w->notifier->signal();
        }
    }
}

void ThreadPool::push(const WorkItem& wi)
{
    {
        lock_guard<mutex> g(mMutex);
        MERIDIAN_ASSERT(!mStop);
        mToDoList.push_back(unique_ptr<WorkItem>(new WorkItem(wi)));
    }
    mDispatcherNotifier.signal();
}

} // namespace common
} // namespace meridian
