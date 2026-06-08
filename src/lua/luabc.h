#ifndef LUABC_H
#define LUABC_H

#include <stddef.h>

#include "lua.h"


extern const char luaJIT_BC_lexer[];
extern const char luaJIT_BC_parser[];
extern const char luaJIT_BC_lpeg[];
#ifdef __DEBUG__
extern const char luaJIT_BC_test[];
extern const char luaJIT_BC_dtools[];
#endif

extern const size_t luaJIT_BC_lexer_sizeof;
extern const size_t luaJIT_BC_parser_sizeof;
extern const size_t luaJIT_BC_lpeg_sizeof;
#ifdef __DEBUG__
extern const size_t luaJIT_BC_test_sizeof;
extern const size_t luaJIT_BC_dtools_sizeof;
#endif


// int lhook_keyword    (lua_State *L);
// int lhook_punctuator (lua_State *L);
// 
// int lhook_iskeyword    (lua_State *L);
// int lhook_ispunctuator (lua_State *L);
// int lhook_isidentifier (lua_State *L);
// int lhook_isconstant   (lua_State *L);
// int lhook_isstring     (lua_State *L);
// 
// int lhook_encode_tokens (lua_State *L);


#endif /* #ifndef LUABC_H */
