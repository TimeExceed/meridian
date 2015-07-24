#include "semaphore.hpp"

using namespace std;

namespace meridian {
namespace common {

void Semaphore::signal()
{
    {
        unique_lock<mutex> lck(mMutex);
        ++mAvailable;
    }
    mCondVar.notify_one();
}

cv_status Semaphore::wait()
{
    unique_lock<mutex> lck(mMutex);
    while(mAvailable == 0) {
        mCondVar.wait(lck);
    }
    --mAvailable;
    return cv_status::no_timeout;
}

cv_status Semaphore::waitFor(const chrono::microseconds& d)
{
    cv_status res = cv_status::no_timeout;
    unique_lock<mutex> lck(mMutex);
    while(mAvailable == 0) {
        res = mCondVar.wait_for(lck, d);
        if (res == cv_status::timeout) {
            return cv_status::timeout;
        }
    }
    --mAvailable;
    return res;
}

} // namespace common
} // namespace meridian

