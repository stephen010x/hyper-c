#ifndef HYPERC_LEXER_H
#define HYPERC_LEXER_H



#include <stdlib.h>



enum {
    TOKEN_NULL = 0,
    TOKEN_WHITESPACE,
    TOKEN_NEWLINE,
    TOKEN_COMMENT,
    TOKEN_STRING,
    TOKEN_DIRECTIVE, // Parser needs to awknowledge these exist, but don't need to do much with them.
    TOKEN_DTOKEN,
    TOKEN_UTOKEN,
    TOKEN_IDENTIFIER,
    TOKEN_NUMERIC,
    TOKEN_EOF,

    //TOKEN_TYPE_COUNT,
    
    TOKEN_INVALID = -1,
};
typedef int token_type_t;






typedef struct {
    token_type_t type;
    int tid;
    const char *start;
    size_t len;
    int line;
    int column;
} token_t;






typedef struct {
    token_t *tokens;
    size_t tlen;
    size_t talloc;
} token_array_t;







extern const char *const token_names[];





void next_token(token_t *restrict token, const char *restrict buff, size_t *restrict index);
int tokenize_buffer(token_array_t *restrict array, const char *restrict buff);
void print_token(token_t *token);
void print_token_array(token_array_t *array);




token_t *token_array_newtoken(token_array_t *array);
int token_count_newlines(token_t *token);



__inline__ int token_array_init(token_array_t *array) {
    *array = (token_array_t){
        .tokens = malloc(64*sizeof(token_t)),
        .tlen = 0,
        .talloc = 64,
    };

    return (array->tokens == NULL);
}


__inline__ void token_array_close(token_array_t *array) {
    free(array->tokens);
    *array = (token_array_t){0};
}


__inline__ token_t *token_array_insert(token_array_t *restrict array, token_t *restrict token) {

    token_t *newtoken = token_array_newtoken(array);

    if (newtoken != NULL)
        *newtoken = *token;

    return newtoken;
}


__inline__ char *get_token_name(token_t *token) {
    if (token->type < 0 || token->type > TOKEN_EOF)
        return TOKEN_INVALID_STR;
    return token_names[token->type];
}





#endif /* #ifndef HYPERC_LEXER_H */
