#ifdef __DEBUG__


#include <stdint.h>
#include <stdio.h>
#include <string.h>

// TODO: for faster compilation, consider using individual headers
// rather than all of them.
// Maybe even just delete this main header. It isn't like this is a library.
//#include "hyperc.h"

#include "hyperc/lexer.h"
#include "hyperc/parser.h"
#include "hyperc/match/lexer.h"
#include "hyperc/match/parser.h"
#include "hyperc.h"
#include "utils/debug.h"



extern global_t global;



int lexer_tests(void);
int parser_tests(void);



token_t *test_tget_handler(token_array_t *tarray, int index);
bool test_tmatch_handler(token_t *token, uint32_t mid);
bool test_eof_handler(token_array_t *tarray, int index);
//char *test_ptoken_name_handler(ptoken_t *ptoken, char *buff, int max);
char *test_ptoken_name_handler(ptoken_t *ptoken);











int run_tests(void) {
    //assert(  lexer_tests() == 0, -1 );
    assert( parser_tests() == 0, -2 );
    return 0;
}






// TODO: move this to it's own source file
//size_t get_file_size(FILE *file) {
size_t get_file_size(char *fname) {
    FILE *file;
    size_t len;
    
    file = fopen(fname, "r");

    if (file == NULL) return -1;
    // if (fseek(file, 0, SEEK_END) != 0) return -1;
    // len = ftell(file);
    //rewind(file);

    fseek(file, 0, SEEK_END);
    len = ftell(file);

    fclose(file);

    return len;
}




int load_file(char *buff, char *fname, size_t bytes) {
    FILE *infile;
    //char *ok;
    //size_t insize;

    infile = fopen(fname, "r");
    if (infile == NULL) return -1;
    //insize = get_file_size(infile);

    //char filestr[insize];

    // ok = (char*)1;
    // for (int index = 0; ok;) {
    //     ok = fgets(&buff[index], bytes-index, infile);
    //     if (ok)
    //         index += strlen(ok);
    // }

    fseek(infile, 0, SEEK_SET);
    bytes = fread(buff, 1, bytes, infile);

    buff[bytes] = '\0';    
    
    fclose(infile);

    return 0;
}






int lexer_tokenize_file(char *fname, token_array_t *tarray) {
    int err;
    size_t insize;
    char *filestr;
    //token_array_t tarray;

    insize = get_file_size(fname);
    //char filestr[insize+1];
    filestr = malloc(insize+1);

    load_file(filestr, fname, insize);

    token_array_init(tarray, filestr);
    err = tokenize_buffer(tarray, filestr);

    return err;
}







int lexer_tests(void) {
    int err;
    token_array_t tarray;

    err = lexer_tokenize_file(global.infile, &tarray);

    if (!err)
        print_token_array(&tarray);

    token_array_close(&tarray);
    return err;
    
        
//     int err;
//     size_t insize;
//     token_array_t tarray;
//     
//     printf("running lexer tests\n");
// 
//     insize = get_file_size(global.infile);
//     char filestr[insize];
//     
//     load_file(filestr, global.infile, insize);
// 
//     token_array_init(&tarray);
//     
//     err = tokenize_buffer(&tarray, filestr);
// 
//     if (!err)
//         print_token_array(&tarray);
// 
//     token_array_close(&tarray)
// 
//     return 0;
}




// ptoken_t *parse_tokens(token_array_t *tarray) {
// 
// }





int parser_tests(void) {
    //int err;
    //size_t insize;
    ptoken_t *ptoken;
    token_array_t tarray;
    match_t m;
    
    printf("running parser tests\n");

    lexer_tokenize_file(global.infile, &tarray);

    m = (match_t){
        .tget_handler = (tget_handler_t)test_tget_handler,
        .tmatch_handler = (tmatch_handler_t)test_tmatch_handler,
        .eof_handler = (eof_handler_t)test_eof_handler,
        .tree = c_mtree,
    };

    ptoken = match(&m, MATCH_TRANSLATION_UNIT, &tarray);

    print_ptoken(ptoken, test_ptoken_name_handler);

    ptoken_free(ptoken);
    token_array_close(&tarray);

    return 0;
}






token_t *test_tget_handler(token_array_t *tarray, int index) {
    return &tarray->tokens[index];
}




bool test_tmatch_handler(token_t *token, uint32_t mid) {
    uint32_t flags;

    //printf("<<MATCH %d %d>>\n", token->tid, mid);

    flags = mid;
    mid &= MF_MATCH_MASK;

    // if utoken
    if (flags & MF_UTOKEN) {
        if ((mid == (uint32_t)token->tid) && (token->type == TOKEN_UTOKEN))
            return true;

    // if dtoken
    } else if (flags & MF_DTOKEN) {
        if ((mid  == (uint32_t)token->tid) && (token->type == TOKEN_DTOKEN))
            return true;

    // if token_type
    } else if (flags & MF_TOKEN_TYPE) {
        if (mid == (uint32_t)token->type)
            return true;
    }

    return false;
}



bool test_eof_handler(token_array_t *tarray, int index) {
    return (tarray->tokens[index].type == TOKEN_EOF || (size_t)index >= tarray->tlen);
}


// TODO: finish writing
//char *test_ptoken_name_handler(ptoken_t *ptoken, char *buff, int max) {
char *test_ptoken_name_handler(ptoken_t *ptoken) {
    
    switch (ptoken->type) {

        case PFLAG_USER:
            print_token(ptoken->user.token);
            break;

        case PFLAG_MATCH:
            printf("%s %s (%d:%d)",
                    match_rule_str[ptoken->match.targetid], 
                    match_type_str[ptoken->match.ruleid],
                    ptoken->match.targetid,
                    ptoken->match.ruleid
                );
            break;

        default:
            printf("unknown type");
    }
    
    return NULL;
}



#endif /* #ifdef __DEBUG__ */
