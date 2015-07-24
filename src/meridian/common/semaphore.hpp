#ifndef MERIDIAN_COMMON_SEMAPHORE_HPP
#define MERIDIAN_COMMON_SEMAPHORE_HPP

#include "meridian/common/noncopyable.hpp"
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <cstdint>

namespace meridian {
namespace common {

class Semaphore : private Noncopyable
{
    std::mutex mMutex;
    std::condition_variable mCondVar;
    int64_t mAvailable;

public:
    Semaphore(Semaphore&&) =delete;
    
    Semaphore(int64_t init = 0)
      : mAvailable(init)
    {}

    void signal();
    std::cv_status wait();
    std::cv_status waitFor(const std::chrono::microseconds&);
};

} // namespace common
} // namespace meridian

#endif /* MERIDIAN_COMMON_SEMAPHORE_HPP */
