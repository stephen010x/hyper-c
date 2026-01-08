#ifndef HYPERC_H
#define HYPERC_H


//#include "hyperc/macros.h"
//#include "hyperc/debug.h"
//#include "hyperc/descriptors.h"

#include "utils/debug.h"
#include "utils/macros.h"
#include "hyperc/lexer.h"
#include "hyperc/parser.h"
#include "hyperc/output.h"
#include "hyperc/match/lexer.h"
#include "hyperc/match/parser.h"
#include "hyperc/test.h"



#define GFLAG_NOOP (1<<0)
#define GFLAG_TEST (1<<1)


typedef uint64_t global_flags_t;



enum {
    //AFLAG_H = 0,

    BFLAG_TEST = 0,

    FLAGID_INVALID = -1,

    FLAGID_TYPE_MASK = 0xFFFF0000,
    FLAGID_TYPE_A = 0x00010000,
    FLAGID_TYPE_B = 0x00020000,
};
typedef uint32_t flagid_t;




typedef struct {
    char* outfile;
    char* infile;
    global_flags_t flags;
} global_t;



#endif /* #ifndef HYPERC_H */
