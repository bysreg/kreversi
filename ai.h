#ifndef AI_H
#define AI_H

#include <lua.h>
#include <lua.hpp>
#include <luaconf.h>
#include <lualib.h>
#include <lauxlib.h>

class Ai
{
public:
    Ai();
};

namespace aif {
    typedef int (*lua_CFunction) (lua_State *L);
}

#endif // AI_H
