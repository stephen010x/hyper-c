#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "lexer.h"
#include "toolbox/debug.h"

#ifdef __OPTIMIZE_SIZE__
#undef __inline__
#define __inline__ static inline
#endif


// #ifdef __OPTIMIZE__
// #define __fold_switch __optimize("-ftree-switch-conversion", "-fipa-cp")
// #else
// #define __fold_switch __optimize("O1", "-ftree-switch-conversion", "-fipa-cp")
// #endif




static void parse_linemarker(clexstate_t *restrict state);
static void next_token_string_literal(clexstate_t *restrict state, token_t *restrict token, uint8_t mode);
static void next_token_numeric(clexstate_t *restrict state, token_t *restrict token);
static void next_token_num_hex(clexstate_t *restrict state, token_t *restrict token);
static void next_token_num_misc(clexstate_t *restrict state, token_t *restrict token);
static void next_token_num_bin(clexstate_t *restrict state, token_t *restrict token);
static void next_token_identifier(clexstate_t *restrict state, token_t *restrict token);
static void next_token_punctuator(clexstate_t *restrict state, token_t *restrict token);
static __noreturn void lex_error(clexstate_t *state, const char *fmt, ...);





// inlined functions
static size_t reader_next(clexstate_t *state, int n);
static char read(clexstate_t *state, int index);
static size_t token_start(clexstate_t *state);
static size_t token_end(clexstate_t *state);
static void token_setup(clexstate_t *state, token_t *token, uint8_t type, uint8_t tid, uint16_t flags);
static char _nib2char(uint8_t n);
static const char *cchar(char c);
// static bool _bcmp(const char *restrict str1, const char *restrict str2, int len);
static bool fcmp(const char *restrict str1, const char *restrict str2, int len);
static bool is_slcomment(const char *c);
static bool is_mlcomment(const char *c);
static int8_t is_encoding_prefix(const char *c);
static bool is_sstring(const char *c);
static bool is_cstring(const char *c);
static bool is_schar(const char *c, bool escape);
static bool is_cchar(const char *c, bool escape);
static bool is_comment(const char *c);
static bool is_truenewline(const char *buff, size_t index);
static bool is_dec_digit(unsigned char c);
static bool is_hex_digit(unsigned char c);
static bool is_oct_digit(unsigned char c);
static bool is_bin_digit(unsigned char c);
static bool is_sign(char c);
// static char to_uppercase(char c);
// static char to_lowercase(char c);
static int8_t is_floating_suffix(const char *c);
// static bool skip_whitespace(clexstate_t *state);
// static void skip_slcomment(clexstate_t *state);
// static void skip_mlcomment(clexstate_t *state);
static void consume_hex_exponent(clexstate_t *state);
static void consume_exponent(clexstate_t *state);
static token_flags_t consume_floating_suffix(clexstate_t *state);
static token_flags_t consume_integer_suffix(clexstate_t *state);
static bool consume_dec_digit(clexstate_t *state);
static bool consume_hex_digit(clexstate_t *state);
static bool consume_oct_digit(clexstate_t *state);
static bool consume_bin_digit(clexstate_t *state);
static bool consume_alphanumeric(clexstate_t *state);




const bool whitespace_map[256] = {
    //[0 ... 255] = false,
    [' ']  = true,
    ['\t'] = true,
    ['\n'] = true,
    ['\v'] = true,
    ['\f'] = true,
    // ['\r'] = true,
};



const uint8_t alphanumeric_map[256] = {
    //[0 ... 255] = 0,
    ['0' ... '9'] = IS_NUMERIC,
    ['a' ... 'z'] = IS_ALPHA,
    ['A' ... 'Z'] = IS_ALPHA,
    ['_']         = IS_ALPHA,
};



const bool punctuator_map[256] = {
        ['!'] = true,
        ['%'] = true,
        ['&'] = true,
        ['('] = true,
        [')'] = true,
        ['*'] = true,
        ['+'] = true,
        [','] = true,
        ['-'] = true,
        ['.'] = true,
        ['/'] = true,
        [':'] = true,
        [';'] = true,
        ['<'] = true,
        ['='] = true,
        ['>'] = true,
        ['?'] = true,
        ['['] = true,
        [']'] = true,
        ['^'] = true,
        ['{'] = true,
        ['|'] = true,
        ['}'] = true,
        ['~'] = true,
};



const bool source_map[256] = {
    //[0 ... 255] = 0,
    ['0' ... '9'] = true,
    ['a' ... 'z'] = true,
    ['A' ... 'Z'] = true,
    ['_']  = true,
    ['{']  = true,
    ['}']  = true,
    ['[']  = true,
    [']']  = true,
    ['#']  = true,
    ['(']  = true,
    [')']  = true,
    ['<']  = true,
    ['>']  = true,
    ['%']  = true,
    [':']  = true,
    [';']  = true,
    ['.']  = true,
    ['?']  = true,
    ['*']  = true,
    ['+']  = true,
    ['-']  = true,
    ['/']  = true,
    ['^']  = true,
    ['&']  = true,
    ['|']  = true,
    ['~']  = true,
    ['!']  = true,
    ['=']  = true,
    [',']  = true,
    ['"']  = true,
    [' ']  = true,
    ['\\'] = true,
    ['\''] = true,
    ['\t'] = true,
    ['\n'] = true,
    ['\v'] = true,
    ['\f'] = true,
    // ['\r'] = true,
};





// const char uppercase_map[256] = {
//     ['a'] = ['A'],
//     ['b'] = ['B'],
//     ['c'] = ['C'],
//     ['d'] = ['D'],
//     ['e'] = ['E'],
//     ['f'] = ['F'],
//     ['g'] = ['G'],
//     ['h'] = ['H'],
//     ['i'] = ['I'],
//     ['j'] = ['J'],
//     ['k'] = ['K'],
//     ['l'] = ['L'],
//     ['m'] = ['M'],
//     ['n'] = ['N'],
//     ['o'] = ['O'],
//     ['p'] = ['P'],
//     ['q'] = ['Q'],
//     ['r'] = ['R'],
//     ['s'] = ['S'],
//     ['t'] = ['T'],
//     ['u'] = ['U'],
//     ['v'] = ['V'],
//     ['w'] = ['W'],
//     ['x'] = ['X'],
//     ['y'] = ['Y'],
//     ['z'] = ['Z'],
// };
// 
// const char lowercase_map[256] = {
//      ['A'] = ['a'],
//      ['B'] = ['b'],
//      ['C'] = ['c'],
//      ['D'] = ['d'],
//      ['E'] = ['e'],
//      ['F'] = ['f'],
//      ['G'] = ['g'],
//      ['H'] = ['h'],
//      ['I'] = ['i'],
//      ['J'] = ['j'],
//      ['K'] = ['k'],
//      ['L'] = ['l'],
//      ['M'] = ['m'],
//      ['N'] = ['n'],
//      ['O'] = ['o'],
//      ['P'] = ['p'],
//      ['Q'] = ['q'],
//      ['R'] = ['r'],
//      ['S'] = ['s'],
//      ['T'] = ['t'],
//      ['U'] = ['u'],
//      ['V'] = ['v'],
//      ['W'] = ['w'],
//      ['X'] = ['x'],
//      ['Y'] = ['y'],
//      ['Z'] = ['z'],
// };






