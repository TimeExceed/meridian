#ifndef MERIDIAN_COMMON_THREAD_POOL_HPP
#define MERIDIAN_COMMON_THREAD_POOL_HPP

#include "meridian/common/semaphore.hpp"
#include "meridian/common/noncopyable.hpp"
#include <thread>
#include <mutex>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <algorithm>
#include <cstdint>

namespace meridian {
namespace common {

class ThreadPool : private Noncopyable
{
public:
    /**
     * WorkItem must throw nothing.
     */
    typedef std::function<void ()> WorkItem;

private:
    struct Worker
    {
        std::thread thread;
        std::unique_ptr<Semaphore> notifier;
        std::unique_ptr<WorkItem> workItem;

        Worker();
    };

    std::vector<Worker> mWorkers;
    std::thread mDispatcher;
    Semaphore mDispatcherNotifier;

    std::mutex mMutex;
    std::list<std::unique_ptr<WorkItem>> mToDoList;
    std::list<Worker*> mWaitingList;
    bool mStop;

public:
    class Full
    {};

    ThreadPool(int64_t threads);
    ~ThreadPool();

    void start();
    void stop();

    /**
     * @brief Pushs @p workItem into the thread pool.
     *
     * It may throw Full if there is no available slots.
     *
     * @p workItem will be copied exactly once, 
     * and will eventually be executed exactly once.
     */
    void push(const WorkItem& workItem);

private:
    void worker(Worker*);
    void dispatcher();
};

} // namespace common
} // namespace meridian

#endif /* MERIDIAN_COMMON_THREAD_POOL_HPP */
