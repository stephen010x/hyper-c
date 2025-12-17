#include <stdint.h>
#include <stdbool.h>

#include "hyperc/descriptors.h"


const bool whitespace_map[256] = {
    //[0 ... 255] = false,
    [' ']  = true,
    ['\t'] = true,
    ['\n'] = true,
    ['\v'] = true,
    ['\f'] = true,
    ['\r'] = true,
};



const uint8_t alphanumeric_map[256] = {
    //[0 ... 255] = 0,
    ['0' ... '9'] = IS_NUMERIC,
    ['a' ... 'z'] = IS_ALPHA,
    ['A' ... 'Z'] = IS_ALPHA,
    ['_']         = IS_ALPHA,
};




// delimited tokens
const char *const dtokens[DTOKEN_LENGTH] = {
    
    [DTOKEN_AUTO]     = "auto",
    [DTOKEN_VOID]     = "void",
    [DTOKEN_UNSIGNED] = "unsigned",
    [DTOKEN_SIGNED]   = "signed",
    [DTOKEN_BOOL]     = "bool",
    [DTOKEN_CHAR]     = "char",
    [DTOKEN_SHORT]    = "short",
    [DTOKEN_LONG]     = "long",
    [DTOKEN_INT]      = "int",
    [DTOKEN_FLOAT]    = "float",
    [DTOKEN_DOUBLE]   = "double",

    [DTOKEN_ENUM]     = "enum",
    [DTOKEN_STRUCT]   = "struct",
    [DTOKEN_UNION]    = "union",

    [DTOKEN_INLINE]   = "inline",
    [DTOKEN_CONST]    = "const",
    [DTOKEN_STATIC]   = "static",
    [DTOKEN_EXTERN]   = "extern",
    [DTOKEN_REGISTER] = "register",
    [DTOKEN_RESTRICT] = "restrict",
    [DTOKEN_VOLATILE] = "volatile",

    [DTOKEN_FALSE]    = "false",
    [DTOKEN_TRUE]     = "true",
    [DTOKEN_NULLPTR]  = "nullptr",

    [DTOKEN_IF]       = "if",
    [DTOKEN_ELSE]     = "else",
    [DTOKEN_WHILE]    = "while",
    [DTOKEN_FOR]      = "for",
    [DTOKEN_DO]       = "do",
    [DTOKEN_SWITCH]   = "switch",
    [DTOKEN_GOTO]     = "goto",
    [DTOKEN_RETURN]   = "return",

    [DTOKEN_CONTINUE] = "continue",
    [DTOKEN_BREAK]    = "break",
    [DTOKEN_CASE]     = "case",
    [DTOKEN_DEFAULT]  = "default",

    [DTOKEN_ALIGNAS]  = "alignas",
    [DTOKEN_ALIGNOF]  = "alignof",
    [DTOKEN_SIZEOF]   = "sizeof",
    [DTOKEN_TYPEDEF]  = "typedef",
    [DTOKEN_TYPEOF]   = "typeof",

    [DTOKEN_CONSTEXPR]     = "constexpr",
    [DTOKEN_STATIC_ASSERT] = "static_assert",
    [DTOKEN_THREAD_LOCAL]  = "thread_local",
    [DTOKEN_TYPEOF_UNQUAL] = "typeof_unqual",
    
    [DTOKEN_ATTRIBUTE]     = "__attribute__",
    // TODO: remember to reserve all characters with one or two underscores, followed by an uppercase letter
};





// undelimited tokens
// NOTE, these are processed in this order. So have the longest ones at the beginning
// or at least make sure nothing that comprises longer operators are in front.
const char *const utokens[UTOKEN_LENGTH] = {

    [UTOKEN_ARROW]        = "->",
    
    [UTOKEN_LOR]          = "||",
    [UTOKEN_LAND]         = "&&",
    [UTOKEN_EQUALS]       = "==",

    [UTOKEN_INCREMENT]    = "++",
    [UTOKEN_DECREMENT]    = "--",
    [UTOKEN_LSHIFT]       = "<<",
    [UTOKEN_RSHIFT]       = ">>",
    
    [UTOKEN_NEQUALS]      = "!=",
    [UTOKEN_ADD_EQ]       = "+=",
    [UTOKEN_SUB_EQ]       = "-=",
    [UTOKEN_MUL_EQ]       = "*=",
    [UTOKEN_DIV_EQ]       = "/=",
    [UTOKEN_MOD_EQ]       = "%=",
    [UTOKEN_AND_EQ]       = "&=",
    [UTOKEN_XOR_EQ]       = "^=",
    [UTOKEN_OR_EQ]        = "|=",
    [UTOKEN_LEQUAL]       = "<=",
    [UTOKEN_GEQUAL]       = ">=",
    [UTOKEN_LSHIFT_EQ]    = "<<=",
    [UTOKEN_RSHIFT_EQ]    = ">>=",
    
    [UTOKEN_COMMENT]      = "//",
    [UTOKEN_CBLOCK_START] = "/*",
    [UTOKEN_CBLOCK_END]   = "*/",
    
    //{UTOKEN_BSLASHNEW,   "\\\n" }, // this might just be a preprocessor thing

    [UTOKEN_L_RBRACKET]   = "(",
    [UTOKEN_R_RBRACKET]   = ")",
    [UTOKEN_L_SBRACKET]   = "[",
    [UTOKEN_R_SBRACKET]   = "]",
    // TODO: create better lexer rules to differentiate between
    // angle brackets and gtr-than and less-than. And to also be able to distinguish beteen
    // bit shift << and nested brackets.
    // Actually, I guess my parser is handling grouping, so it doesn't need to be here.
    [UTOKEN_L_ABRACKET]   = "<",
    [UTOKEN_R_ABRACKET]   = ">",
    [UTOKEN_L_CBRACKET]   = "{",
    [UTOKEN_R_CBRACKET]   = "}",
    
    [UTOKEN_PERIOD]       = ".",
    [UTOKEN_UNDERSCORE]   = "_",
    [UTOKEN_COMMA]        = ",",
    [UTOKEN_COLON]        = ":",
    [UTOKEN_SEMICOLON]    = ";",
    
    [UTOKEN_PLUS]         = "+",
    [UTOKEN_MINUS]        = "-",
    [UTOKEN_STAR]         = "*",
    [UTOKEN_FSLASH]       = "/",
    [UTOKEN_PERCENT]      = "%",
    [UTOKEN_AND]          = "&",
    [UTOKEN_XOR]          = "^",
    [UTOKEN_OR]           = "|",
    
    [UTOKEN_TILDE]        = "~",
    [UTOKEN_EXCLAIM]      = "!",
    
    [UTOKEN_QUESTION]     = "?",
    [UTOKEN_ASSIGN]       = "=",
    [UTOKEN_BSLASH]       = "\\",
    
};