__inline__ size_t reader_next(clexstate_t *state, int n) {
    int i;
    for (i = 0; state->buff[state->index+i] && (i < n); i++) {
        if (state->buff[state->index] == '\n') {
            state->line++;
            state->line_index = state->index;
            state->column = 0;
        } else {
            state->column++;
        }
    }
    // for (int i = 0; i > n; i--) {
    //     if (state->buff[state->index[0]] == '\n') {
    //         state->line--;
    //         state->column = -1;
    //     } else {
    //         state->column--;
    //     }
    // }
    return (state->index += i);
}




TODO
__finish __inline__ char read(clexstate_t *state, int index) {(void)state; (void)index; return 0;}
__finish __inline__ size_t token_start(clexstate_t *state) {(void)state; return 0;}
__finish __inline__ size_t token_end(clexstate_t *state) {(void)state; return 0;}



TODO
__finish __inline__ void token_setup(clexstate_t *state, token_t *token, uint8_t type, uint8_t tid, uint16_t flags) {
    (void)state; (void)token; (void)type; (void)tid; (void)flags;
    #if 0
    *token = (token_t){
        .start  = state->buff + state->token_start,
        .len    = state->index - state->token_start,
        .line   = state->line,
        .column = state->column,
        .type   = type,
        .tid    = tid,
        .flags  = flags,
    };
    #endif
}





__inline__ char _nib2char(uint8_t n) {
    n &= 0x0F;
    if (n < 10) return n + '0';
    return n - 9 + 'F';
}




// don't call more than once before using the string
__inline__ const char *cchar(char c) {
    static char s[5] = "\\x00";
    if (is_source(c)) {
        s[3] = c;
        return s+3;
    } else {
        s[2] = _nib2char(c >> 4);
        s[3] = _nib2char(c & 0x0F);
        return s;
    }
}





#if 0
#ifdef __OPTIMIZE__
__force_inline bool _bcmp(const char *restrict str1, const char *restrict str2, int len)
#else
static bool _bcmp(const char *restrict str1, const char *restrict str2, int len)
#endif
{
    switch (len) {
        case 1:  return str1[0] == str2[0];
        case 2:  return *(const int16_t*)&str1[0] == *(const int16_t*)&str2[0];
        case 4:  return *(const int32_t*)&str1[0] == *(const int32_t*)&str2[0];
        case 8:  return *(const int64_t*)&str1[0] == *(const int64_t*)&str2[0];
        default: exit(1);
    }
}
#endif



// __inline__ __fold_switch bool _fcmp1(const char *restrict str1, const char *restrict str2, int len) {
//     switch (len) {
//         case 0:  return true;
//         case 1:  return _bcmp(str1, str2, 1);
//         case 2:  return _bcmp(str1, str2, 2);
//         case 3:  return _bcmp(str1, str2, 2) && _bcmp(str1+2, str2+2, 1);
//         case 4:  return _bcmp(str1, str2, 4);
//         case 5:  return _bcmp(str1, str2, 4) && _bcmp(str1+4, str2+4, 1);
//         case 6:  return _bcmp(str1, str2, 4) && _bcmp(str1+4, str2+4, 2);
//     }
// }
// 
// 
// 
// __inline__ __fold_switch bool _fcmp(const char *restrict str1, const char *restrict str2, int len) {
//     switch (len) {
//         case 0:  return true;
//         case 1:  return _bcmp(str1, str2, 1);
//         case 2:  return _bcmp(str1, str2, 2);
//         case 3:  return _bcmp(str1, str2, 2) &&  _bcmp(str1+2, str2+2, 1);
//         case 4:  return _bcmp(str1, str2, 4);
//         case 5:  return _bcmp(str1, str2, 4) &&  _bcmp(str1+4, str2+4, 1);
//         case 6:  return _bcmp(str1, str2, 4) &&  _bcmp(str1+4, str2+4, 2);
//         case 7:  return _bcmp(str1, str2, 4) && _fcmp1(str1+4, str2+4, 3);
//         case 8:  return _bcmp(str1, str2, 8);
//         case 9:  return _bcmp(str1, str2, 8) &&  _bcmp(str1+8, str2+8, 1);
//         case 10: return _bcmp(str1, str2, 8) &&  _bcmp(str1+8, str2+8, 2);
//         case 11: return _bcmp(str1, str2, 8) && _fcmp1(str1+8, str2+8, 3);
//         case 12: return _bcmp(str1, str2, 8) &&  _bcmp(str1+8, str2+8, 4);
//         case 13: return _bcmp(str1, str2, 8) && _fcmp1(str1+8, str2+8, 5);
//         case 14: return _bcmp(str1, str2, 8) && _fcmp1(str1+8, str2+8, 6);
//         default: return !memcmp(str1, str2, len);
//     }
// }


// it was a fun idea, but it also allows for the loading of misaligned memory, so...
// I imagine that memcmp probably does a better job at this than me, so...
# if 0
#ifdef __OPTIMIZE__
// __inline__ bool fcmp(const char *restrict str1, const char *restrict str2, int len)
__force_inline bool fcmp(const char *restrict str1, const char *restrict str2, int len)
#else
static bool fcmp(const char *restrict str1, const char *restrict str2, int len)
#endif
{
    switch (len) {
        case 0:  return true;
        case 1:  return _bcmp(str1, str2, 1);
        case 2:  return _bcmp(str1, str2, 2);
        case 3:  return _bcmp(str1, str2, 2) && _bcmp(str1+2, str2+2, 1);
        case 4:  return _bcmp(str1, str2, 4);
        case 5:  return _bcmp(str1, str2, 4) && _bcmp(str1+4, str2+4, 1);
        case 6:  return _bcmp(str1, str2, 4) && _bcmp(str1+4, str2+4, 2);
        case 7:  return _bcmp(str1, str2, 4) &&  fcmp(str1+4, str2+4, 3);
        case 8:  return _bcmp(str1, str2, 8);
        case 9:  return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 1);
        case 10: return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 2);
        case 11: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 3);
        case 12: return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 4);
        case 13: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 5);
        case 14: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 6);
        case 15: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 7);
        case 16: return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 8);
        case 17: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 9);
        case 18: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 10);
        case 19: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 11);
        case 20: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 12);
        case 21: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 13);
        case 22: return _bcmp(str1, str2, 8) &&  fcmp(str1+8, str2+8, 14);
        default: return !memcmp(str1, str2, len);
    }
}
# endif

__force_inline bool fcmp(const char *restrict str1, const char *restrict str2, int len) {
    return !memcmp(str1, str2, len);
}



// __inline__ bool fcmp_oneof(const char *restrict str1, const char *restrict list[], int len) {}



#define FCMP(__buff, __str) (fcmp((__buff), (__str), sizeof(__str)-1))
    
#define FCMP1(__buff, __len, __str) \
    (sizeof(__str)-1 == (__len) && fcmp((__buff)+1, (__str)+1, sizeof(__str)-2))

#define FCMP2(__buff, __len, __str) \
    (sizeof(__str)-1 == (__len) && fcmp((__buff)+1, (__str)+2, sizeof(__str)-3))





__inline__ bool is_slcomment(const char *c) {
    return FCMP(c, "//");
}

__inline__ bool is_mlcomment(const char *c) {
    return FCMP(c, "/*");
}

// __inline__ bool is_encoding_prefix(char *c) {
//     return *c == 'u' ||
//            *c == 'U' ||
//            *c == 'L' ||
//            FCMP(c, "u8");
// }

