#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"


int main(int argc, char **argv) {
    FILE *file;
    char *buff;
    size_t len;
    clexstate_t state;
    token_t token;

    // check args
    if (argc < 2) return 1;

    // open file
    file = fopen(argv[1], "rb");
    if (!file) return 1;

    // seek file length
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    rewind(file);

    // allocate memory
    buff = malloc(len + 32);
    if (!buff) return 1;

    // read file into buffer
    fread(buff+16, 1, len, file);
    buff[len+16] = '\0';

    // close file
    fclose(file);

    // init lexer reader
    clexer_init(&state, buff+16);

    for (int i = 0; state.buff[state.index]; i++) {
        next_token(&state, &token);
        print_token(&token, i);
        // printf("%d:%d type=%u tid=%u len=%zu \"%.*s\"\n",
        //     token.line, token.column,
        //     token.type, token.tid,
        //     token.len, (int)token.len, token.start);
    }

    free(buff);
    return 0;
}
