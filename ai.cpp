#include "ai.h"

namespace aif{

    int getNumberOfMoves(lua_State *L) {
        string game_state = luaL_checkstring(L, 1);

        return 1;
    }

    int simulate(lua_State *L) {
        return 1;
    }

    int whoWin(lua_State *L) {
        return 1;
    }

    int getTurn(lua_State *L) {
        return 1;
    }

    int evaluate(lua_State *L) {
        return 1;
    }
}

Ai::Ai()
{

}