// returns length of prefix
__inline__ int8_t is_encoding_prefix(const char *c) {
    // if (FCMP(c, "u8") return 2;
    // if (*c == 'u' ||
    //     *c == 'U' ||
    //     *c == 'L' ||) return 1;
    switch (c[0]) {
        case 'u':
            if (c[1] == '8') return 2;
            __fallthrough;
        case 'U':
        case 'L':
            return 1;
        default:
            return 0;
    }
}

__inline__ bool is_sstring(const char *c) {
    // return c[0] == '"' || is_encoding_prefix(c) && c[1] == '"';
    return c[is_encoding_prefix(c)] == '"';
}

__inline__ bool is_cstring(const char *c) {
    // return c[0] == '\'' || is_encoding_prefix(c) && c[1] == '\'';
    return c[is_encoding_prefix(c)] == '\'';
}

// TODO: maybe I should have the parameter be `bool *escape`, and have it be managed by this
__inline__ bool is_schar(const char *c, bool escape) {
    // return is_source(c[0]) && (c[0] != '"' || c[-1] == '\\');
    return is_source(c[0]) && (escape || c[0] != '"');
}

// TODO: maybe I should have the parameter be `bool *escape`, and have it be managed by this
__inline__ bool is_cchar(const char *c, bool escape) {
    // return is_source(c[0]) && (c[0] != '\'' || c[-1] == '\\');
    return is_source(c[0]) && (escape || c[0] != '\'');
}

__inline__ bool is_comment(const char *c) {
    return is_slcomment(c) || is_mlcomment(c);
}

__inline__ bool is_truenewline(const char *buff, size_t index) {
    return buff[index] && buff[index] == '\n' && (index < 1 || buff[index-1] != '\\');
}

__inline__ bool is_dec_digit(unsigned char c) {
    return (unsigned)(c-'0') <= '9'-'0';
}

__inline__ bool is_hex_digit(unsigned char c) {
    return ((unsigned)(c-'0') <= '9'-'0') || ((unsigned)(c-'a') <= 'f'-'a') || ((unsigned)(c-'A') <= 'F'-'A');
}

__inline__ bool is_oct_digit(unsigned char c) {
    return (unsigned)(c-'0') <= '7'-'0';
}

__inline__ bool is_bin_digit(unsigned char c) {
    return (unsigned)(c-'0') <= '1'-'0';
}

__inline__ bool is_sign(char c) {
    return c == '-' || c == '+';
}

// __inline__ char to_uppercase(char c) {
//     if (uppercase_map[c]) return uppercase_map[c];
//     return c;
// }
// 
// __inline__ char to_lowercase(char c) {
//     if (lowercase_map[c]) return lowercase_map[c];
//     return c;
// }


// TODO: replace (most) of these invocations with is_alpha
__inline__ int8_t is_floating_suffix(const char *c) {
    // return is_alpha(c[0]);

    TODO
    // TODO: fix this first conditional. It was really thrown on for an edge case
    // if (c[0] == 'f' || c[0] == 'l')
    if (c[0] == 'f' || (c[0] == 'l' && c[1] != 'l' &&  c[1] != 'u' &&  c[1] != 'U'))
        return 1;
    else if (c[0] == 'd')
        if (c[1] == 'f' || c[1] == 'd' || c[1] == 'l')
            return 2;
    else if (c[0] == 'D')
        if (c[1] == 'F' || c[1] == 'D' || c[1] == 'L')
            return 2;
    return 0;
//     switch (c[0]) {
//         case 'f':
//         case 'l':
//             return 1;
// 
//         case 'd':
//             switch (c[1]) {
//                 case 'f':
//                 case 'd':
//                 case 'l':
//                     return 2;
//             }
//             break;
// 
//         case 'D':
//             switch (c[1]) {
//                 case 'F':
//                 case 'D':
//                 case 'L':
//                     return 2;
//             }
//             break;
//     }
//     return 0;
}

// // returns true if whitespace contains a newline
// __inline__ bool skip_whitespace(clexstate_t *state) {
//     const char *restrict buff = state->buff;
//     size_t *restrict const index = &state->index;
//     bool is_newline = false;
//     
//     for (; is_whitespace(buff[index[0]]); reader_next(state, 1));
// 
//     return is_newline;
// }

// __inline__ void skip_slcomment(clexstate_t *state) {
//     const char *restrict buff = state->buff;
//     size_t *restrict const index = &state->index;
// 
//     // for (; buff[index[0]] && (buff[index[0]] != '\n' || FCMP(buff+index[0]-1, "\\\n")); index[0]++)
//     for (; buff[index[0]] && !is_truenewline(buff, index[0]); reader_next(state, 1));
// 
// }

// __inline__ void skip_mlcomment(clexstate_t *state) {
//     const char *restrict buff = state->buff;
//     size_t *restrict const index = &state->index;
// 
//     for (; buff[index[0]] && !FCMP(buff+index[0], "*/"); reader_next(state, 1));
// }







// note the length check makes first char collisions barely a problem, so dont worry about
// nesting switch statements until there are 6 or more char collisions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"

