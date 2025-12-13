
// I think here we will simply just tokenize the string


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "hyperc/macros.h"
#include "hyperc/debug.h"
#include "hyperc/lexer.h"




enum {
    TOKEN_NULL = 0,
    TOKEN_WHITESPACE,
    TOKEN_NEWLINE,
    TOKEN_COMMENT,
    TOKEN_STRING,
    TOKEN_DIRECTIVE, // Parser needs to awknowledge these exist, but don't need to do anything with them.
    TOKEN_DTOKEN,
    TOKEN_UTOKEN,
    TOKEN_IDENTIFIER,
    TOKEN_NUMERIC,
    TOKEN_EOF,
    TOKEN_INVALID = -1,
};
typeof int token_type_t;



char *token_names[] = {
    [TOKEN_NULL]        = #TOKEN_NULL,
    [TOKEN_WHITESPACE]  = #TOKEN_WHITESPACE,
    [TOKEN_NEWLINE]     = #TOKEN_NEWLINE,
    [TOKEN_COMMENT]     = #TOKEN_COMMENT,
    [TOKEN_STRING]      = #TOKEN_STRING,
    [TOKEN_DIRECTIVE]   = #TOKEN_DIRECTIVE,
    [TOKEN_DTOKEN]      = #TOKEN_DTOKEN,
    [TOKEN_UTOKEN]      = #TOKEN_UTOKEN,
    [TOKEN_IDENTIFIER]  = #TOKEN_IDENTIFIER,
    [TOKEN_NUMERIC]     = #TOKEN_NUMERIC,
    [TOKEN_EOF]         = #TOKEN_EOF,
}



typedef struct {
    token_type_t type;
    int tindex;
    char *start;
    size_t len;
    int line;
    int column;
} token_t;




// the golden ratio
#define GOLDEN_ALLOC_MULT 1.61803


typedef struct {
    token_t *tokens;
    size_t tlen;
    size_t talloc;
} token_array_t;





#define STR_RED(__str) ANSI_RED __str ANSI_DEFAULT


// format must be a string literal
#define lexer_error(__format, ...) do {                                         \
    printf(STR_RED("[lexer error]") "\t" __format, __VA_OPT__(,) __VA_ARGS__)   \
} while(0)






// __inline__ int extract_whitespace(token_t *restrict token, const char *restrict buff, size_t *restrict index) {
//     // check to see if first character is whitespace
//     if (!is_whitespace(buff[*index]))
//         return 1;
// 
//     *token = {
//         .type = TOKEN_WHITESPACE,
//         .start = buff + index,
//         .len = 0,
//     }
// 
//     // extract rest of whitespace
//     while (is_whitespace(buff[*index + token->len]))
//         token->len++;
// 
//     return 0;
// }


// returns length. If zero then no whitespace
__inline__ size_t prod_whitespace(const char *buff) {
    size_t index;
    // extract whitespace
    while (is_whitespace(buff[index]))
        index++;

    return index;
}




// returns length. If zero then no alphanumerics
__inline__ size_t prod_alphanumeric(const char *buff) {
    size_t index;
    // extract alphanumeric
    while (is_alphanumeric(buff[index]))
        index++;
    
    return index;
}




// returns length. If zero then no comment.
__inline__ size_t prod_comment(const char *buff) {
    size_t index;

    // determine if first two characters are '//'
    if (*(uint16_t*)buff == (uint16_t)'//') {
        // loop until end of line
        for (index = 2; buff[index] != '\n', index++);
        return index;
    }

    // determine if first two characters are '/ *'
    // I think multibyte string comparisons have to be byte swapped
    if (*(uint16_t*)buff == (uint16_t)'*/') {
        // loop until '* /' found
        for (index = 2; *(uint16_t*)(buff+index) == (uint16_t)'/*', index++);
        return index+1; // increment by one to include the final '/' character
    }

    return 0;
}





// returns length. If zero then no string. Includes the quotation marks.
__inline__ size_t prod_string(const char *buff) {
    size_t index;

    // determine if first character is (')
    if (buff[0] == '\'') {
        // loop until another (') that isn't following an escape character
        for (index = 1; buff[index] != '\'' || buff[index-1] == '\\', index++);
        return index;
    }

    // determine if first character is (")
    if (buff[0] == '"') {
        // loop until another (") that isn't following an escape character
        for (index = 1; buff[index] != '"' || buff[index-1] == '\\', index++);
        return index;
    }

    return 0;
}





// returns length. If zero then no directive.
__inline__ size_t prod_directive(const char *buff) {
    size_t index;

    // determine if first character is '#'
    if (buff[0] == '#') {
        // loop until end of line that isn't following an escape character
        for (index = 1; buff[index] != '\'' || buff[index-1] == '\\', index++);
        return index;
    }

    return 0;
}





