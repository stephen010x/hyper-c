// hyperc/parser.h

#ifndef HYPERC_PARSER_H
#define HYPERC_PARSER_H



#include <stdint.h>




// reserved flags
// 0xF0000000

// non-reserved flags
// 0x0FFF0000

// if any of the non-reserved flags are set, then the match is
// passed to the input_handler, rather than called recursively by match()


// bits 16-27 are free


#define MF_FLAGS_MASK       0xFFFF0000
#define MF_MATCH_MASK       0x0000FFFF
#define MF_RESERVED_MASK    0xF0000000
#define MF_UNRES_MASK       0x0FFF0000
#define MF_FLAG_SHIFT       16
#define MF_RESFLAG_SHIFT    28

#define MF_OPTIONAL         ( 0b1<<28 )
#define _MF_RESERVED_1      ( 0b1<<29 )
#define _MF_RESERVED_2      ( 0b1<<30 )
#define MF_NOT_IMPLEMENTED  ( 0b1<<31 )

//#define MF_ZERO_OR_ONE  ( 0b1<<16 )
//#define MF_ZERO_OR_MORE ( 0b1<<17 )
//#define MF_ONE_OR_MORE  ( 0b1<<18 )


// can pass in an array expression, or an array type
// will return a decayed version of that.
#define DECAY(__array) typeof(&(*(typeof(__array)*)NULL)[0])


// remember, an array can decay to a pointer, but it itself
// is not a pointer. So for clarity, use pointers to arrays
// rather than let it decay for all but array indexing
// actually, nevermind. Taking a pointer to an array
// will result in a type that is a pointer to an array
// with no decay. So in other words we need arrays 
// to decay.

// a subrule is an integer
// a rule is a variable length static array of const decayed subrules
// a target is a variable length static array of const decayed rules
// a tree is a variable length static array of const decayed targets
typedef uint32_t match_subrule_t;

typedef match_subrule_t const static_rule_t[];
typedef DECAY(static_rule_t) match_rule_t;

typedef match_rule_t const static_target_t[];
typedef DECAY(static_target_t) match_target_t;

typedef match_target_t const static_tree_t[];
typedef DECAY(static_tree_t) match_tree_t;



// typedef struct {
//     uint32_t **layouts;
// } mclass_t;


// TODO: try to merge tget_handler and eof_handler
typedef void *(*tget_handler_t)(void *input, int index);
typedef bool (*tmatch_handler_t)(void *token, uint32_t mid);
typedef bool (*eof_handler_t)(void *input, int index);
typedef char *(*ptoken_name_handler_t)(ptoken_t *ptoken, char *buff, int max);



typedef struct {
    //void (*input_handler)(void);    // TODO: Use different function prototype
    //void (*free_handler)(void);     // TODO: Use different function prototype
    //void (*output_handler)(void);   // TODO: Use different function prototype
    tget_handler_t tget_handler;
    tmatch_handler_t tmatch_handler;
    eof_handler_t eof_handler;
    //void *state;
    
    match_tree_t tree;
    
    //int tree_len;
    //int state_len;
    //int token_size; // in bytes. if 1, then it is basically a string input
} match_t;







enum {
    PFLAG_USER = 1,
    PFLAG_MATCH,
};


typedef struct ptoken_t;
typedef struct {
    uint32_t type;
    union {
        struct {
            int tindex; // index into input token list
            void *token;
        } user;
        struct {
            //uint32_t mid;
            int targetid;
            int ruleid;
            int count;
            ptoken_t **children;
        } match;
    };
} ptoken_t;








typedef struct {
    union {
        int index; // points to end of stack, not last item of stack
        int length;
    }
    int alloc;
    ptoken_t **data;
} pstack_t;






void ptoken_free(ptoken_t *token);
void print_ptoken(ptoken_t *token, ptoken_name_handler_t handler);
ptoken_t *match(match_t *m, int rule, void *input);




// typedef struct {
// } match_args_t;



// #ifdef __DEBUG__
// 
// //int test_match_tree_init(void);
// int test_match_tree(token_t *tokens, int_t token_count);
// 
// #endif /* #ifdef __DEBUG__ */





#endif /* #ifndef HYPERC_PARSER_H */