ktoken_id_t match_keyword(const char *str, int len) {
    if (len < (signed)sizeof("do")-1 || len > (signed)sizeof("static_assert")-1) return KTOKEN_INVALID;
    
    switch (str[0]) {
        case '_':
            switch (str[1]) {
                case 'A': if (FCMP2(str, len, "_Atomic"))       return KTOKEN_ATOMIC;       break;
                case 'B': if (FCMP2(str, len, "_BitInt"))       return KTOKEN_BITINT;       break;
                case 'C': if (FCMP2(str, len, "_Complex"))      return KTOKEN_COMPLEX;      break;
                case 'D': if (FCMP2(str, len, "_Decimal128"))   return KTOKEN_DECIMAL128;
                          if (FCMP2(str, len, "_Decimal64"))    return KTOKEN_DECIMAL64;
                          if (FCMP2(str, len, "_Decimal32"))    return KTOKEN_DECIMAL32;    break;
                case 'I': if (FCMP2(str, len, "_Imaginary"))    return KTOKEN_IMAGINARY;    break;
                case 'N': if (FCMP2(str, len, "_Noreturn"))     return KTOKEN_NORETURN;     break;
                case '_': if (FCMP2(str, len, "__attribute__")) return KTOKEN_ATTRIBUTE;    break;
            }                                                   return KTOKEN_INVALID;
        case 'a': if (FCMP1(str, len, "alignas"))   return KTOKEN_ALIGNAS;
                  if (FCMP1(str, len, "alignof"))   return KTOKEN_ALIGNOF;
                  if (FCMP1(str, len, "auto"))      return KTOKEN_AUTO;     break;
        case 'b': if (FCMP1(str, len, "bool"))      return KTOKEN_BOOL;
                  if (FCMP1(str, len, "break"))     return KTOKEN_BREAK;    break;
        case 'c': if (FCMP1(str, len, "case"))      return KTOKEN_CASE;
                  if (FCMP1(str, len, "char"))      return KTOKEN_CHAR;
                  if (FCMP1(str, len, "const"))     return KTOKEN_CONST;
                  if (FCMP1(str, len, "constexpr")) return KTOKEN_CONSTEXPR;
                  if (FCMP1(str, len, "continue"))  return KTOKEN_CONTINUE; break;
        case 'd': if (FCMP1(str, len, "default"))   return KTOKEN_DEFAULT;
                  if (FCMP1(str, len, "do"))        return KTOKEN_DO;
                  if (FCMP1(str, len, "double"))    return KTOKEN_DOUBLE;   break;
        case 'e': if (FCMP1(str, len, "else"))      return KTOKEN_ELSE;
                  if (FCMP1(str, len, "enum"))      return KTOKEN_ENUM;
                  if (FCMP1(str, len, "extern"))    return KTOKEN_EXTERN;   break;
        case 'f': if (FCMP1(str, len, "false"))     return KTOKEN_FALSE;
                  if (FCMP1(str, len, "float"))     return KTOKEN_FLOAT;
                  if (FCMP1(str, len, "for"))       return KTOKEN_FOR;      break;
        case 'g': if (FCMP1(str, len, "goto"))      return KTOKEN_GOTO;     break;
        case 'i': if (FCMP1(str, len, "if"))        return KTOKEN_IF;
                  if (FCMP1(str, len, "inline"))    return KTOKEN_INLINE;
                  if (FCMP1(str, len, "int"))       return KTOKEN_INT;      break;
        case 'l': if (FCMP1(str, len, "long"))      return KTOKEN_LONG;     break;
        case 'n': if (FCMP1(str, len, "nullptr"))   return KTOKEN_NULLPTR;  break;
        case 'r': if (FCMP1(str, len, "register"))  return KTOKEN_REGISTER;
                  if (FCMP1(str, len, "restrict"))  return KTOKEN_RESTRICT;
                  if (FCMP1(str, len, "return"))    return KTOKEN_RETURN;   break;
        case 's':
            switch (str[1]) {
                case 'h': if (FCMP2(str, len, "short"))         return KTOKEN_SHORT;    break;
                case 'i': if (FCMP2(str, len, "signed"))        return KTOKEN_SIGNED;
                          if (FCMP2(str, len, "sizeof"))        return KTOKEN_SIZEOF;   break;
                case 't': if (FCMP2(str, len, "static"))        return KTOKEN_STATIC;
                          if (FCMP2(str, len, "static_assert")) return KTOKEN_STATIC_ASSERT;
                          if (FCMP2(str, len, "struct"))        return KTOKEN_STRUCT;   break;
                case 'w': if (FCMP2(str, len, "switch"))        return KTOKEN_SWITCH;   break;
            }                                                   return KTOKEN_INVALID;
        case 't': if (FCMP1(str, len, "thread_local"))  return KTOKEN_THREAD_LOCAL;
                  if (FCMP1(str, len, "true"))          return KTOKEN_TRUE;
                  if (FCMP1(str, len, "typedef"))       return KTOKEN_TYPEDEF;
                  if (FCMP1(str, len, "typeof"))        return KTOKEN_TYPEOF;
                  if (FCMP1(str, len, "typeof_unqual")) return KTOKEN_TYPEOF_UNQUAL;    break;
        case 'u': if (FCMP1(str, len, "union"))         return KTOKEN_UNION;
                  if (FCMP1(str, len, "unsigned"))      return KTOKEN_UNSIGNED;         break;
        case 'v': if (FCMP1(str, len, "void"))          return KTOKEN_VOID;
                  if (FCMP1(str, len, "volatile"))      return KTOKEN_VOLATILE;         break;
        case 'w': if (FCMP1(str, len, "while"))         return KTOKEN_WHILE;            break;
    }                                                   return KTOKEN_INVALID;
}
#pragma GCC diagnostic pop



// // TODO: consider just not allowing bools and have them be lib defined like in older C standards
// // that way we dont need this predefined constant obfuscation
// // Oooh. That OR I can just have these be globally defined variables, so lets just ignore this
// // exists, yeah?
// bool is_predef_const(ktoken_id_t kid) {
//     switch (kid) {
//         case KTOKEN_TRUE:    return true;
//         case KTOKEN_FALSE:   return true;
//         case KTOKEN_NULLPTR: return true;
//         default:          return false;
//     }
// }




#define STRCMP(__buff, __str) (!memcmp((__buff), (__str), sizeof(__str)-1))


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"

ptoken_id_t match_punctuator(const char *restrict str, int *restrict len) {
    switch (str[0]) {
        case '!': if (fcmp(str+1, "!="+1,  1))  return (*len = 2, PTOKEN_NEQUALS);
                                                return (*len = 1, PTOKEN_EXCLAIM);
        case '%': if (fcmp(str+1, "%="+1,  1))  return (*len = 2, PTOKEN_MOD_EQ);
                                                return (*len = 1, PTOKEN_PERCENT);
        case '&': if (fcmp(str+1, "&&"+1,  1))  return (*len = 2, PTOKEN_LAND);
                  if (fcmp(str+1, "&="+1,  1))  return (*len = 2, PTOKEN_AND_EQ);
                                                return (*len = 1, PTOKEN_AND);
        case '(':                               return (*len = 1, PTOKEN_L_RBRACKET);
        case ')':                               return (*len = 1, PTOKEN_R_RBRACKET);
        case '*': if (fcmp(str+1, "*/"+1,  1))  return (*len = 2, PTOKEN_CBLOCK_END);
                  if (fcmp(str+1, "*="+1,  1))  return (*len = 2, PTOKEN_MUL_EQ);
                                                return (*len = 1, PTOKEN_STAR);
        case '+': if (fcmp(str+1, "++"+1,  1))  return (*len = 2, PTOKEN_INCREMENT);
                  if (fcmp(str+1, "+="+1,  1))  return (*len = 2, PTOKEN_ADD_EQ);
                                                return (*len = 1, PTOKEN_PLUS);
        case ',':                               return (*len = 1, PTOKEN_COMMA);
        case '-': if (fcmp(str+1, "--"+1,  1))  return (*len = 2, PTOKEN_DECREMENT);
                  if (fcmp(str+1, "-="+1,  1))  return (*len = 2, PTOKEN_SUB_EQ);
                  if (fcmp(str+1, "->"+1,  1))  return (*len = 2, PTOKEN_R_ARROW);
                                                return (*len = 1, PTOKEN_MINUS);
        case '.': if (fcmp(str+1, "..."+1, 2))  return (*len = 3, PTOKEN_VARGS);
                                                return (*len = 1, PTOKEN_PERIOD);
        case '/': if (fcmp(str+1, "/*"+1,  1))  return (*len = 2, PTOKEN_CBLOCK_START);
                  if (fcmp(str+1, "//"+1,  1))  return (*len = 2, PTOKEN_COMMENT);
                  if (fcmp(str+1, "/="+1,  1))  return (*len = 2, PTOKEN_DIV_EQ);
                                                return (*len = 1, PTOKEN_FSLASH);
        case ':':                               return (*len = 1, PTOKEN_COLON);
        case ';':                               return (*len = 1, PTOKEN_SEMICOLON);
        case '<': if (fcmp(str+1, "<<="+1, 2))  return (*len = 3, PTOKEN_LSHIFT_EQ);
                  if (fcmp(str+1, "<<"+1,  1))  return (*len = 2, PTOKEN_LSHIFT);
                  if (fcmp(str+1, "<="+1,  1))  return (*len = 2, PTOKEN_LEQUAL);
                                                return (*len = 1, PTOKEN_L_ABRACKET);
        case '=': if (fcmp(str+1, "=="+1,  2))  return (*len = 2, PTOKEN_EQUALS);
                                                return (*len = 1, PTOKEN_ASSIGN);
        case '>': if (fcmp(str+1, ">>="+1, 2))  return (*len = 3, PTOKEN_RSHIFT_EQ);
                  if (fcmp(str+1, ">>"+1,  1))  return (*len = 2, PTOKEN_RSHIFT);
                  if (fcmp(str+1, ">="+1,  1))  return (*len = 2, PTOKEN_GEQUAL);
                                                return (*len = 1, PTOKEN_R_ABRACKET);
        case '?':                               return (*len = 1, PTOKEN_QUESTION);
        case '[':                               return (*len = 1, PTOKEN_L_SBRACKET);
        // case '\\':                              return (*len = 1, PTOKEN_BSLASH);
        case ']':                               return (*len = 1, PTOKEN_R_SBRACKET);
        case '^': if (fcmp(str+1, "^="+1,  2))  return (*len = 2, PTOKEN_XOR_EQ);
                                                return (*len = 1, PTOKEN_XOR);
        case '{':                               return (*len = 1, PTOKEN_L_CBRACKET);
        case '|': if (fcmp(str+1, "|="+1,  1))  return (*len = 2, PTOKEN_OR_EQ);
                  if (fcmp(str+1, "||"+1,  1))  return (*len = 2, PTOKEN_LOR);
                                                return (*len = 1, PTOKEN_OR);
        case '}':                               return (*len = 1, PTOKEN_R_CBRACKET);
        case '~':                               return (*len = 1, PTOKEN_TILDE);
    }                                           return (*len = 1, PTOKEN_INVALID);
}

