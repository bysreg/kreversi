#include <kdebug.h>
#include <KStandardDirs>
#include <iostream>
#include "ai.h"

namespace aif{

    int getNumberOfMoves(lua_State *L) {
        std::string game_state = luaL_checkstring(L, 1);
        Engine engine(game_state);
        lua_pushinteger(L, engine.getNumberOfMovesWithPass());
        return 1;
    }

    int simulate(lua_State *L) {
        std::string game_state = luaL_checkstring(L, 1);
        int move_num = luaL_checkinteger(L, 2);
        Engine engine(game_state);
        PosList poslist = engine.getAllMoves();
        engine.putPiece(poslist[move_num].row, poslist[move_num].col);
        lua_pushstring(L, engine.getGameStateString().c_str());
        return 1;
    }

    int whoWin(lua_State *L) {
        std::string game_state = luaL_checkstring(L, 1);
        Engine engine(game_state);
        int winner = engine.whoWin();
        int ret = 0;
        if(winner == Engine::NONE_REP)
            ret = 2;
        else if (winner == Engine::DARK_REP)
            ret = 1;
        else if (winner == Engine::LIGHT_REP)
            ret = -1;
        else if (winner == Engine::TIE_REP)
            ret = 0;
        lua_pushinteger(L, ret);
        return 1;
    }

    int getTurn(lua_State *L) {
        std::string game_state = luaL_checkstring(L, 1);
        Engine engine(game_state);
        lua_pushinteger(L, Engine::chipColor2Char(engine.getTurn()));
        return 1;
    }

    int evaluate(lua_State *L) {
        std::string game_state = luaL_checkstring(L, 1);
        Engine engine(game_state);
        int ret = engine.EvaluatePosition(Black);//ai evaluate function is always seen from the first player perspective which is the black player
        //kDebug() << "eval : " << ret << " ";
        lua_pushnumber(L, ret);
        return 1;
    }
}

lua_State* Ai::L = NULL;

void bail(lua_State *L, const char *msg){
    kError() << "FATAL ERROR: " << msg << ": " << lua_tostring(L, -1);
    exit(1);
}

static const struct luaL_Reg aiclib_funcs [] = {
    {"getNumberOfMoves", aif::getNumberOfMoves},
    {"simulate", aif::simulate},
    {"whoWin", aif::whoWin},
    {"getTurn", aif::getTurn},
    {"evaluate", aif::evaluate},
    {NULL, NULL}  /* sentinel */
};

int luaopen_aiclib (lua_State *L) {
    luaL_newlib(L, aiclib_funcs);
    return 1;
}

Ai::Ai(std::string ai_profile)
{    
    QString ai_profiles_path = KStandardDirs::locate("appdata", "ai_profiles.lua");

    staticInit();
    lua_getglobal(L, "createProfile");
    lua_pushstring(L, ai_profiles_path.toAscii());
    lua_pushstring(L, ai_profile.c_str());
    if(lua_pcall(L, 2, 1, 0))
        bail(L, "lua_pcall() failed");          /* Error out if Lua file has an error */

    profile_ref = luaL_ref(L, LUA_REGISTRYINDEX); // pop the resulting profile object and store its reference
}

Ai::~Ai()
{
    lua_close(L);
    L = NULL;
}

KReversiPos Ai::selectMove(const Engine& engine)
{
    PosList legalMoves = engine.getAllMoves();

    for(int i=0;i<legalMoves.size();i++)
        kDebug() << "move_index  " << i << legalMoves[i].row << " " <<legalMoves[i].col;

    lua_getglobal(L, "exec");
    lua_rawgeti(L, LUA_REGISTRYINDEX, profile_ref);
    lua_pushstring(L, engine.getGameStateString().c_str());

    if(lua_pcall(L, 2, 1, 0))
        bail(L, "lua_pcall() failed");          /* Error out if Lua file has an error */

    int ret = lua_tonumber(L, -1);
    lua_pop(L, 1);  /* pop returned value */
    kDebug()<<"best_move_index : " << ret;
    //because kreversigame board representation is zero-based index. we will convert move to zero-based index
    legalMoves[ret].row--;
    legalMoves[ret].col--;
    return legalMoves[ret];
}

void Ai::staticInit()
{
    if(L == NULL) {        
        QString ai_lib_path = KStandardDirs::locate("appdata", "ai.lua");
        L = luaL_newstate();
        luaL_openlibs(L);                           /* open all standard Lua libraries */
        luaL_requiref(L, "aif", luaopen_aiclib, 1);

        if (luaL_loadfile(L, ai_lib_path.toAscii()))    /* Load but don't run the Lua script */
            bail(L, "luaL_loadfile() failed");      /* Error out if file can't be read */

        if (lua_pcall(L, 0, 0, 0))                  /* Run the loaded Lua script */
            bail(L, "lua_pcall() failed");          /* Error out if Lua file has an error */
    }
}
