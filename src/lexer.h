#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "toolbox/macros.h"





enum {

    KTOKEN_AUTO = 0,
    KTOKEN_VOID,
    KTOKEN_UNSIGNED,
    KTOKEN_SIGNED,
    KTOKEN_BOOL,
    KTOKEN_CHAR,
    KTOKEN_SHORT,
    KTOKEN_LONG,
    KTOKEN_INT,
    KTOKEN_FLOAT,
    KTOKEN_DOUBLE,

    KTOKEN_ENUM,
    KTOKEN_STRUCT,
    KTOKEN_UNION,

    KTOKEN_INLINE,
    KTOKEN_CONST,
    KTOKEN_STATIC,
    KTOKEN_EXTERN,
    KTOKEN_REGISTER,
    KTOKEN_RESTRICT,
    KTOKEN_VOLATILE,

    KTOKEN_FALSE,
    KTOKEN_TRUE,
    KTOKEN_NULLPTR,

    KTOKEN_IF,
    KTOKEN_ELSE,
    KTOKEN_WHILE,
    KTOKEN_FOR,
    KTOKEN_DO,
    KTOKEN_SWITCH,
    KTOKEN_GOTO,
    KTOKEN_RETURN,
    
    KTOKEN_CONTINUE,
    KTOKEN_BREAK,
    KTOKEN_CASE,
    KTOKEN_DEFAULT,

    KTOKEN_ALIGNAS,
    KTOKEN_ALIGNOF,
    KTOKEN_SIZEOF,
    KTOKEN_TYPEDEF,
    KTOKEN_TYPEOF,

    KTOKEN_CONSTEXPR,
    KTOKEN_STATIC_ASSERT,
    KTOKEN_THREAD_LOCAL,
    KTOKEN_TYPEOF_UNQUAL,

    KTOKEN_ATTRIBUTE,

    KTOKEN_ATOMIC,
    KTOKEN_BITINT,
    KTOKEN_COMPLEX,
    KTOKEN_IMAGINARY,
    KTOKEN_NORETURN,

    KTOKEN_DECIMAL32,
    KTOKEN_DECIMAL64,
    KTOKEN_DECIMAL128,

    KTOKEN_LENGTH,
    KTOKEN_INVALID = (uint8_t)-1
};
typedef uint8_t ktoken_id_t;



enum {

    PTOKEN_VARGS = 0,

    PTOKEN_R_ARROW,

    PTOKEN_LOR,
    PTOKEN_LAND,
    PTOKEN_EQUALS,
    
    PTOKEN_INCREMENT,
    PTOKEN_DECREMENT,
    PTOKEN_LSHIFT,
    PTOKEN_RSHIFT,

    PTOKEN_NEQUALS,
    PTOKEN_ADD_EQ,
    PTOKEN_SUB_EQ,
    PTOKEN_MUL_EQ,
    PTOKEN_DIV_EQ,
    PTOKEN_MOD_EQ,
    PTOKEN_AND_EQ,
    PTOKEN_XOR_EQ,
    PTOKEN_OR_EQ,
    PTOKEN_LEQUAL,
    PTOKEN_GEQUAL,
    PTOKEN_LSHIFT_EQ,
    PTOKEN_RSHIFT_EQ,

    PTOKEN_COMMENT,
    PTOKEN_CBLOCK_START,
    PTOKEN_CBLOCK_END,

    PTOKEN_L_RBRACKET,
    PTOKEN_R_RBRACKET,
    PTOKEN_L_SBRACKET,
    PTOKEN_R_SBRACKET,
    PTOKEN_L_ABRACKET,
    PTOKEN_R_ABRACKET,
    PTOKEN_L_CBRACKET,
    PTOKEN_R_CBRACKET,

    PTOKEN_PERIOD,
    PTOKEN_UNDERSCORE,
    PTOKEN_COMMA,
    PTOKEN_COLON,
    PTOKEN_SEMICOLON,

    PTOKEN_PLUS,
    PTOKEN_MINUS,
    PTOKEN_STAR,
    PTOKEN_FSLASH,
    PTOKEN_PERCENT,
    PTOKEN_AND,
    PTOKEN_XOR,
    PTOKEN_OR,

    PTOKEN_TILDE,
    PTOKEN_EXCLAIM,

    PTOKEN_QUESTION,
    PTOKEN_ASSIGN,
    PTOKEN_BSLASH,