#pragma GCC diagnostic pop






__inline__ void consume_hex_exponent(clexstate_t *state) {
    int i;
    const char *buff    = state->buff;
    size_t *const index = &state->index;
    
    // safety check, gets optimized out if this function is used right
    if (buff[index[0]] != 'p' && buff[index[0]] != 'P')
        lex_error(state, "Missing exponent in hex float.");
    // consume exponent prefix
    reader_next(state, 1);
    // consume sign (opt)
    if (is_sign(buff[index[0]])) reader_next(state, 1);
    // consume digits
    for(i = 0; consume_dec_digit(state); i++);

    if (i == 0)
        lex_error(state, "No digits in hex float exponent.");

    // return 0;
}




__inline__ void consume_exponent(clexstate_t *state) {
    int i;
    const char *buff    = state->buff;
    size_t *const index = &state->index;
    
    // safety check, gets optimized out if this function is used right
    if (buff[index[0]] != 'e' && buff[index[0]] != 'E')
        lex_error(state, "Missing exponent in float.");
    // consume exponent prefix
    reader_next(state, 1);
    // consume sign (opt)
    if (is_sign(buff[index[0]])) reader_next(state, 1);
    // consume digits
    for(i = 0; consume_dec_digit(state); i++);

    if (i == 0)
        lex_error(state, "No digits in float exponent.");
}




__inline__ token_flags_t consume_floating_suffix(clexstate_t *state) {
    int n;
    const char *buff    = state->buff;
    size_t *const index = &state->index;
    token_flags_t fsuffix = 0;
    // bool unvalid = true;
    
    // n = is_floating_suffix(buff+index[0]);
    // if (n == 0) lex_error(state, "Invalid floating suffix.");
    // else unvalid = false;

    // Just check for alpha char as a lookahead for this function

    n = 0;
    switch (buff[index[0]]) {
        case 'd':
            switch (buff[index[0]+1]) {
                case 'f': n = 2; fsuffix = TFLAG_FSUFFIX_DEC32;  break;
                case 'd': n = 2; fsuffix = TFLAG_FSUFFIX_DEC64;  break;
                case 'l': n = 2; fsuffix = TFLAG_FSUFFIX_DEC128; break;
            }
            break;
        case 'D':
            switch (buff[index[0]+1]) {
                case 'F': n = 2; fsuffix = TFLAG_FSUFFIX_DEC32;  break;
                case 'D': n = 2; fsuffix = TFLAG_FSUFFIX_DEC64;  break;
                case 'L': n = 2; fsuffix = TFLAG_FSUFFIX_DEC128; break;
            }
            break;
        case 'f':
        case 'F': n = 1; fsuffix = TFLAG_FSUFFIX_FLOAT; break;
        case 'l':
        case 'L': n = 1; fsuffix = TFLAG_FSUFFIX_LONG;  break;
        // default:
        //     lex_error(state, "Missing floating suffix after hex float.");
    }
    
    reader_next(state, n);

    printf("char %c\n", buff[index[0]]);
    
    // check for lingering alphanumeric characters
    if (is_alphanumeric(buff[index[0]]))
        lex_error(state, "Invalid floating suffix.");
    else if (n == 0)
        lex_error(state, "Missing floating suffix.");

    return fsuffix;
}




__inline__ token_flags_t consume_integer_suffix(clexstate_t *state) {
    int n;
    const char *buff    = state->buff;
    size_t *const index = &state->index;
    token_flags_t suffix = 0;
    // Just check for alpha char as a lookahead for this function

    // consume leading unsigned
    if (buff[index[0]] == 'u' || buff[index[0]] == 'U') {
        reader_next(state, 1);
        suffix |= TFLAG_IUNSIGN_TRUE;
    }

    n = 0;
    switch (buff[index[0]]) {
        case 'l':
            switch (buff[index[0]+1]) {
                case 'l': n = 2; suffix |= TFLAG_ISUFFIX_LLONG;  break;
                default:  n = 1; suffix |= TFLAG_ISUFFIX_LONG;
            }
            break;
        case 'L':
            switch (buff[index[0]+1]) {
                case 'L': n = 2; suffix |= TFLAG_ISUFFIX_LLONG;  break;
                default:  n = 1; suffix |= TFLAG_ISUFFIX_LONG;
            }
            break;
        case 'w': if (buff[index[0]+1] == 'b') (n = 2, suffix |= TFLAG_ISUFFIX_WB); break;
        case 'W': if (buff[index[0]+1] == 'B') (n = 2, suffix |= TFLAG_ISUFFIX_WB); break;
    }
    
    // consume trailing unsigned
    if (!(suffix & TFLAG_IUNSIGN_MASK) && (buff[index[0]+n] == 'u' || buff[index[0]+n] == 'U')) {
        reader_next(state, 1);
        suffix |= TFLAG_IUNSIGN_TRUE;
    }
    
    reader_next(state, n);
    
    // check for lingering alphanumeric characters
    if (is_alphanumeric(buff[index[0]]))
        lex_error(state, "Invalid integer suffix.");
    else if (n == 0)
        lex_error(state, "Missing integer suffix.");

    return suffix;
}




__inline__ bool consume_dec_digit(clexstate_t *state) {
    if (is_dec_digit(state->buff[state->index])) {
        reader_next(state, 1);
        return true;
    } else return false;
}


__inline__ bool consume_hex_digit(clexstate_t *state) {
    if (is_hex_digit(state->buff[state->index])) {
        reader_next(state, 1);
        return true;
    } else return false;
}


__inline__ bool consume_oct_digit(clexstate_t *state) {
    if (is_oct_digit(state->buff[state->index])) {
        reader_next(state, 1);
        return true;
    } else return false;
}


__inline__ bool consume_bin_digit(clexstate_t *state) {
    if (is_bin_digit(state->buff[state->index])) {
        reader_next(state, 1);
        return true;
    } else return false;
}


