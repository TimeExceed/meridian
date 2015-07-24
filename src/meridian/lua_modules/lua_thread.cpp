#include "meridian/common/lua_helper.hpp"
#include "meridian/common/semaphore.hpp"
#include "meridian/common/noncopyable.hpp"
#include "lua/lua.hpp"
#include <thread>
#include <atomic>
#include <utility>
#include <functional>

using namespace ::std;
using namespace ::meridian::common;

namespace {

class ThreadWrapper : private Noncopyable
{
public:
    enum State {
        INIT,
        RUNNING,
        STOPPED,
    };
    
private:
    lua_State* mLua;
    int mRC;
    Semaphore mSemaphore;
    thread mThread;
    atomic<State> mState;

public:
    ThreadWrapper() =delete;
    ThreadWrapper(ThreadWrapper&&) =delete;
    explicit ThreadWrapper(lua_State* l)
      : mLua(NULL), mRC(LUA_OK), mSemaphore(0), mState(INIT)
    {
        mLua = luaL_newstate();
        luaL_openlibs(mLua);
        copyArgs(mLua, l, 1);
        mThread = move(thread(bind(&ThreadWrapper::run, this, l)));
    }

    ~ThreadWrapper()
    {
        mThread.join();
        if (mLua != NULL) {
            lua_close(mLua);
        }
    }

    int wait(lua_State* s)
    {
        lua_pop(s, lua_gettop(s));
        mSemaphore.wait();
        copyArgs(s, mLua, 1);
        if (mRC == LUA_OK) {
            return lua_gettop(mLua);
        } else {
            lua_error(s);
            return 0; // make compiler happy
        }
    }

private:
    void run(lua_State* l)
    {
        mState.store(RUNNING, memory_order_acq_rel);
        mRC = lua_resume(mLua, NULL, lua_gettop(mLua) - 1);
        mState.store(STOPPED, memory_order_acq_rel);
        mSemaphore.signal();
    }
};

int newThread(lua_State* s)
{
    int n = lua_gettop(s);
    if (n == 0) {
        lua_pushstring(s, "apply() must take at least one arg");
        lua_error(s);
    }
    if (!lua_isfunction(s, 1)) {
        lua_pushstring(s, "1st arg of apply() must be function");
        lua_error(s);
    }
    ThreadWrapper* th = new ThreadWrapper(s);
    lua_pushlightuserdata(s, th);
    return 1;
}

int join(lua_State* s)
{
    ThreadWrapper* th = (ThreadWrapper*) lua_touserdata(s, -1);
    int n = th->wait(s);
    delete th;
    return n;
}

const struct luaL_Reg mod[] = {
    {"newThread", newThread},
    {"join", join},
    {NULL, NULL},
};

} // namespace

extern "C" int luaopen_lua_thread(lua_State* l)
{
    luaL_newlib(l, mod);
    return 1;
}