    PTOKEN_LENGTH,
    PTOKEN_INVALID = (uint8_t)-1
};
typedef uint8_t ptoken_id_t;




enum {
    IS_ALPHA = 1,
    IS_NUMERIC,
};





enum {
    TOKEN_NULL = 0,
    
    TOKEN_KEYWORD,
    TOKEN_PUNCTUATOR,
    TOKEN_IDENTIFIER,
    TOKEN_CONSTANT,
    // TOKEN_STRING_LITERAL,
    
    // TOKEN_ERROR = -1,
    // TOKEN_INVALID = -2,
};
typedef uint8_t token_type_t;


enum {
    CONST_NULL = 0,
    
    CONST_FLOATING,
    CONST_INTEGER,
    // CONST_CHARACTER,
    CONST_STRING,
    CONST_PREDEFINED,
    // TOKEN_STRING_LITERAL,
    
    CONST_INVALID = -1,
};
typedef uint8_t token_const_type_t;




// NOTE: DEFAULT FLAGS MUST ALWAYS BE ZERO!!!
enum {
    TFLAG_DEFAULT = 0,

    // for strings/character encoding
    TFLAG_ENCC_MASK    = 0b111<<0,
    TFLAG_ENCC_DEFAULT = 0b000<<0,
    TFLAG_ENCC_UTF8    = 0b001<<0,
    TFLAG_ENCC_UTF16   = 0b010<<0,
    TFLAG_ENCC_UTF32   = 0b011<<0,
    TFLAG_ENCC_WIDE    = 0b100<<0,

    TFLAG_STRING_MASK = 0b1<<3,
    TFLAG_STRING_S    = 0b0<<3,
    TFLAG_STRING_C    = 0b1<<3,

    // for floats/integers
    TFLAG_BASE_MASK = 0b11<<0,
    TFLAG_BASE_DEC  = 0b00<<0,
    TFLAG_BASE_HEX  = 0b01<<0,
    TFLAG_BASE_OCT  = 0b10<<0,
    TFLAG_BASE_BIN  = 0b11<<0,

    // for floats
    TFLAG_FSUFFIX_MASK    = 0b1111<<8,
    TFLAG_FSUFFIX_DEFAULT = 0b0000<<8,
    TFLAG_FSUFFIX_DOUBLE  = 0b0000<<8,
    TFLAG_FSUFFIX_FLOAT   = 0b0001<<8,
    TFLAG_FSUFFIX_LONG    = 0b0010<<8,
    TFLAG_FSUFFIX_DEC32   = 0b0011<<8,
    TFLAG_FSUFFIX_DEC64   = 0b0100<<8,
    TFLAG_FSUFFIX_DEC128  = 0b0101<<8,

    // for integers (indicates smallest type)
    TFLAG_ISUFFIX_MASK    = 0b111<<8,
    TFLAG_ISUFFIX_DEFAULT = 0b000<<8,
    TFLAG_ISUFFIX_INT     = 0b000<<8,
    TFLAG_ISUFFIX_LONG    = 0b001<<8,
    TFLAG_ISUFFIX_LLONG   = 0b010<<8,
    TFLAG_ISUFFIX_WB      = 0b011<<8,

    TFLAG_IUNSIGN_MASK    = 0b1<<11,
    TFLAG_IUNSIGN_DEFAULT = 0b0<<11,
    TFLAG_IUNSIGN_FALSE   = 0b0<<11,
    TFLAG_IUNSIGN_TRUE    = 0b1<<11,
    
    // TFLAG_ISUFFIX_MASK    = 0b1111<<8,
    // TFLAG_ISUFFIX_DEFAULT = 0b0000<<8,
    // TFLAG_ISUFFIX_INT     = 0b0001<<8,
    // TFLAG_ISUFFIX_UINT    = 0b0010<<8,
    // TFLAG_ISUFFIX_LONG    = 0b0011<<8,
    // TFLAG_ISUFFIX_ULONG   = 0b0100<<8,
    // TFLAG_ISUFFIX_LLONG   = 0b0101<<8,
    // TFLAG_ISUFFIX_ULLONG  = 0b0110<<8,
    // TFLAG_ISUFFIX_WB      = 0b0111<<8,
    // TFLAG_ISUFFIX_UWB     = 0b1000<<8,
};
typedef uint16_t token_flags_t;




