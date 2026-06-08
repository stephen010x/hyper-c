#include <stdio.h>

// #include <lauxlib.h>

// #include "LuaJIT/lua.h"
// #include "LuaJIT/lualib.h"
// #include "LuaJIT/luaxlib.h"
// #include "LuaJIT/luajit.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luajit.h"


#include "toolkit/debug.h"
#include "lua/luabc.h"

int luaopen_lpeg (lua_State *L);



// https://luajit.org/ext_c_api.html
// https://www.lua.org/manual/5.1/
// https://www.lua.org/pil/contents.html#P4
// https://www.lua.org/manual/5.3/manual.html



// extern const unsigned char luaJIT_BC_lexer[];

#ifdef __DEBUG__
int err_handler(lua_State *L);
_Static_assert(__builtin_types_compatible_p(typeof(&err_handler), lua_CFunction));
#endif
int call_chunk(lua_State *L, const char *name);
int load_chunk(lua_State *L, const char *buff, size_t size, const char *name);
void openlibs_list(lua_State *L, const luaL_Reg lualibs[]);
void gen_argtable(lua_State *L, char *argv[], int argc);




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
  // creates a dlopen warning on compilation due to static linking
  { LUA_LOADLIBNAME, luaopen_package },
  #ifdef __DEBUG__
  { LUA_DBLIBNAME,   luaopen_debug   },
  #endif

  { "lpeg", luaopen_lpeg },
  
  { NULL, NULL }
  // I'll keep the null terminator for now
  // in case I want to pass this as a parameter
  // rather than it be a static global
};



static const struct {
    const char *const restrict buff;
    const size_t *const restrict size;
    const char *const restrict name;
} BCchunks[] = {
    { luaJIT_BC_lexer,  &luaJIT_BC_lexer_sizeof,  "lexer"  },
    { luaJIT_BC_parser, &luaJIT_BC_parser_sizeof, "parser" },
    { luaJIT_BC_parser, &luaJIT_BC_parser_sizeof, "parser" },
    #ifdef __DEBUG__
    { luaJIT_BC_dtools, &luaJIT_BC_dtools_sizeof, "dtools" },
    { luaJIT_BC_test,   &luaJIT_BC_test_sizeof,   "test"   },
    #else
    { luaJIT_BC_main,   &luaJIT_BC_main_sizeof,   "main"   },
    #endif
    { NULL, NULL, NULL },
};




int main(int argc, char *argv[]) {
    int err;
    #ifdef __DEBUG__
    int sstarti, sendi;    // starting index into stack
    #endif
    // const char *err_str;
    lua_State *L;

    // create new lua state
    L = luaL_newstate();
    assert(L != NULL, -1);

    #ifdef __DEBUG__
    sstarti = lua_gettop(L);
    #endif

    // attach standard libraries to environment
    openlibs_list(L, lualibs);

    // push args to table
    gen_argtable(L, argv, argc);

    // load the chunks
//     load_chunk(L, luaJIT_BC_lexer,  luaJIT_BC_lexer_sizeof,  "lexer");
//     load_chunk(L, luaJIT_BC_parser, luaJIT_BC_parser_sizeof, "parser");
//     #ifdef __DEBUG__
//     load_chunk(L, luaJIT_BC_dtools,  luaJIT_BC_dtools_sizeof,  "dtools");
//     #endif
// 
//     #ifdef __DEBUG__
//     // run the test chunk
//     luaL_loadbuffer(L, luaJIT_BC_test,   luaJIT_BC_test_sizeof,   "test");
//     #else
//     luaL_loadbuffer(L, luaJIT_BC_main,   luaJIT_BC_main_sizeof,   "main");
//     #endif
//     
//     err = lua_pcall(L, 0, 0, 0);
// 
//     err_str = lua_tostring(L, -1);
//     if (err_str) printf("%s\n", err_str);
//     lua_pop(L, 1);

    // TODO: create a function for this
    // Also, create a naming convention for custom c/lua interface functions
    for (int i = 0; BCchunks[i].buff; i++) {
        err = load_chunk(L, BCchunks[i].buff, *BCchunks[i].size, BCchunks[i].name);
        if (err) goto exit;
    }

    #ifdef __DEBUG__
    err = call_chunk(L, "test");
    #else
    err = call_chunk(L, "main");
    #endif

    exit:

    #ifdef __DEBUG__
    sendi = lua_gettop(L);
    if (sstarti != sendi) {
        printf("WARNING: stack leak +%d detected! (s:%d e:%d)", sendi - sstarti, sstarti, sendi);
    }
    #endif
    
    #ifdef __DEBUG__
    printf("program exited (%d)\n", err);
    #endif
    return err;
}


#ifdef __DEBUG__
int err_handler(lua_State *L) {
    const char *msg;
    
    // lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    // lua_getfield(L, -1, "traceback");

    msg = lua_tostring(L, -1);
    luaL_traceback(L, L, msg, 1);
    lua_remove(L, -2);

    return 1;
}
#endif



int call_chunk(lua_State *L, const char *name) {
    int err = 0;
    const char *msg;

    #ifdef __DEBUG__
    lua_pushcfunction(L, err_handler);
    #endif

    lua_getfield(L, LUA_GLOBALSINDEX, "package");
    lua_getfield(L, -1, "preload");
    lua_getfield(L, -1, name);
    
    // #ifdef __DEBUG__
    // if (lua_isnil(L, -1)) {
    //     lua_pop(L, 3);
    //     printf("ERROR: attempted to call nonexistant lua chunk \"%s\"", name);
    //     return -1;
    // } else {
    //     lua_pushcfunction(L, err_handler);
    //     err = lua_pcall(L, 2, 0, -1);
    //     lua_pop(L, 3);
    // #else
    //     err = lua_pcall(L, 2, 0, 0);
    // #endif
    //     if (err) {
    //         msg = lua_tostring(L, -1);
    //         printf("%s\n", msg);
    //         lua_pop(L, 1);
    //     }
    //     return err;
    // #else
    // #ifdef __DEBUG__
    // }
    // #endif

    #ifdef __DEBUG__
    
    if (lua_isnil(L, -1)) {
        lua_pop(L, 4);
        printf("ERROR: attempted to call nonexistant lua chunk \"%s\"", name);
        return -1;
    }
    
    err = lua_pcall(L, 0, 0, -4);
    if (err) {
        msg = lua_tostring(L, -1);
        printf("%s\n", msg);
        lua_pop(L, 1);
    }
    lua_pop(L, 3);
    return err;
    
    #else

    err = lua_pcall(L, 0, 0, 0);
    if (err) {
        msg = lua_tostring(L, -1);
        printf("%s\n", msg);
        lua_pop(L, 1);
    }
    lua_pop(L, 2);
    return err;

    #endif
    
}



int load_chunk(lua_State *L, const char *buff, size_t size, const char *name) {
    int err;

    err = luaL_loadbuffer(L, buff, size, name);
    if (err) return err;

    lua_getfield(L, LUA_GLOBALSINDEX, "package");
    lua_getfield(L, -1, "preload");
    lua_pushvalue(L, -3);
    lua_setfield(L, -2, name);

    lua_pop(L, 3);
    return 0;
}



void openlibs_list(lua_State *L, const luaL_Reg lualibs[]) {
    const luaL_Reg *lib = lualibs;

    // oooh. I rather like this. I just push it to the stack
    // and then call it? Nice.
    // TODO: maybe hand these off to package rather than call myself
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
