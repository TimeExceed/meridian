#include "meridian/common/semaphore.hpp"
#include "lua/lua.hpp"
#include <condition_variable>
#include <chrono>
#include <cassert>

using namespace std;
using namespace meridian::common;

namespace {

int waitFor(lua_State* s)
{
    int64_t usecs = lua_tonumber(s, -1);
    Semaphore sema(0);
    chrono::microseconds d(usecs);
    cv_status r = sema.waitFor(d);
    assert(r == std::cv_status::timeout);
    return 0;
}

const struct luaL_Reg mod[] = {
    {"waitFor", waitFor},
    {NULL, NULL},
};

} // namespace

extern "C" int luaopen_chrono(lua_State* l)
{
    luaL_newlib(l, mod);
    return 1;
}
