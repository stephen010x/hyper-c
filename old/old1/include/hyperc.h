

//#define MAX_STATEMENT_SIZE  1024
//#define MAX_TOKEN_SIZE            256


#define MAX_TOKENS_PER_STATEMENT  64

#define __inline__ __attribute__((always_inline))
#define _inline __attribute__((always_inline))



#define lenof(__n) (sizeof(__n)/(sizeof((__n)[0])))

#define segfault() (((void (*)(void))NULL)())



#define acharslit_len(...)       sizeof((uint8_t[])__VA_ARGS__)
#define acharslit_to_uint8(...)  (((uint8_t*)(char[])__VA_ARGS__)[0])
#define acharslit_to_uint16(...) (((uint16_t*)(char[])__VA_ARGS__)[0])
#define acharslit_to_uint32(...) (((uint32_t*)(char[])__VA_ARGS__)[0])
#define acharslit_to_uint64(...) (((uint64_t*)(char[])__VA_ARGS__)[0])
#define str_to_uint8(__str) (((uint8_t*)__str)[0])
#define str_to_uint16(__str) (((uint16_t*)__str)[0])
#define str_to_uint32(__str) (((uint32_t*)__str)[0])
#define str_to_uint64(__str) (((uint64_t*)__str)[0])

#define __uint32mask 0xFFFFFFFF
#define __uint64mask 0xFFFFFFFFFFFFFFFF

// this doesnt quite work yet
// also, look into a rolling hash search for optimized string matching, which is 
// especially useful for larger substrings, which isn't really this.
// so for now just use the standard library functions, yeah?
// also look into Locality-sensitive hashing
#define fastmatch_char(__str, ...) ({                                               \
    register bool retval;                                                           \
    register size_t n = acharslit_len(__VA_ARGS__);                                 \
    switch (n) {                                                                    \
        case 1:                                                                     \
            retval = (acharslit_to_uint8(__VA_ARGS__) == str_to_uint8(__str));      \
            break;                                                                  \
        case 2:                                                                     \
            retval = (acharslit_to_uint16(__VA_ARGS__) == str_to_uint16(__str));    \
            break;                                                                  \
        case 3:                                                                     \
        case 4:                                                                     \
            retval = (acharslit_to_uint32(__VA_ARGS__)&(__uint32mask>>((4-n)*8)) == \
                        str_to_uint32(__str)&(__uint32mask>>((4-n)*8)));            \
            break;                                                                  \
        case 5:                                                                     \
        case 6:                                                                     \
        case 7:                                                                     \
        case 8:                                                                     \
            retval = (acharslit_to_uint64(__VA_ARGS__)&(__uint64mask>>((8-n)*8)) == \
                        str_to_uint64(__str)&(__uint64mask>>((8-n)*8)));            \
            break;                                                                  \
        default:                                                                    \
            segfault();                                                             \
    }                                                                               \
    retval;                                                                         \
})




/*
	- Keywords
	- Identifiers
	- Literals
	- Operators (e.g., `+`, `-`, `*`, `/`, etc.)
	- Punctuation (e.g., `;`, `,`, `()`, `{}`, etc.)
*/



enum {

    DTOKEN_AUTO = 1,
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

enum {

    UTOKEN_ARROW = 1,

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


typedef int token_id_t;





enum {
    TOKEN_TYPE_UNKNOWN = 0,
    
    TOKEN_TYPE_SUBSTATEMENT_RAW = 0x40,
    TOKEN_TYPE_SUBSTATEMENT,
    
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_LITERAL,     // ultimately a static literal that consists of N bytes
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_PUNCTUATION,

    TOKEN_TYPE_WHITESPACE,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_DIRECTIVE,

    TOKEN_TYPE_TEMP_UNDELIMITED = 0x80,
    TOKEN_TYPE_TEMP_DELIMITED,
    TOKEN_TYPE_TEMP_UNKNOWN,
};
typedef int token_type_t;



typedef struct {
    token_type_t type;
        
    union {
        token_id_t id;
        
        struct {
            char *string;
            int   slen;
        };
    };
} token_t;


// scopes will be maintained in the stack when parsing, so no allocations for 
// this struct are needed
typedef struct scope_state {
    struct scope_state *parent;
} scope_state_t;



typedef struct {
    //char *buffer;   // instead of allocating a buffer, we just slice the original string
    //int   blen;
    scope_state_t *root_scope;
    scope_state_t *scope;       // current scope

    token_t tokens[MAX_TOKENS_PER_STATEMENT];
} state_t;





// undelimited tokens
extern const char *const utokens[UTOKEN_LENGTH];
// delimited tokens
extern const char *const dtokens[DTOKEN_LENGTH];



//char whitespace[] = {' ', '\t', '\v', '\f', '\n'};

extern const bool whitespace_map[256];
extern const bool alphanumeric_map[256];



__inline bool is_whitespace(char c) {
    return whitespace_map[(int)c];
}

__inline bool is_alphanumeric(char c) {
    return alphanumeric_map[(int)c];
}
