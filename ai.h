#ifndef AI_H
#define AI_H

#include <lua.hpp>
#include <luaconf.h>
#include <lauxlib.h>
#include <lualib.h>

#include <string>

#include "Engine.h"

class Engine;

class Ai
{
public:
    Ai(std::string ai_profile);
    ~Ai();
    KReversiPos selectMove(const Engine& engine);
    static void staticInit();
private:
    static lua_State *L;
    int profile_ref;
};

namespace aif {
    typedef int (*lua_CFunction) (lua_State *L);
}

#endif // AI_H