typedef struct {
    const char *start;
    size_t len;
    int line;
    int column;
    union {
        struct {
            uint8_t type;
            uint8_t tid;
        };
        uint16_t ttid;
    };
    uint16_t flags;
} token_t;





typedef struct {
    const char *buff;
    size_t index;
    int line_index;
    // int line, line_index;
    int line, column;
    const char *filename;
    int filenamelen;
    int flags;
} clexstate_t;




// typedef struct {
//     token_t *tokens;
//     size_t tlen;
//     size_t talloc;
//     char *data;     // wat this for?
// } tokenlist_t;






extern const bool    whitespace_map[256];
extern const bool    source_map[256];
extern const bool    punctuator_map[256];
extern const uint8_t alphanumeric_map[256];




__inline__ bool is_whitespace(char c) {
    return whitespace_map[(int)(unsigned char)c];
}

__inline__ bool is_alphanumeric(char c) {
    return !!alphanumeric_map[(int)(unsigned char)c];
}

__inline__ bool is_alpha(char c) {
    return alphanumeric_map[(int)(unsigned char)c] == IS_ALPHA;
}

__inline__ bool is_numeric(char c) {
    return alphanumeric_map[(int)(unsigned char)c] == IS_NUMERIC;
}

__inline__ bool is_punctuator(char c) {
    return source_map[(int)(unsigned char)c];
}

__inline__ bool is_source(char c) {
    return source_map[(int)(unsigned char)c];
}



__inline__ bool is_uppercase(unsigned char c) {
    return c-'A' <= 'Z'-'A';
}

__inline__ bool is_lowercase(unsigned char c) {
    return c-'a' <= 'z'-'a';
}

__inline__ char to_uppercase(char c) {
    if (is_lowercase(c)) return c-'a'+'A';
    return c;
}

__inline__ char to_lowercase(char c) {
    if (is_uppercase(c)) return c-'A'+'a';
    return c;
}







// __inline__ int tokenlist_init(tokenlist_t *array, char* data) {
//     *array = (tokenlist_t){
//         .tokens = malloc(64*sizeof(token_t)),
//         .tlen = 0,
//         .talloc = 64,
//         .data = data,
//     };
// 
//     return (array->tokens == NULL);
// }


// __inline__ void tokenlist_close(tokenlist_t *array) {
//     free(array->tokens);
//     free(array->data);
//     DEBUG( *array = (tokenlist_t){0}; );
// }


// __inline__ token_t *tokenlist_insert(tokenlist_t *restrict array, token_t *restrict token) {
// 
//     token_t *newtoken = tokenlist_newtoken(array);
// 
//     if (newtoken != NULL)
//         *newtoken = *token;
// 
//     return newtoken;
// }


// __inline__ const char *get_token_name(token_t *token) {
//     if (token->type < 0 || token->type > TOKEN_EOF)
//         return token_invalid_str;
//     return token_names[token->type];
// }






// __inline__ void clexer_init(clexstate_t *restrict state, const char *restrict buff) {
//     *state = (clexstate_t){
//         .buff = buff,
//         .index = 0,
//         .line = 0,
//         .line_index = 0,
//         .column = 0,
//     };
// }
// 
// 
// 
// 
// __inline__ size_t reader_next(clexstate_t *state, int n) {
//     int i;
//     for (i = 0; state->buff[state->index+i] && (i < n); i++) {
//         if (state->buff[state->index] == '\n') {
//             state->line++;
//             state->line_index = state->index;
//             state->column = 0;
//         } else {
//             state->column++;
//         }
//     }
//     // for (int i = 0; i > n; i--) {
//     //     if (state->buff[state->index[0]] == '\n') {
//     //         state->line--;
//     //         state->column = -1;
//     //     } else {
//     //         state->column--;
//     //     }
//     // }
//     return (state->index += i);
// }




TODO
// TODO: fix the column number for tokens





ktoken_id_t match_keyword(const char *str, int len);
ptoken_id_t match_punctuator(const char *restrict str, int *restrict len);
void next_token(clexstate_t *restrict state, token_t *restrict token);
void print_token(const token_t *token, int i);
// void print_tokenlist(tokenlist_t *array);
void clexer_init(clexstate_t *restrict state, const char *restrict buff);




#endif  /* #ifndef LEXER_H */
