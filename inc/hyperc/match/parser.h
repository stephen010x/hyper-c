#ifndef METAC_MATCH_PARSER_H
#define METAC_MATCH_PARSER_H



#include <stdint.h>



#define TYPE_A  0
#define TYPE_B  1
#define TYPE_C  2
#define TYPE_D  3
#define TYPE_E  4
#define TYPE_F  5
#define TYPE_G  6
#define TYPE_H  7
#define TYPE_I  8
#define TYPE_J  9
#define TYPE_K  10
#define TYPE_L  11
#define TYPE_M  12
#define TYPE_N  13
#define TYPE_O  14
#define TYPE_P  15
#define TYPE_Q  16
#define TYPE_R  17
#define TYPE_S  18
#define TYPE_T  19
#define TYPE_U  20
#define TYPE_V  21
#define TYPE_W  22
#define TYPE_X  23
#define TYPE_Y  24
#define TYPE_Z  25



#define MF_MATCH_MASK   ( 0x0000FFFF )
#define MF_ZERO_OR_ONE  ( 0x1<<16 )
#define MF_OPTIONAL     ( 0x1<<16 )     // maybe flags like this can be handled by the input handler...
// #define MF_ZERO_OR_MORE ( 0x1<<17 )
// #define MF_ONE_OR_MORE  ( 0x1<<18 )
#define MF_UTOKEN       ( 0x1<<19 )     // ...since these would be handled by the input handler as well
#define MF_DTOKEN       ( 0x1<<20 )
#define MF_TOKEN_TYPE   ( 0x1<<21 )




// TODO: Order these from easiest to determine/match to hardest
enum {
    MATCH_END = 0,
};
typedef uint32_t match_class_t;





// typedef struct {
//     uint32_t **layouts;
// } mclass_t;


typedef struct {
    void (*input_handler)(void);    // TODO: Use different function prototype
    void (*output_handler)(void);   // TODO: Use different function prototype
    //mclass_t *mtree;
    uint32_t ***tree;
} match_tree_t;


/* (kinda cool)

# Arity Series (latin)

    0 - Nullary
    1 - Unary
    2 - Binary
    3 - Ternary
    4 - Quaternary


# Adicity Series (greek)

    0 - Niladic
    1 - Monadic
    2 - Dyadic
    3 - Triadic
    4 - Tetradic

*/






#endif /* #ifndef METAC_MATCH_PARSER_H */
