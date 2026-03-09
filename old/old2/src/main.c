
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//#include "utils/macros.h"
#include "utils/macros.h"
#include "hyperc/test.h"
#include "hyperc.h"





char helpstr[] = 
    "\n"
    "usage: hyperc -h\n"
    "       hyperc [options] [infile] [outfile]\n"
    "\n"
    "options:\n"
    "    -h        print help options\n"
    "    --test    run tests (only available with debug version)\n"
    "\n";




// char flags_a[] = {
//     [AFLAG_H] = "h",
// };

char *flags_b[] = {
    [BFLAG_TEST] = "test",
};





global_t global = {0};



//int test(void);
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
        run_tests();
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





