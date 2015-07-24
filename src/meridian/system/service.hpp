#ifndef MERIDIAN_SYSTEM_SERVICE_HPP
#define MERIDIAN_SYSTEM_SERVICE_HPP

#include <functional>
#include <memory>
#include <cstdint>

extern "C" {
struct lua_State;
}

namespace meridian {
namespace system {

class Instance;

class Service
{
public:
    virtual ~Service() {}

    virtual std::string name() const =0;
    virtual void start() =0;
    virtual void stop() =0;
    virtual std::unique_ptr<Instance> newInstance(int64_t id) =0;
};

class Instance
{
    int64_t mId;
    Service* mService; // not own

public:
    Instance(
        int64_t id,
        Service* serv /* not own */)
      : mId(id), mService(serv)
    {}
    virtual ~Instance() {}

    int64_t id() const
    {
        return mId;
    }
    Service* service()
    {
        return mService;
    }
    Service const * service() const
    {
        return mService;
    }
    
    /**
     * @brief closes internal lua vm
     */
    virtual void close() =0;
    /**
     * @brief copys args from @p s to the internal lua vm,
     * and returns a function that will resume the internal vm once invoked.
     *
     * Caveats:
     * The resulting function must not be invoked when this Instance
     * is destroyed.
     */
    virtual std::function<void()> runner(lua_State* s) =0;
};

} // namespace system
} // namespace meridian

#endif /* MERIDIAN_SYSTEM_SERVICE_HPP */

