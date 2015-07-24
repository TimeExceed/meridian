#include "lua_helper.hpp"
#include "lua/lua.hpp"
#include <string>
#include <cstdint>

using namespace std;

namespace meridian {
namespace common {

ScopedLuaState::~ScopedLuaState()
{
    lua_close(mLua);
}

namespace {

void copyArg(lua_State* to, lua_State* from, int idx);

int strWriter(lua_State* l, const void* p, size_t sz, void* ud)
{
    string* str = (string*) ud;
    str->append((char*) p, sz);
    return 0;
}

void copyFunc(lua_State* to, lua_State* from, int idx)
{
    lua_pushvalue(from, idx);
    string str;
    int r = lua_dump(from, strWriter, &str, 0);
    if (r != 0) {
        lua_pushstring(from, "lua_dump fails");
        lua_error(from);
    }
    lua_pop(from, 1);
    r = luaL_loadbuffer(to, str.c_str(), str.size(), "func__");
    if (r != LUA_OK) {
        lua_pushstring(from, lua_tostring(to, -1));
        lua_error(from);
    }
}

void copyStr(lua_State* to, lua_State* from, int idx)
{
    size_t sz = 0;
    const char* p = lua_tolstring(from, idx, &sz);
    lua_pushlstring(to, p, sz);
}

void copyNum(lua_State* to, lua_State* from, int idx)
{
    if (lua_isinteger(from, idx)) {
        int64_t i = lua_tointeger(from, idx);
        lua_pushinteger(to, i);
    } else {
        double d = lua_tonumber(from, idx);
        lua_pushnumber(to, d);
    }
}

void copyBool(lua_State* to, lua_State* from, int idx)
{
    int b = lua_toboolean(from, idx);
    lua_pushboolean(to, b);
}

void copyNil(lua_State* to, lua_State*, int idx)
{
    lua_pushnil(to);
}

void copyLightUserData(lua_State* to, lua_State* from, int idx)
{
    void* p = lua_touserdata(from, idx);
    lua_pushlightuserdata(to, p);
}

void copyTable(lua_State* to, lua_State* from, int idx)
{
    lua_pushnil(from);
    lua_newtable(to);
    int n = lua_gettop(to);
    for(; lua_next(from, idx) != 0;) {
        copyArg(to, from, -2);
        copyArg(to, from, -1);
        lua_settable(to, n);
        lua_pop(from, 1);
    }
}

void copyArg(lua_State* to, lua_State* from, int idx)
{
    int tp = lua_type(from, idx);
    if (tp == LUA_TFUNCTION) {
        copyFunc(to, from, idx);
    } else if (tp == LUA_TSTRING) {
        copyStr(to, from, idx);
    } else if (tp == LUA_TNUMBER) {
        copyNum(to, from, idx);
    } else if (tp == LUA_TBOOLEAN) {
        copyBool(to, from, idx);
    } else if (tp == LUA_TNIL) {
        copyNil(to, from, idx);
    } else if (tp == LUA_TLIGHTUSERDATA) {
        copyLightUserData(to, from, idx);
    } else if (tp == LUA_TTABLE) {
        copyTable(to, from, idx);
    } else {
        luaL_error(from, "unknown type: %s", luaL_typename(from, idx));
    }
}

} // namespace

void copyArgs(lua_State* to, lua_State* from, int startIdx)
{
    for(int i = startIdx, sz = lua_gettop(from); i <= sz; ++i) {
        copyArg(to, from, i);
    }
}

} // namespace common
} // namespace meridian
