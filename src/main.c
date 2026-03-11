#include <stdio.h>

#include <lauxlib.h>

#include "LuaJIT/lua.h"
#include "LuaJIT/lualib.h"
#include "LuaJIT/luaxlib.h"
#include "LuaJIT/luajit.h"


#include "utils/debug.h"



// https://www.lua.org/manual/5.1/
// https://www.lua.org/pil/contents.html#P4
// https://www.lua.org/manual/5.3/manual.html



extern const unsigned char luaJIT_BC_lexer[];



int main(int argc, char *argv[]) {
    lua_State *lstate;

    // create new lua state
    lstate = luaL_newstate();
    assert(lstate!= NULL, -1);

    // attach standard libraries to environment
    luaL_openlibs(lstate);

    // load all of the chunks
    // actually, lets create a function that allows for lua to load
    // the chunks themselves. So we only need to load the main chunk
    
    return 0;
}
