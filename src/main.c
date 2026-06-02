#include <stdio.h>

#include <lauxlib.h>

#include "LuaJIT/lua.h"
#include "LuaJIT/lualib.h"
#include "LuaJIT/luaxlib.h"
#include "LuaJIT/luajit.h"


#include "utils/debug.h"
#include "lua/luabc.h"



// https://luajit.org/ext_c_api.html
// https://www.lua.org/manual/5.1/
// https://www.lua.org/pil/contents.html#P4
// https://www.lua.org/manual/5.3/manual.html



extern const unsigned char luaJIT_BC_lexer[];



// The lua libraries we want to load
// I dont want to load all of them, hence this
// https://stackoverflow.com/questions/4551101/lual-openlibs-and-sandboxing-scripts
static const luaL_Reg lualibs[] = {
  { "",              luaopen_base    },
  { LUA_TABLIBNAME,  luaopen_table   },
  { LUA_IOLIBNAME,   luaopen_io      },
  //{ LUA_OSLIBNAME,   luaopen_os      },
  { LUA_STRLIBNAME,  luaopen_string  },
  { LUA_MATHLIBNAME, luaopen_math    },
  #ifdef __DEBUG__
  { LUA_DBLIBNAME,   luaopen_debug   },
  #endif
  //{ LUA_LOADLIBNAME, luaopen_package },
  { NULL,            NULL            }
  // I'll keep the null terminator for now
  // in case I want to pass this as a parameter
  // rather than it be a static global
}





int main(int argc, char *argv[]) {
    lua_State *L;

    // create new lua state
    L = luaL_newstate();
    assert(L != NULL, -1);

    // attach standard libraries to environment
    openlibs_list(L, lualibs);

    // push args to table
    gen_argtable(L, argv, argc);

    // load the cunks
    luaL_loadbuffer(L, luaJIT_BC_lexer_size,  luaJIT_BC_lexer,  "lexer");
    luaL_loadbuffer(L, luaJIT_BC_parser_size, luaJIT_BC_parser, "parser");
    luaL_loadbuffer(L, luaJIT_BC_lpeg_size,   luaJIT_BC_lpeg,   "lpeg");
    #ifdef __DEBUG__
    luaL_loadbuffer(L, luaJIT_BC_test_size,   luaJIT_BC_test,   "test");
    luaL_loadbuffer(L, luaJIT_BC_debug_size,  luaJIT_BC_debug,  "debug");
    #endif
    
    return 0;
}




void openlibs_list(lua_State *L, const luaL_Reg lualibs[]) {
    const luaL_Reg *lib = lualibs;

    // oooh. I rather like this. I just push it to the stack
    // and then call it? Nice.
    for (; lib->func; lib++) {
        lua_pushcfunction(L, lib->func);
        // looks like this is mainly for backwards compatability
        // it still gets consumed either way, so it is no harm.
        lua_pushstring(L, lib->name);
        lua_call(L, 1, 0);
    }
}



void gen_argtable(lua_State *L, char *argv[], int argc) {

    lua_createtable(L, argc, 0);    // create new table

    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]); // push argument onto stack
        lua_rawseti(L, -2, i);      // push argument from stack into table
    }

    lua_setglobal(L, "arg");       // pop table into global 'arg'
}
