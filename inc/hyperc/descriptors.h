#ifndef HYPERC_DESCRIPTORS_H
#define HYPERC_DESCRIPTORS_H

#include "hyperc/macros.h"


/*
	- Keywords
	- Identifiers
	- Literals
	- Operators (e.g., `+`, `-`, `*`, `/`, etc.)
	- Punctuation (e.g., `;`, `,`, `()`, `{}`, etc.)
*/

enum {

    DTOKEN_AUTO = 0,
    DTOKEN_VOID,
    DTOKEN_UNSIGNED,
    DTOKEN_SIGNED,
    DTOKEN_BOOL,
    DTOKEN_CHAR,
    DTOKEN_SHORT,
    DTOKEN_LONG,
    DTOKEN_INT,
    DTOKEN_FLOAT,
    DTOKEN_DOUBLE,

    DTOKEN_ENUM,
    DTOKEN_STRUCT,
    DTOKEN_UNION,

    DTOKEN_INLINE,
    DTOKEN_CONST,
    DTOKEN_STATIC,
    DTOKEN_EXTERN,
    DTOKEN_REGISTER,
    DTOKEN_RESTRICT,
    DTOKEN_VOLATILE,

    DTOKEN_FALSE,
    DTOKEN_TRUE,
    DTOKEN_NULLPTR,

    DTOKEN_IF,
    DTOKEN_ELSE,
    EYWORD_WHILE,
    DTOKEN_FOR,
    DTOKEN_DO,
    DTOKEN_SWITCH,
    DTOKEN_GOTO,
    DTOKEN_RETURN,
    
    DTOKEN_CONTINUE,
    DTOKEN_BREAK,
    DTOKEN_CASE,
    DTOKEN_DEFAULT,

    DTOKEN_ALIGNAS,
    DTOKEN_ALIGNOF,
    DTOKEN_SIZEOF,
    DTOKEN_TYPEDEF,
    DTOKEN_TYPEOF,

    DTOKEN_CONSTEXPR,
    DTOKEN_STATIC_ASSERT,
    DTOKEN_THREAD_LOCAL,
    DTOKEN_TYPEOF_UNQUAL,

    DTOKEN_LENGTH
};
typedef int dtoken_id_t;



enum {

    UTOKEN_ARROW = 0,

    UTOKEN_LOR,
    UTOKEN_LAND,
    UTOKEN_EQUALS,
    
    UTOKEN_INCREMENT,
    UTOKEN_DECREMENT,
    UTOKEN_LSHIFT,
    UTOKEN_RSHIFT,

    UTOKEN_NEQUALS,
    UTOKEN_ADD_EQ,
    UTOKEN_SUB_EQ,
    UTOKEN_MUL_EQ,
    UTOKEN_DIV_EQ,
    UTOKEN_MOD_EQ,
    UTOKEN_AND_EQ,
    UTOKEN_XOR_EQ,
    UTOKEN_OR_EQ,
    UTOKEN_LEQUAL,
    UTOKEN_GEQUAL,
    UTOKEN_LSHIFT_EQ,
    UTOKEN_RSHIFT_EQ,

    UTOKEN_COMMENT,
    UTOKEN_CBLOCK_START,
    UTOKEN_CBLOCK_END,

    UTOKEN_L_PARENTH,
    UTOKEN_R_PARENTH,
    UTOKEN_L_BRACKET,
    UTOKEN_R_BRACKET,

    UTOKEN_PERIOD,
    UTOKEN_UNDERSCORE,
    UTOKEN_COMMA,
    UTOKEN_COLON,
    UTOKEN_SEMICOLON,

    UTOKEN_PLUS,
    UTOKEN_MINUS,
    UTOKEN_STAR,
    UTOKEN_FSLASH,
    UTOKEN_PERCENT,
    UTOKEN_AND,
    UTOKEN_XOR,
    UTOKEN_OR,

    UTOKEN_TILDE,
    UTOKEN_EXCLAIM,

    UTOKEN_QUESTION,
    UTOKEN_ASSIGN,
    UTOKEN_BSLASH,

    UTOKEN_LENGTH
};
typedef int utoken_id_t;





enum {
    IS_ALPHA = 1,
    IS_NUMERIC,
};




// undelimited tokens
extern const char *const utokens[UTOKEN_LENGTH];
// delimited tokens
extern const char *const dtokens[DTOKEN_LENGTH];


extern const bool whitespace_map[256];
extern const bool alphanumeric_map[256];



__inline__ bool is_whitespace(char c) {
    return whitespace_map[(unsigned)c];
}

__inline__ bool is_alphanumeric(char c) {
    return alphanumeric_map[(unsigned)c];
}

__inline__ bool is_alpha(char c) {
    return alphanumeric_map[(unsigned)c] == IS_ALPHA;
}

__inline__ bool is_numeric(char c) {
    return alphanumeric_map[(unsigned)c] == IS_NUMERIC;
}





#endif /* #ifndef HYPERC_DESCRIPTORS_H */
