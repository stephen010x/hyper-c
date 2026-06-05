#ifndef LUABC_H
#define LUABC_H

#include <stddef.h>

extern const char luaJIT_BC_lexer[];
extern const char luaJIT_BC_parser[];
extern const char luaJIT_BC_lpeg[];
#ifdef __DEBUG__
extern const char luaJIT_BC_test[];
extern const char luaJIT_BC_debug[];
#endif

extern const size_t luaJIT_BC_lexer_sizeof;
extern const size_t luaJIT_BC_parser_sizeof;
extern const size_t luaJIT_BC_lpeg_sizeof;
#ifdef __DEBUG__
extern const size_t luaJIT_BC_test_sizeof;
extern const size_t luaJIT_BC_debug_sizeof;
#endif

#endif /* #ifndef LUABC_H */
