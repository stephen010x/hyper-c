
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "utils/macros.h"
#include "hyperc.h"



#define GFLAG_NOOP (1<<0)
#define GFLAG_TEST (1<<1)


typedef uint64_t global_flags_t;



char helpstr[] = 
    "\n"
    "usage: hyperc -h\n"
    "       hyperc [options] [infile] [outfile]\n"
    "\n"
    "options:\n"
    "    -h        print help options\n"
    "    --test    run tests (only available with debug version)\n"
    "\n";



enum {
    //AFLAG_H = 0,

    BFLAG_TEST = 0,

    FLAGID_INVALID = -1,

    FLAGID_TYPE_MASK = 0xFFFF0000,
    FLAGID_TYPE_A = 0x00010000,
    FLAGID_TYPE_B = 0x00020000,
};
typedef uint32_t flagid_t;



// char flags_a[] = {
//     [AFLAG_H] = "h",
// };

char *flags_b[] = {
    [BFLAG_TEST] = "test",
};



struct {
    char* outfile;
    char* infile;
    global_flags_t flags;
} global = {0};



int test(void);
int parse_args(int argc, char *argv[]);
int parse_flag(flagid_t flagid);
void print_help(void);




int main(int argc, char *argv[]) {
    int err;
    
    err = parse_args(argc, argv);
    if (err)
        return -1;

    if (global.flags & GFLAG_TEST)
#       ifdef __DEBUG__
        test();
#       else
        printf("--test is not avaliable in the release version\n");
#       endif
    
    return 0;
}



// Alright, the arguments for this will be simple
// hyperc [flags] [infile] [outfile]
int parse_args(int argc, char *argv[]) {
    int index = 1;

    // for (int i = 0; i < argc; i++) {
    //     printf("[arg %d]\t%s\n", i, argv[i]);
    // }

    // loop through options
    for (; index < argc; index++) {
        char *flag = argv[index];
        //flagid_t flagid = FLAGID_INVALID;

        // if not a flag then stop looking for flags and break
        if (flag[0] != '-')
            break;

        // check if flag type 'a' or type 'b'
        
        if (flag[1] == '-') {   // type b

            for (int i = 0; (size_t)i < lenof(flags_b); i++) {

                if (strcmp(flags_b[i], flag+2) == 0) {

                    int err = parse_flag(FLAGID_TYPE_B | i);
                    if (err) {
                        printf("'%s' is not a valid flag\n", flag);
                        return err;
                    }
                    
                    break;
                }
            }
            
        } else {                // type a

            for (int i = 1; flag[i] != '\0'; i++) {

                int err = parse_flag(FLAGID_TYPE_A | (flagid_t)(unsigned)flag[i]);
                if (err) {
                    printf("'-%c' is not a valid flag\n", flag[i]);
                    return err;
                }
                
            }
            
        }
    }

    // if noop specified, then don't look for infile and outfile arguments
    if (global.flags & GFLAG_NOOP)
        return 0;

    if (index >= argc) {
        printf("no input file specified\n");
        return -1;
    }

    global.infile = argv[index++];

        if (index >= argc) {
        printf("no output target specified\n");
        return -1;
    }

    global.outfile = argv[index++];
    
    return 0;
}



int parse_flag(flagid_t flagid) {
    switch (flagid) {
        case FLAGID_TYPE_A | (flagid_t)(unsigned)'h':
            global.flags |= GFLAG_NOOP;
            print_help();
            break;
            
        case FLAGID_TYPE_B | BFLAG_TEST:
            global.flags |= GFLAG_TEST;
            break;
            
        default: 
            return -1;
    }
    return 0;
}





void print_help(void) {
    printf(helpstr);
}



// TODO: move this to it's own source file
size_t get_file_size(FILE *file) {
    if (file == NULL) return -1;
    if (fseek(file, 0, SEEK_END) != 0) return -1;
    size_t len = ftell(file);
    rewind(file);
    return len;
}




#ifdef __DEBUG__

int test(void) {
    FILE *infile;
    size_t insize;
    int err;

    printf("running tests\n");

    infile = fopen(global.infile, "r");
    insize = get_file_size(infile);

    char filestr[insize];

    char *ok = (char*)1;
    for (int index = 0; ok;) {
        ok = fgets(&filestr[index], insize, infile);
        if (ok)
            index += strlen(ok);
    }
    
    fclose(infile);


    token_array_t tarray;
    token_array_init(&tarray);
    
    err = tokenize_buffer(&tarray, filestr);

    if (!err)
        print_token_array(&tarray);

    return 0;
}

#endif