__inline__ bool consume_alphanumeric(clexstate_t *state) {
    if (is_alphanumeric(state->buff[state->index])) {
        reader_next(state, 1);
        return true;
    } else return false;
}








// as a consumption practice, all whitespace before a token is consumed, 
// and all whitespace after a token is consumed up until the next newline, including the newline
// This way it is trivial to detect if our next token starts on a newline
// no token should contain leading or trailing whitespace
// TODO: Note, that there is a rare chance of segfault if the buffer's end lies on a page boundry
//       Therefore, just add extra bytes on buffers you load.
void next_token(clexstate_t *restrict state, token_t *restrict token) {
    // int err;
    const char *restrict buff = state->buff;
    size_t *restrict const index = &state->index;
    // bool is_newline = state->line_index == index[0];
    bool is_newline = state->column == 0;
    
    // skip whitespace and comments, and parse linemarkers
    for (bool did_action = true; did_action;) {
        did_action = false;
        
        // whitespace check
        for (; is_whitespace(buff[index[0]]); reader_next(state, 1))
            did_action = true;
        
        // single-line comment check
        if (is_slcomment(buff+index[0]))
            for (; buff[index[0]] && !is_truenewline(buff, index[0]); reader_next(state, 1))
                did_action = true;
            
        // milti-line comment check
        if (is_mlcomment(buff+index[0]))
            for (; buff[index[0]] && !FCMP(buff+index[0], "*/"); reader_next(state, 1))
                did_action = true;
            
        // linemarker check
        if (is_newline && buff[index[0]] == '#') {
            parse_linemarker(state);
            did_action = true;
        }
    }

    // check string literal constant
    if (is_sstring(buff+index[0]))
        next_token_string_literal(state, token, TFLAG_STRING_S);

    // check char literal constant
    else if (is_cstring(buff+index[0]))
        next_token_string_literal(state, token, TFLAG_STRING_C);

    // check numeric constant
    else if (is_numeric(buff[index[0]]))
        next_token_numeric(state, token);

    // check alpha leading token
    else if (is_alpha(buff[index[0]])) {
        uint8_t kid;
    
        // pretend it is identifier
        next_token_identifier(state, token);

        // check if actually keyword
        kid = match_keyword(token->start, token->len);
        if (kid != KTOKEN_INVALID) {
            token->type = TOKEN_KEYWORD;
            token->tid = kid;
        }
    }
    // check punctuator
    else if (is_punctuator(buff[index[0]]))
        next_token_punctuator(state, token);

    // we can error now
    else
        lex_error(state, "Unexpected token \"%s\".", cchar(buff[index[0]]));

    // consume trailing whitespace before newline
    for (; is_whitespace(buff[index[0]]) && buff[index[0]] != '\n'; reader_next(state, 1));
    // consume newline
    if (buff[index[0]] == '\n') reader_next(state, 1);

    // if (err) return -1;
}






// TODO: Might need a filename tree structure for this. But implement that later.
// TODO: Also I should probably make this more robust in the future
// https://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
__todo static void parse_linemarker(clexstate_t *restrict state) {
    const char *restrict buff = state->buff;
    size_t *restrict const index = &state->index;
    int n = -1, flags = 0;
    const char *file = NULL;
    int filelen;

    // TODO: add fatal check gate here

    // Skip # char and space
    reader_next(state, 1);
    // buff[index[0]] != '\n' && reader_next(state, 1);
    if (buff[index[0]] != '\n') reader_next(state, 1);

    // match first number
    for (int i = 0; is_numeric(buff[index[0]]); reader_next(state, 1)) {
        i *= 10;
        i += buff[index[0]] - '0';
        n = i;
    }
    
    // Skip space
    if (buff[index[0]] != '\n') reader_next(state, 1);

    // match string
    // if (buff[index[0]] == '"') {
    //     buff[index[0]] != '\n' && reader_next(state, 1);
    //     for (int i = 0; buff[index[0]] != '\n' && buff[index[0]] != '"'); reader_next(state, 1), i++) {
    //         file = realloc(file, i+2);
    //         file[i] = buff[index[0]];
    //         file[i] = '\0';
    //     }
    // }
    filelen = 0;
    if (buff[index[0]] == '"' && buff[index[0]+1]) {
        if (buff[index[0]] != '\n') reader_next(state, 1);
        if (buff[index[0]]) file = buff+index[0];
        for (; buff[index[0]] != '"'; reader_next(state, 1), filelen++);
    }
    
    // consume everything else up to newline, consuming flags
    for (int i = 0; buff[index[0]] != '\n'; reader_next(state, 1), i++) {
        if (buff[index[0]] >= '1' && buff[index[0]] <= '4')
            flags |= 1<<(buff[index[0]] - '1');
    }
    
    state->line = n;
    state->filename = file;
    state->filenamelen = filelen;
    state->flags = flags;
}




// I suppose I should let the parser handle interpreting the string so I dont need to malloc
// a buffer for this.
// TODO: replace start and start_index with a function to start and end the token via the state
__todo static void next_token_string_literal(clexstate_t *restrict state, token_t *restrict token, uint8_t mode) {
    int n;
    const char *buff = state->buff;
    size_t *const index = &state->index;
    uint16_t flags;
    const char *start;
    bool escape;

    // TODO: Add fatal check gate here

    // consume prefix
    n = is_encoding_prefix(buff+index[0]);
    if (n == 1) {
        switch (buff[index[0]]) {
            case 'u': flags = TFLAG_ENCC_UTF16; break;
            case 'U': flags = TFLAG_ENCC_UTF32; break;
            case 'L': flags = TFLAG_ENCC_WIDE;  break;
            default: fatal(1);
        }
    } else if (n == 2) {
        if (buff[index[0]+1] == '8')
            flags = TFLAG_ENCC_UTF8;
        else fatal(1);

    } else {
        flags = TFLAG_ENCC_DEFAULT;
    }
    
    // consume prefix and quote
    reader_next(state, n+1);

    // consume string
    start = buff+index[0];
    escape = false;
    switch (mode) {
        case TFLAG_STRING_S:
            for (n = 0; is_schar(buff+index[0], escape); reader_next(state, 1), n++)
                if (buff[index[0]] == '\\' && !escape) escape = true;
                else escape = false;
            break;
        case TFLAG_STRING_C:
            for (n = 0; is_cchar(buff+index[0], escape); reader_next(state, 1), n++)
                if (buff[index[0]] == '\\' && !escape) escape = true;
                else escape = false;
            break;
    }

    // consume quote
    reader_next(state, 1);

    // create token
    *token = (token_t){
        .start  = start,
        .len    = buff + index[0] - start - 1,
        .line   = state->line,
        .column = state->column,
        .type   = TOKEN_CONSTANT,
        .tid    = CONST_STRING,
        .flags  = mode | flags,
    };
}




static void next_token_numeric(clexstate_t *restrict state, token_t *restrict token) {
    // int err;
    const char *buff = state->buff;
    size_t *const index = &state->index;

    #ifdef __DEBUG__
    // safety check
    if (!is_numeric(buff[index[0]]))
        fatal(1);
    #endif

    if (buff[index[0]] == '0') {
        if (buff[index[0]+1] == 'x' || buff[index[0]+1] == 'X')
            next_token_num_hex(state, token);
            
        else if (buff[index[0]+1] == 'b' || buff[index[0]+1] == 'B')
            next_token_num_bin(state, token);
            
        else
            next_token_num_misc(state, token);
            
    } else {
        next_token_num_misc(state, token);
    }

    // return err;
}






