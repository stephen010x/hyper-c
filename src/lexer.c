
// TODO: Convert this to a match-tree


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "utils/macros.h"
#include "utils/debug.h"
//#include "hyperc/descriptors.h"
#include "hyperc/lexer.h"
#include "hyperc/match/lexer.h"




const char *const token_names[] = {
    [TOKEN_NULL]        = "TOKEN_NULL     ",
    [TOKEN_WHITESPACE]  = "TOKEN_WHITESPAC",
    [TOKEN_NEWLINE]     = "TOKEN_NEWLINE  ",
    [TOKEN_COMMENT]     = "TOKEN_COMMENT  ",
    [TOKEN_STRING]      = "TOKEN_STRING   ",
    [TOKEN_DIRECTIVE]   = "TOKEN_DIRECTIVE",
    [TOKEN_DTOKEN]      = "TOKEN_DTOKEN   ",
    [TOKEN_UTOKEN]      = "TOKEN_UTOKEN   ",
    [TOKEN_IDENTIFIER]  = "TOKEN_IDENTIFIE",
    [TOKEN_NUMERIC]     = "TOKEN_NUMERIC  ",
    [TOKEN_EOF]         = "TOKEN_EOF      ",
};

const char token_invalid_str[] = "TOKEN_INVALID  ";
#define TOKEN_INVALID_STR token_invalid_str





// the golden ratio
#define GOLDEN_ALLOC_MULT 1.61803






#define STR_RED(__str) ANSI_RED __str ANSI_DEFAULT


// format must be a string literal
#define lexer_error(__format, ...) do {                                         \
    printf(STR_RED("[lexer error]") "\t" __format __VA_OPT__(,) __VA_ARGS__);   \
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
    for (index = 0; is_whitespace(buff[index]); index++);

    return index;
}




// returns length. If zero then no alphanumerics
__inline__ size_t prod_alphanumeric(const char *buff) {
    size_t index;
    // extract alphanumeric
    for (index = 0; is_alphanumeric(buff[index]); index++);
    
    return index;
}




// returns length. If zero then no comment.
__inline__ size_t prod_comment(const char *buff) {
    size_t index;

    // determine if first two characters are '//'
    if (*(uint16_t*)buff == (uint16_t)'//') {
        // loop until end of line or EOF
        for (index = 2; buff[index] != '\n' ; index++);
        return index;
    }

    // determine if first two characters are '/ *'
    // I think multibyte string comparisons have to be byte swapped
    if (*(uint16_t*)buff == (uint16_t)'*/') {
        // loop until '* /' found
        for (index = 2; *(uint16_t*)(buff+index) == (uint16_t)'/*'; index++);
        return index+1; // increment by one to include the final '/' character
    }

    return 0;
}



// could potentially crash or cause issues when used on the first two characters
// of a file
// this is only for detecting if a quotation token is escaped, not for any other escape sequence
__inline__ bool is_escaped(const char *buff) {
    // count number of backwards slashes
    // if even then not escaped
    // if odd then escaped

    int i;

    // loop until no more escape characters found
    for (i = -1; buff[i] == '\\'; i--);

    // printf("[%d:%d]\t%.10s\n", (-i)%2, i, buff+i);
    // fflush(stdout);

    return !(i % 2);
}





// returns length. If zero then no string. Includes the quotation marks.
__inline__ size_t prod_string(const char *buff) {
    size_t index;

    // determine if first character is (')
    if (buff[0] == '\'') {
        // loop until another (') that isn't following an escape character
        for (index = 1; buff[index] != '\'' || is_escaped(&buff[index]); index++);
        return index+1;
    }

    // determine if first character is (")
    if (buff[0] == '"') {
        // loop until another (") that isn't following an escape character
        for (index = 1; buff[index] != '"' || is_escaped(&buff[index]); index++);
        return index+1;
    }

    return 0;
}





