#ifndef MERIDIAN_SYSTEM_SCHEDULER_HPP
#define MERIDIAN_SYSTEM_SCHEDULER_HPP

#include "meridian/common/thread_pool.hpp"
#include "meridian/common/noncopyable.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <cstdint>

extern "C" {
struct lua_State;
}

namespace meridian {
namespace system {

class Service;
class Instance;

class Scheduler : private common::Noncopyable
{
    typedef std::unordered_map<int64_t, std::unique_ptr<Instance>> InstanceMap;
    typedef std::unordered_map<std::string, std::unique_ptr<Service>> ServiceMap;

    common::ThreadPool mThreads;

    std::mutex mMutex;
    int64_t mLastId;
    InstanceMap mInstances;
    ServiceMap mServices;
    
public:
    Scheduler(int64_t threads);
    ~Scheduler();

    void addService(std::unique_ptr<Service>&&);
    void start();
    void stop();

    Instance* newInstance(const std::string& service);
    Instance* getInstance(int64_t id);
    void schedule(lua_State* args);
};

} // namespace system
} // namespace meridian

#endif /* MERIDIAN_SYSTEM_SCHEDULER_HPP */