// > For hexadecimal floating constants, the exponent is not optional to avoid ambiguity resulting from an f suffix being mistaken as a hexadecimal digit. [https://en.cppreference.com/c/language/floating_constant]
__todo static void next_token_num_hex(clexstate_t *restrict state, token_t *restrict token) {
    int err, n, i;
    // const char *start;
    const char *buff    = state->buff;
    size_t start_index  = state->index;
    size_t *const index = &state->index;
    token_flags_t flags;
    token_const_type_t type;

    (void)n;

    // TODO: make floating suffix checks default to integer before float

    //// First attempt, won't work due to 'f' ambiguity
    // hex float:
    // 0x             . hex_digit^1 (S(pP) sign^-1 digit^1)^-1 floating_suffix^-1
    // 0x hex_digit^1 . hex_digit^0 (S(pP) sign^-1 digit^1)^-1 floating_suffix^-1
    // 0x hex_digit^1               (S(pP) sign^-1 digit^1)    floating_suffix^-1
    // 0x hex_digit^1                                          floating_suffix

    // hex float:
    // 0x             . hex_digit^1 (S(pP) sign^-1 digit^1)    floating_suffix^-1
    // 0x hex_digit^1 . hex_digit^0 (S(pP) sign^-1 digit^1)    floating_suffix^-1
    // 0x hex_digit^1               (S(pP) sign^-1 digit^1)    floating_suffix^-1

    // hex int:
    // 0x hex_digit^1 integer_suffix^-1

    // Only worry about storing the suffix, as the string can easily be decoded sequentially by the parser.
    // lexer makes sure it is valid syntax, to make it way easier for the parser to parse

    // err = 0;
    (void)err;

    flags = TFLAG_BASE_HEX;

    #ifdef __DEBUG__
    // safety check
    if (!(buff[index[0]] == '0' && (buff[index[0]+1] == 'x' || buff[index[0]+1] == 'X')))
        fatal(1);
    #endif

    // consume hex prefix
    reader_next(state, 2);

    // check for leading period
    if (buff[index[0]] == '.') {
        type = CONST_FLOATING;
    
        // consume hex digits
        for (i = 0; consume_hex_digit(state); i++);
        if (i == 0) lex_error(state, "Invalid hex fractional.");

//         // consume exponent (opt)
//         if (buff[index[0]] == 'p' || buff[index[0]] == 'P')
//             consume_hex_exponent(state);
// 
//         // consume float suffix (opt)
//         if (is_floating_suffix(buff+index[0]))
//              flags |= consume_floating_suffix(state);
//         else flags |= TFLAG_FSUFFIX_DEFAULT;
    
    // check for leading hex digits
    } else if (is_hex_digit(buff[index[0]])) {
        // consume hex digits
        while(consume_hex_digit(state));

        // check for period
        if (buff[index[0]] == '.') {
            type = CONST_FLOATING;
            
            // consume period
            reader_next(state, 1);
            // consume hex digits (opt)
            while(consume_hex_digit(state));

//             // consume exponent (opt)
//             if (buff[index[0]] == 'p' || buff[index[0]] == 'P')
//                 consume_hex_exponent(state);
// 
//             // consume floating suffix (opt)
//             // n = is_floating_suffix(buff+index[0]);
//             // reader_next(state, n);
//             if (is_floating_suffix(buff+index[0]))
//                  flags |= consume_floating_suffix(state);
//             else flags |= TFLAG_FSUFFIX_DEFAULT;

        // check for exponent
        } else if (buff[index[0]] == 'p' || buff[index[0]] == 'P') {
            type = CONST_FLOATING;
            
//             // consume exponent
//             consume_hex_exponent(state);
// 
//             // consume floating suffix (opt)
//             if (is_floating_suffix(buff+index[0]))
//                  flags |= consume_floating_suffix(state);
//             else flags |= TFLAG_FSUFFIX_DEFAULT;

//         // check for floating suffix
//         } else if (is_floating_suffix(buff+index[0])) {
//             type = CONST_FLOATING;
// 
//             // consume floating suffix (opt)
//             flags |= consume_floating_suffix(state);

        // this is an integer
        } else {
            type = CONST_INTEGER;

            // consume integer suffix (opt)
            if (is_alpha(buff[index[0]]))
                flags |= consume_integer_suffix(state);
        }
    } else {
        fatal(1);
    }

    if (type == CONST_FLOATING) {
        // consume exponent (opt)
        consume_hex_exponent(state);

        // consume float suffix (opt)
        // if (is_floating_suffix(buff+index[0]))
        if (is_alpha(buff[index[0]]))
             flags |= consume_floating_suffix(state);
        else flags |= TFLAG_FSUFFIX_DEFAULT;
    }

    // check for extra alphanumeric
    if (is_alpha(buff[index[0]]))
        if (type == CONST_FLOATING)
            lex_error(state, "Unexpected character \"%s\" in hex float.", cchar(buff[index[0]]));
        else
            lex_error(state, "Unexpected character \"%s\" in hex integer.", cchar(buff[index[0]]));

    // create token
    *token = (token_t){
        .start  = buff + start_index,
        .len    = index[0] - start_index + 1,
        .line   = state->line,
        .column = state->column,
        .type   = TOKEN_CONSTANT,
        .tid    = type,
        .flags  = flags,
    };
}