// returns length. If zero then no directive.
__inline__ size_t prod_directive(const char *buff) {
    size_t index;

    // TODO: finish this
    // make sure if it is preceeded by whitespace with at least one newline
    // prod_directive needs to be called before prod_whitespace as a result
    //index = prod_whitespace(buff);
    //if 
    index = 0;

    // determine if first character is '#'
    if (buff[0] == '#') {
        // loop until end of line that isn't following an escape character or EOF
        for (index += 1; (buff[index] != '\n' || buff[index-1] == '\\') && buff[index] != '\0'; index++);
        // include newline
        return index+1;
    }

    return 0;
}





// returns dtoken index. If -1 then no match
__inline__ size_t match_dtoken(const char *buff, size_t length) {
    // loop through dtokens until a match is found
    int index;
    for (index = 0; index < DTOKEN_LENGTH && strncmp(buff, dtokens[index], length); index++);
    
    //return index % DTOKEN_LENGTH;   // will ensure that if no match, then returns zero
    return (index == DTOKEN_LENGTH) ? -1 : index;
}




// returns utoken index. If -1 then no match
__inline__ size_t match_utoken(const char *buff) {
    // loop through utokens until a match is found
    int index;
    for (index = 0; index < UTOKEN_LENGTH && strncmp(buff, utokens[index], strlen(utokens[index])); index++);
    
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
    size_t len;
    const char *nbuff = buff + *index;
    token_type_t type = TOKEN_NULL;
    int tid = 0x80000000;        // increases likelyhood of segfault if misused


    if (nbuff[0] == '\0') {                             // End Of File check
        type = TOKEN_EOF;
        len = 0;

    // } else if (nbuff[0] == '\n') {                      // newline check
    //     type = TOKEN_NEWLINE;
    //     len = 1;

    // check directive before whitespace
    } else if ((len = prod_directive(nbuff)) != 0) {    // directive check
        type = TOKEN_DIRECTIVE; 
        
    } else if ((len = prod_whitespace(nbuff)) != 0) {   // whitespace check
        type = TOKEN_WHITESPACE;

    } else if ((len = prod_comment(nbuff)) != 0) {      // comment check
        type = TOKEN_COMMENT;

    } else if ((len = prod_string(nbuff)) != 0) {       // string check
        type = TOKEN_STRING;
        
    } else if ((len = prod_alphanumeric(nbuff)) != 0) { // alphanumeric check

        if (is_numeric(nbuff[0])) {                                 // numeric check
            type = TOKEN_NUMERIC;
            
        } else if ((tid = match_dtoken(nbuff, len)) != -1) {     // dtoken check
            type = TOKEN_DTOKEN;

        } else {                                                    // otherwise identifier
            type = TOKEN_IDENTIFIER;
        }
        
    } else if ((tid = match_utoken(nbuff)) != -1) {  // utoken check
        type = TOKEN_UTOKEN;
        len = strlen(utokens[tid]);
        
    } else {                                            // otherwise invalid token
        // let the higher level handle error statements
        // assume invalid token is a single character
        //lexer_error("invalid token \"%s\"", line, column, nbuff[0], nbuff[0]);

        type = TOKEN_INVALID;
        len = 1;
    }

    *index += len;

    token->type  = type;
    token->tid   = tid;
    token->start = nbuff;
    token->len   = len;

    //printf("\n####################################################################\n");
    //printf("####################################################################\n\n");

    // if (token->type != TOKEN_WHITESPACE) {
    //     print_token(token);
    //     fflush(stdout);
    // }

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








token_t *token_array_newtoken(token_array_t *array) {

    if (array->tlen >= array->talloc) {
    
        array->talloc *= GOLDEN_ALLOC_MULT;
        array->tokens = realloc(array->tokens, array->talloc * sizeof(token_t));

        if (array->tokens == NULL)
            return NULL;
    }    
    return array->tokens + array->tlen++;
}




int token_count_newlines(token_t *token) {
    int count = 0;

    for (size_t i = 0; i < token->len; i++)
        if (token->start[i] == '\n')
            count++;

    return count;
}





// TODO: Make this incremental
int tokenize_buffer(token_array_t *restrict array, const char *restrict buff) {
    token_t token;
    int line = 1;
    int column = 1;
    size_t index = 0;

    for (;;) {

        // get next token
        next_token(&token, buff, &index);

        // set token line and column number
        token.line = line;
        token.column = column;

        switch (token.type) {

            // effectively ignore these
            case TOKEN_WHITESPACE:
            case TOKEN_COMMENT:
                break;
            
            case TOKEN_STRING:
            case TOKEN_DIRECTIVE:
            case TOKEN_DTOKEN:
            case TOKEN_UTOKEN:
            case TOKEN_IDENTIFIER:
            case TOKEN_NUMERIC:
            case TOKEN_EOF:

                // insert token into array
                void *ok = token_array_insert(array, &token);
                if (!ok)
                    return -2;

                if (token.type == TOKEN_EOF)
                    return 0;
                    
                break;

            case TOKEN_NULL:
            case TOKEN_INVALID:
            default:

                lexer_error("invalid token %d:%d: \"%c\" (0x%02x)\n",
                    line, column, token.start[0], token.start[0]);
                return -1;
        }

        // print_token(&token);
        // printf("\n");

        column += token.len;

        int newlines = token_count_newlines(&token);
        if (newlines > 0) {
            line += newlines;
            column = 1;
        }


    }

    return 0;
}





void print_token_array(token_array_t *array) {
    for (size_t i = 0; i < array->tlen; i++) {
        print_token(&array->tokens[i]);
        printf("\n");
    }
}



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
char *special_chars[256] = {
    [0x00 ... 0x1F] = (char*)-1,
    ['\a'] = "\\a",
    ['\b'] = "\\b",
    ['\f'] = "\\f",
    ['\n'] = "\\n",
    ['\r'] = "\\r",
    ['\t'] = "\\t",
    ['\v'] = "\\v",
    [0x7F] = (char*)-1,
};
#pragma GCC diagnostic pop



// printf with escape characters
void printf_wesc(const char *format, ...) {
    //char *str;
    size_t strlen;

    va_list vargs, vargs2;
    va_start(vargs, format);
    va_copy(vargs2, vargs);
     
    strlen = vsnprintf(NULL, 0, format, vargs);
    va_end(vargs);

    if (strlen == (size_t)-1) {
        printf("\n");
        va_end(vargs2);
        vassert(("string error", false));
    }

    char str[strlen+1];
    //char *str = malloc(strlen+1);

    vsnprintf(str, strlen+1, format, vargs2);
    va_end(vargs2);

    for (size_t i = 0; i < strlen; i++) {
        char c = str[i];
        int ci = (int)(unsigned char)c;

        if (special_chars[ci] == NULL)
            printf("%c", c);
            
        else if (special_chars[ci] == (char*)-1)
            printf("\\0x%02x", ci);

        else {
            printf("\\0x%x", ci);
            fflush(stdout);
            printf(special_chars[ci]);
        }
    }

    //free(str);
}



void print_token(token_t *token) {
    char str[token->len+1];
    const char *ttypestr;
    // char *format;

    // if (false && token->type == TOKEN_WHITESPACE) {
    //     //str[0] = '\0';
    //     return;
    // } else {
    //     strncpy(str, token->start, token->len);
    //     str[token->len] = '\0';
    // }

    strncpy(str, token->start, token->len);
    str[token->len] = '\0';

    ttypestr = (token->type == TOKEN_INVALID) ? TOKEN_INVALID_STR : token_names[token->type];

    // printf("(\"%s\")\n", str);

    switch (token->type) {
        case TOKEN_DTOKEN:
        case TOKEN_UTOKEN:
            printf("line %03d:%03d [%s\b\b%02d]:\t", token->line, token->column, ttypestr, token->tid);
            break;
        default:
            printf("line %03d:%03d [%s]:\t", token->line, token->column, ttypestr);
            break;
    }
    
    
    printf_wesc("%s", str); // passes str as argument rather than format so as to not confuse format strings
    //printf("\n");
}