// returns dtoken index. If -1 then no match
__inline__ size_t match_dtoken(const char *buff, size_t length) {
    // loop through dtokens until a match is found
    int index;
    for (index = 0; i < DTOKEN_LENGTH && strncmp(buff, dtokens[index], length); index++);
    
    //return index % DTOKEN_LENGTH;   // will ensure that if no match, then returns zero
    return (index == DTOKEN_LENGTH) ? -1 : index;
}




// returns utoken index. If -1 then no match
__inline__ size_t match_utoken(const char *buff) {
    // loop through utokens until a match is found
    int index;
    for (index = 0; i < UTOKEN_LENGTH && strcmp(buff, utokens[index]); index++);
    
    return (index == UTOKEN_LENGTH) ? -1 : index;
}




// __inline__ size_t find_previous_newline(const char *restrict buff, size_t *restrict index) {
//     // loop until previous newline is found or start of buffer
//     int len;
//     for (len = 1; (len <= index) || (buff[index-len] != '\n'); len++)
//     return len;
// }




// TODO: add a way to add new conditional checks via callback that will check,
// and then return a bool, a type, and length.
// int line, int column
void next_token(token_t *restrict token, const char *restrict buff, size_t *restrict index) {
    size_t len = 0;
    char *nbuff = buff + *index;
    token_type_t type = TOKEN_NULL;
    int tindex = 0x80000000;        // increases likelyhood of segfault if misused


    if (nbuff[0] == '\0') {                             // End Of File check
        type = TOKEN_EOF;

    } else if ((len = prod_whitespace(nbuff)) != 0) {   // whitespace check
        type = TOKEN_WHITESPACE;

    } else if (nbuff[0] == '\n') {                      // newline check
        type = TOKEN_COMMENT;

    } else if ((len = prod_comment(nbuff)) != 0) {      // comment check
        type = TOKEN_COMMENT;

    } else if ((len = prod_string(nbuff)) != 0) {       // string check
        type = TOKEN_STRING;
        
    } else if ((len = prod_directive(nbuff)) != 0) {    // directive check
        type = TOKEN_DIRECTIVE;
        
    } else if ((len = prod_alphanumeric(nbuff)) != 0) { // alphanumeric check

        if (is_numeric(nbuff[0])) {                                 // numeric check
            type = TOKEN_NUMERIC;
            
        } else if ((tindex = match_dtoken(nbuff, len)) != -1) {     // dtoken check
            type = TOKEN_DTOKEN;

        } else {                                                    // otherwise identifier
            type = TOKEN_IDENTIFIER;
        }
        
    } else if ((tindex = match_utoken(nbuff)) != -1) {  // utoken check
        type = TOKEN_UTOKEN;
        
    } else {                                            // otherwise invalid token
        // let the higher level handle error statements
        // assume invalid token is a single character
        //lexer_error("invalid token \"%s\"", line, column, nbuff[0], nbuff[0]);

        type = TOKEN_INVALID;
        len = 1;
    }

    *index += len;

    token->type  = type;
    token->start = nbuff;
    token->len   = len;

//     token_t token = {
//         .type = type,
//         .start = nbuff,
//         .len = len,
//         .line = line,
//         .column = column,
//     };
// 
//     return token;
}







__inline__ int token_array_init(token_array_t *array) {
    *array = (token_array_t){
        .tokens = malloc(64*sizeof(token_t)),
        .tlen = 0,
        .talloc = 64,
    };

    return (array->tokens == NULL)
}

__inline__ void token_array_close(token_array_t *array) {
    free(array->tokens)
    *array = {0};
}




token_t *token_array_newtoken(token_array_t *array) {

    if (array->tlen >= array->talloc) {
    
        array->talloc *= GOLDEN_ALLOC_MULT
        array->tokens = realloc(array->tokens, array->talloc * sizeof(token_t));

        if (array->tokens == NULL)
            return NULL;
    }    
    return array->tokens + array->tlen++;
}




__inline__ token_t *token_array_insert(token_array_t *restrict array, token_t *restrict token) {

    token_t *newtoken = token_array_newtoken(array);

    if (newtoken != NULL)
        *newtoken = *token;

    return newtoken;
}






int tokenize_buffer(token_array_t *restrict array, const char *restrict buff) {
    token_t token;
    int line = 1;
    int column = 1;
    size_t index = 0;

    for (;;) {

        next_token(&token, buff, &index);

        switch {

            // effectively ignore these
            case TOKEN_WHITESPACE:;
            case TOKEN_COMMENT:
                break;
            
            case TOKEN_NEWLINE:
                break;
            
            case TOKEN_STRING:
            case TOKEN_DIRECTIVE:
            case TOKEN_DTOKEN:
            case TOKEN_UTOKEN:
            case TOKEN_IDENTIFIER:
            case TOKEN_NUMERIC: 
                break;
            
            case TOKEN_EOF: 
                break;

            case TOKEN_NULL:
            case TOKEN_INVALID:
            default:

                lexer_error("invalid token %s:%s:\"%c\" (0x%02x)",
                    line, column, token.start[0], token.start[0]);
                return -1;
        }

    }

    return 0;
}