__todo static void next_token_num_misc(clexstate_t *restrict state, token_t *restrict token) {
    int i;
    // uint16_t flags;
    // const char *start;
    const char *buff    = state->buff;
    size_t start_index  = state->index;
    size_t *const index = &state->index;
    token_flags_t flags;
    token_const_type_t type;
    bool start_zero, octal_valid;
    // bool is_octal;

    // TODO: make floating suffix checks default to integer before float

    // float:
    //         . digit^1 ((S(eE) sign^-1 digit^1))^-1 floating_suffix^-1
    // digit^1 . digit^0 ((S(eE) sign^-1 digit^1))^-1 floating_suffix^-1
    // digit^1           ((S(eE) sign^-1 digit^1))    floating_suffix^-1
    // digit^1                                        floating_suffix

    // int:
    // nonzero_digit digit^0 integer_suffix^-1

    // oct:
    // 0 digit^0 integer_suffix^-1

    // Only worry about storing the suffix, as the string can easily be decoded sequentially by the parser.
    // lexer makes sure it is valid syntax, to make it way easier for the parser to parse

    flags = TFLAG_BASE_DEC;

    #ifdef __DEBUG__
    // safety check
    if (!is_numeric(buff[index[0]]))
        fatal(1);
    #endif

    start_zero = buff[index[0]] == '0';
    octal_valid = true;
    // is_octal = buff[index[0]] == '0';

    // check for leading period
    if (buff[index[0]] == '.') {
        type = CONST_FLOATING;
    
        // consume digits
        for (i = 0; consume_dec_digit(state); i++);
        if (i == 0) lex_error(state, "Invalid fractional.");

        // consume exponent (opt)
        if (buff[index[0]] == 'e' || buff[index[0]] == 'E')
            consume_exponent(state);

        // consume float suffix (opt)
        // if (is_floating_suffix(buff+index[0]))
        if (is_alpha(buff[index[0]]))
             flags |= consume_floating_suffix(state);
        else flags |= TFLAG_FSUFFIX_DEFAULT;
    
    // check for leading digits
    } else if (is_dec_digit(buff[index[0]])) {
        // consume digits, track if octal compatable
        while(is_dec_digit(buff[index[0]])) {
            if (!is_oct_digit(buff[index[0]]))
                octal_valid = false;
            reader_next(state, 1);
        }

        // check for period
        if (buff[index[0]] == '.') {
            type = CONST_FLOATING;
            
            // consume period
            reader_next(state, 1);
            // consume digits (opt)
            while(consume_dec_digit(state));

            // consume exponent (opt)
            if (buff[index[0]] == 'e' || buff[index[0]] == 'E')
                consume_exponent(state);

            // if (is_floating_suffix(buff+index[0]))
            if (is_alpha(buff[index[0]]))
                 flags |= consume_floating_suffix(state);
            else flags |= TFLAG_FSUFFIX_DEFAULT;

        // check for exponent
        } else if (buff[index[0]] == 'e' || buff[index[0]] == 'E') {
            type = CONST_FLOATING;
            
            // consume exponent
            consume_exponent(state);

            // consume floating suffix (opt)
            // if (is_floating_suffix(buff+index[0]))
            if (is_alpha(buff[index[0]]))
                 flags |= consume_floating_suffix(state);
            else flags |= TFLAG_FSUFFIX_DEFAULT;

        // check for floating suffix
        } else if (is_floating_suffix(buff+index[0])) {
            type = CONST_FLOATING;

            // consume floating suffix (opt)
            flags |= consume_floating_suffix(state);

        // this is an integer
        } else {
            type = CONST_INTEGER;

            // check if octal
            if (start_zero)
                if (octal_valid)
                    flags = TFLAG_BASE_OCT;
                else
                    lex_error(state, "Invalid characters in octal integer.");

            // consume integer suffix (opt)
            if (is_alpha(buff[index[0]]))
                flags |= consume_integer_suffix(state);
        }
    } else {
        fatal(1);
    }

    // check for extra alphanumeric
    if (is_alpha(buff[index[0]]))
        if (type == CONST_FLOATING)
            lex_error(state, "Unexpected character \"%s\" in float.", 
                    cchar(buff[index[0]]));
        else
            lex_error(state, "Unexpected character \"%s\" in integer.", 
                    cchar(buff[index[0]]));

    // create token
    *token = (token_t){
        .start  = buff + start_index,
        .len    = index[0] - start_index,
        .line   = state->line,
        .column = state->column,
        .type   = TOKEN_CONSTANT,
        .tid    = type,
        .flags  = flags,
    };
}






static void next_token_num_bin(clexstate_t *restrict state, token_t *restrict token) {
    int i;
    // uint16_t flags;
    // const char *start;
    const char *buff    = state->buff;
    size_t start_index  = state->index;
    size_t *const index = &state->index;
    token_flags_t flags;

    // 0b binary_digit^1 integer_suffix^-1

    #ifdef __DEBUG__
    // safety check
    if (!(buff[index[0]] == '0' && (buff[index[0]+1] != 'b' || buff[index[0]+1] == 'B')))
    fatal(1);
    #endif

    flags = TFLAG_BASE_BIN;

    // consume binary prefix
    reader_next(state, 2);

    // consume binary digits
    for (i = 0; consume_bin_digit(state); i++);
    if (i == 0) lex_error(state, "Invalid binary constant.");

    // consume integer suffix (opt)
    if (is_alpha(buff[index[0]]))
        flags |= consume_integer_suffix(state);

    // check for extra alphanumeric
    if (is_alpha(buff[index[0]]))
        lex_error(state, "Unexpected character \"%s\" in binary integer.", cchar(buff[index[0]]));

    // create token
    *token = (token_t){
        .start  = buff + start_index,
        .len    = index[0] - start_index,
        .line   = state->line,
        .column = state->column,
        .type   = TOKEN_CONSTANT,
        .tid    = CONST_INTEGER,
        .flags  = flags,
    };
}





static void next_token_identifier(clexstate_t *restrict state, token_t *restrict token) {
    const char *buff    = state->buff;
    size_t start_index  = state->index;
    size_t *const index = &state->index;

    #ifdef __DEBUG__
    // safety check
    if (!is_alpha(buff[index[0]])) fatal(1);
    #endif

    // consume alphanumeric tokens
    while (consume_alphanumeric(state));

    // create token
    *token = (token_t){
        .start  = buff + start_index,
        .len    = index[0] - start_index,
        .line   = state->line,
        .column = state->column,
        .type   = TOKEN_IDENTIFIER,
    };
}




static void next_token_punctuator(clexstate_t *restrict state, token_t *restrict token) {
    const char *buff    = state->buff;
    size_t start_index  = state->index;
    size_t *const index = &state->index;
    ptoken_id_t tid;
    int len;

    #ifdef __DEBUG__
    // safety check
    if (!is_punctuator(buff[index[0]])) fatal(1);
    #endif

    // consume punctuator
    tid = match_punctuator(buff + index[0], &len);
    reader_next(state, len);

    if (tid == PTOKEN_INVALID)
        lex_error(state, "Unknown punctuator \"%c\".", buff[index[0]]);

    // create token
    *token = (token_t){
        .start  = buff + start_index,
        .len    = index[0] - start_index,
        .line   = state->line,
        .column = state->column,
        .type   = TOKEN_PUNCTUATOR,
        .tid    = tid,
    };
}





// __inline__ void clexer_init(clexstate_t *restrict state, const char *restrict buff) {
void clexer_init(clexstate_t *restrict state, const char *restrict buff) {
    *state = (clexstate_t){
        .buff = buff,
        .index = 0,
        .line = 0,
        .line_index = 0,
        .column = 0,
    };
}





void print_token(const token_t *token, int num) {
    const char *type;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmisleading-indentation"
    switch (token->type) {
        case TOKEN_KEYWORD:             type = "keyword    "; break;
        case TOKEN_PUNCTUATOR:          type = "punctuator "; break;
        case TOKEN_IDENTIFIER:          type = "identifier "; break;
        case TOKEN_CONSTANT:
            switch (token->tid) {
                case CONST_FLOATING:    type = "floating   "; break;
                case CONST_INTEGER:     type = "integer    "; break;
                case CONST_PREDEFINED:  type = "predefined "; break;
                case CONST_STRING:
                    if ((token->flags & TFLAG_STRING_MASK) == TFLAG_STRING_S)
                                        type = "string     ";
                    else                type = "char string"; break;
                default:                type = "invalid constant";
            }
            break;
        default:    type = "invalid token";
    }
    #pragma GCC diagnostic pop
    
    printf("TOKEN %5d\t %.*s:%u:%-3u\t%s\t\"%.*s\"\n",
        num,
        (int)sizeof("nofile"), "nofile",
        token->line,
        token->column,
        type,
        (int)(token->len < 50 ? token->len : 50), token->start
    );
}






static __noreturn void lex_error(clexstate_t *state, const char *fmt, ...) {
    const char *start;
    int len;

    va_list args;
    va_start(args, fmt);

    start = state->buff + state->line_index;
    for (len = 0; start[len] && start[len] != '\n'; len++);
    
    fprintf(stderr, "%.*s:%d:%d: error: ", state->filenamelen, state->filename, state->line, state->column);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n%5d | %.*s\n", state->line, len, start); 
    
    va_end(args);
    exit(2);
}
