#ifndef LUABC_H
#define LUABC_H

extern const unsigned char luaJIT_BC_lexer[];
extern const unsigned char luaJIT_BC_parser[];
extern const unsigned char luaJIT_BC_lpeg[];
#ifdef __DEBUG__
extern const unsigned char luaJIT_BC_test[];
extern const unsigned char luaJIT_BC_debug[];
#endif

extern const uint32_t luaJIT_BC_lexer_size;
extern const uint32_t luaJIT_BC_parser_size;
extern const uint32_t luaJIT_BC_lpeg_size;
#ifdef __DEBUG__
extern const uint32_t luaJIT_BC_test_size;
extern const uint32_t luaJIT_BC_debug_size;
#endif

#endif /* #ifndef LUABC_H */
