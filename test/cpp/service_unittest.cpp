#include "meridian/system/service.hpp"
#include "meridian/system/scheduler.hpp"
#include "meridian/common/semaphore.hpp"
#include "meridian/common/lua_helper.hpp"
#include "meridian/common/assert.hpp"
#include "meridian/common/noncopyable.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "lua/lua.hpp"
#include <climits>

using namespace ::testing;
using namespace ::std;
using namespace ::meridian;
using namespace ::meridian::common;

namespace {

class PlusInstance
  : public system::Instance,
    private common::Noncopyable
{
    static char const * const kProg;

    string* mLog;
    lua_State* mLua;
    Semaphore* mSemaphore;

public:
    PlusInstance(int64_t id, system::Service* serv, string* log, Semaphore* sem)
      : Instance(id, serv),
        mLog(log),
        mLua(NULL),
        mSemaphore(sem)
    {
        mLua = luaL_newstate();
        luaL_openlibs(mLua);
        int r = luaL_loadstring(mLua, kProg);
        MERIDIAN_ASSERT(r == LUA_OK)(r);
        r = lua_resume(mLua, NULL, 0);
        MERIDIAN_ASSERT(r == LUA_OK)(r);
        MERIDIAN_ASSERT(lua_gettop(mLua) == 1)(lua_gettop(mLua));
    }
    ~PlusInstance()
    {
        MERIDIAN_ASSERT(mLua == NULL);
    }

    void close()
    {
        mLog->append("iclose");
        mLog->append(to_string(id()));
        mLog->push_back(',');
        lua_close(mLua);
        mLua = NULL;
    }

    function<void()> runner(lua_State* p)
    {
        mLog->append("irun,");
        common::copyArgs(mLua, p, 2);
        return bind(&PlusInstance::run, this, p);
    }

private:
    void run(lua_State* p)
    {
        lua_resume(mLua, NULL, 2);
        mLog->append(to_string(lua_tointeger(p, -2)));
        mLog->push_back('+');
        mLog->append(to_string(lua_tointeger(p, -1)));
        mLog->push_back('=');
        mLog->append(to_string(lua_tointeger(mLua, -1)));
        mLog->push_back(',');
    }
};

char const * const PlusInstance::kProg =
    "local function f(x, y)\n"
    "return {}, x + y\n"
    "end\n"
    "return f\n";


class PlusService
  : public system::Service,
    private common::Noncopyable
{
    string* mLog;
    Semaphore* mSemaphore;

public:
    PlusService(string* log, Semaphore* sem)
      : mLog(log), mSemaphore(sem)
    {}
    
    string name() const
    {
        return "PlusService";
    }
    void start()
    {
        mLog->append("sstartPlusService,");
    }
    void stop()
    {
    }
    unique_ptr<system::Instance> newInstance(int64_t id)
    {
        mLog->append("snInst");
        mLog->append(to_string(id));
        mLog->push_back(',');
        return unique_ptr<system::Instance>(new PlusInstance(id, this, mLog, mSemaphore));
    }
};

} // namespace

TEST(ServiceTest, Instance) {
    string log;
    Semaphore sem;
    PlusService service(&log, &sem);
    EXPECT_EQ(service.name(), string("PlusService"));
    service.start();
    unique_ptr<system::Instance> ins = service.newInstance(1234);
    lua_State* p = luaL_newstate();
    common::ScopedLuaState sls(p);
    lua_createtable(p, 0, 0);
    lua_pushinteger(p, 1);
    lua_pushinteger(p, 2);
    function<void()> runner = ins->runner(p);
    runner();
    ins->close();
    EXPECT_STREQ(log.c_str(),
        "sstartPlusService,snInst1234,irun,1+2=3,iclose1234,");
}

TEST(SchedulerTest, CppInstance) {
    system::Scheduler schd(1);
    string log;
    Semaphore sem;
    schd.addService(unique_ptr<system::Service>(new PlusService(&log, &sem)));
    schd.start();
    system::Instance* ins = schd.newInstance("PlusService");
    lua_State* p = luaL_newstate();
    common::ScopedLuaState sls(p);
    lua_createtable(p, 0, 1);
    lua_pushstring(p, "goto");
    lua_pushinteger(p, ins->id());
    lua_settable(p, -3);
    lua_pushinteger(p, 1);
    lua_pushinteger(p, 2);
    schd.schedule(p);
    string oracle = "sstartPlusService,snInst" + to_string(ins->id())
        + ",irun,1+2=3,iclose"+ to_string(ins->id()) + ",";
    schd.stop();
    EXPECT_STREQ(log.c_str(), oracle.c_str());
}

