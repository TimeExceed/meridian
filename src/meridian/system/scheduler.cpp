#include "scheduler.hpp"
#include "service.hpp"
#include "meridian/common/assert.hpp"
#include "lua/lua.hpp"
#include <utility>

using namespace ::std;

namespace meridian {
namespace system {

Scheduler::Scheduler(int64_t threads)
  : mThreads(threads), mLastId(0)
{}

Scheduler::~Scheduler()
{}

void Scheduler::addService(unique_ptr<Service>&& serv)
{
    lock_guard<mutex> g(mMutex);
    mServices.insert(make_pair(serv->name(), move(serv)));
}

void Scheduler::start()
{
    {
        lock_guard<mutex> g(mMutex);
        for(const pair<const string, unique_ptr<Service>>& i: mServices) {
            i.second->start();
        }
    }
    mThreads.start();
}

void Scheduler::stop()
{
    mThreads.stop();
    {
        lock_guard<mutex> g(mMutex);
        for(const pair<const int64_t, unique_ptr<Instance>>& i: mInstances) {
            i.second->close();
        }
        mInstances.clear();
        for(const pair<const string, unique_ptr<Service>>& i: mServices) {
            i.second->stop();
        }
        mServices.clear();
    }
}

Instance* Scheduler::newInstance(const string& serv)
{
    lock_guard<mutex> g(mMutex);
    ServiceMap::const_iterator si = mServices.find(serv);
    if (si == mServices.end()) {
        return nullptr;
    } else {
        ++mLastId;
        MERIDIAN_ASSERT(mLastId > 0)(mLastId);
        unique_ptr<Instance> ins = si->second->newInstance(mLastId);
        Instance* res = ins.get();
        MERIDIAN_ASSERT(mInstances.find(mLastId) == mInstances.end())
            (mLastId);
        mInstances[mLastId] = move(ins);
        return res;
    }
}

Instance* Scheduler::getInstance(int64_t id)
{
    lock_guard<mutex> g(mMutex);
    InstanceMap::iterator i = mInstances.find(id);
    if (i != mInstances.end()) {
        return i->second.get();
    } else {
        return nullptr;
    }
}

namespace {

const char kGoto[] = "goto";

} // namespace

void Scheduler::schedule(lua_State* args)
{
    lua_pushlstring(args, kGoto, sizeof(kGoto) - 1);
    //lua_pushstring(args, kGoto);
    int rc = lua_gettable(args, 1);
    MERIDIAN_ASSERT(rc == LUA_TNUMBER)(rc);
    int isnum = 0;
    int64_t id = lua_tointegerx(args, -1, &isnum);
    MERIDIAN_ASSERT(isnum);
    lua_pop(args, 1);
    Instance* ins = getInstance(id);
    MERIDIAN_ASSERT(ins != nullptr);
    function<void()> f = ins->runner(args);
    mThreads.push(f);
}

} // namespace system
} // namespace meridian
