#ifndef MERIDIAN_COMMON_LUA_HELPER_HPP
#define MERIDIAN_COMMON_LUA_HELPER_HPP

class lua_State;

namespace meridian {
namespace common {

class ScopedLuaState
{
    lua_State* mLua;

public:
    ScopedLuaState() =delete;
    explicit ScopedLuaState(lua_State* l)
      : mLua(l)
    {}
    ScopedLuaState(const ScopedLuaState&) =delete;
    ScopedLuaState& operator=(const ScopedLuaState&) =delete;
    ~ScopedLuaState();
};

void copyArgs(lua_State* to, lua_State* from, int startIdx);

} // namespace common
} // namespace meridian

#endif /* MERIDIAN_COMMON_LUA_HELPER_HPP */
