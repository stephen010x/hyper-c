
/* ########################
 * ##  !! IMPORTANT !!   ##
 * ########################
 *
 * This document is based almost entierly on these specs and
 * matching rules:
 * 
 * https://port70.net/~nsz/c/c23/n3220.html#A
 *
 * ######################## */



#include "hyperc/match/parser.h"


// start at:
// https://port70.net/~nsz/c/c23/n3220.html#A.3



/*
    New idea
    The easiest things to match should go first rather than last
    That way we can just have a simple cache where future matches that use
    those as..
    ehh, nevermind, the idea is a big shaky. Come back to this.

    // TODO
    I could test a hashmap cache though, where every match type with match position and match size
    are cached in a hashmap with overwriting conflicts. and then test the speed of the parser
    at different cache sizes and different hash keys, and pick a balance between cache size and 
    speed. 

    An alternative idea is that when a match is found, it will then check all later 
    matches to see if it is a valid subset of a different match. this way we ultimately end up
    with the match that has the most subsets

    Although, one potential assumption to make is that a match is always correct
    But multiple matches can be correct due to subsets, which actually feels rather unideal.
    So instead mayhaps we have a prime matching statement, or multiple prime matching statements, through 
    which all other statements are matched through their subsets.
    For instance, in a C file, the only things we should prime match for are type declarations,
    static declarations, static initilzers, and function declarations and definitions, with perhaps
    a few more.

    Now, instead of having a flag that marks a matcher as a prime matcher, instead we should have
    a function similar to next_token, where we do a next_match, but testing a particular match.
    And that match will either return a no-match, or it would return a match with an output.
    And then we can just do a loop that checks all of our prime matches until there are no more
    matches in the document. 

    And then the input/output can be a struct that contains this information, as a sort of 
    self-implementation and not fundamental to the match tree:

     - state

     - input
     - inoffset
     - inalloc

     - output
     - outoffset
     - outalloc

     - line number   (may just be placed in the state?)
     - column number (may just be placed in the state?)

    but basically in other words, the order of objects in the match tree do not matter at all, 
    beause submatches are checked using the index in the descriptor. So all checks are tree-like
    rather than array-like. Which, when tested with a cache should be rather fast.

    On the note of the cache, we never need to flush the cache either. Previous checks can potentially
    add tokens and matches that will be reused by future prime matches, or even different branches of 
    the same prime match.

    I should count cache hits, and cache misses, for debugging purposes.

    Ooh. I could have a cache counter. So existing entries in the cache have a life counter.
    Any cache hit in that spot will increase its life counter. And any cache misses in that spot
    will reduce that counter. And when the counter is reduced to zero, it is overwritten by the new 
    entry.
    Perhaps have hits and misses during reads affect the life counter. And then if the life counter is
    zero or less, then writes will overwrite it.
    New entries that overwrite the previous will reset the counter to one. The reason for this is because
    the way I will use this cache will basically guarentee that a cache read will occur in a location 
    before attempting a cache write. So if the life is 1 and it is a miss, then it will be overwritten by
    the following write

    Also, I was thinking alternatively we can have the cache-read only increment the counter when a
    cache-hit, and then have the cache-write only decrement the counter when there is a mismatch.
    IDK. Look into this later.
    
*/



match_tree_t mtree;

mtree.tree = (uint32_t **[]){

    //////////////////////////////////////////////////////
    //            EXPRESSIONS                           ##
    //####################################################

    [MATCH_IDENTIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ TOKEN_IDENTIFIER | MF_TOKEN_TYPE, NULL },    // identifier
        NULL,
    },
    
    [MATCH_CONSTANT] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ TOKEN_NUMERIC | MF_TOKEN_TYPE, NULL },       // constant
        NULL,
    },
    
    [MATCH_STRING_LITERAL] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ TOKEN_STRING | MF_TOKEN_TYPE, NULL },        // string-literal
        NULL,
    },

    // primary-expression
    [MATCH_PRIMARY_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ MATCH_IDENTIFIER, NULL },        // identifier
        [TYPE_B] = (uint32_t []){ MATCH_CONSTANT, NULL },          // constant
        [TYPE_C] = (uint32_t []){ MATCH_STRING_LITERAL, NULL },    // string-literal
        [TYPE_D] = (uint32_t []){
            UTOKEN_L_RBRACKET | MF_UTOKEN,                              // ( expression )
            MATCH_IDENTIFIER,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // postfix-expression
    [MATCH_POSTFIX_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // primary-expression
            MATCH_PRIMARY_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // postfix-expression [ expression ]
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // postfix-expression ( argument-expression-listopt )
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_ARGUMENT_EXRESSION_LIST | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_D] = (uint32_t []){       // postfix-expression . identifier
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_PERIOD | MF_UTOKEN,
            MATCH_IDENTIFIER,
            NULL,
        },
        [TYPE_E] = (uint32_t []){       // postfix-expression -> identifier
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_R_ARROW | MF_UTOKEN,
            MATCH_IDENTIFIER,
            NULL,
        },
        [TYPE_F] = (uint32_t []){       // postfix-expression ++
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_INCREMENT | MF_UTOKEN,
            NULL,
        },
        [TYPE_G] = (uint32_t []){       // postfix-expression --
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_DECREMENT | MF_UTOKEN,
            NULL,
        },
        [TYPE_H] = (uint32_t []){       // compound-literal
            MATCH_COMPOUND_LITERAL,
            NULL,
        },
        NULL,
    },

    // argument-expression-list
    [MATCH_ARGUMENT_EXPRESSION_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // assignment-expression
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // argument-expression-list , assignment-expression
            MATCH_EXRESSION_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // compound-literal
    [MATCH_COMPOUND_LITERAL] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // ( storage-class-specifiersopt type-name ) braced-initializer
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_STORAGE_CLASS_SPECIFIERS | MF_OPTIONAL,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_BRACED_INITIALIZER,
            NULL,
        },
        NULL,
    },
    
    // storage-class-specifiers
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // storage-class-specifier
            MATCH_STORAGE_CLASS_SPECIFIER,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // storage-class-specifiers storage-class-specifier
            MATCH_STORAGE_CLASS_SPECIFIERS,
            MATCH_STORAGE_CLASS_SPECIFIER,
            NULL,
        },
        NULL,
    },

    // unary-expression
    [MATCH_UNARY_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // postfix-expression
            MATCH_POSTFIX_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // ++ unary-expression
            UTOKEN_INCREMENT | MF_UTOKEN
            MATCH_UNARY_EXPRESSION,
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // -- unary-expression
            UTOKEN_DECREMENT | MF_UTOKEN
            MATCH_GENERIC,
            NULL,
        },
        [TYPE_D] = (uint32_t []){       // unary-operator cast-expression
            MATCH_UNARY_OPERATOR,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_E] = (uint32_t []){       // sizeof unary-expression
            DTOKEN_SIZEOF | MF_DTOKEN,
            MATCH_UNARY_EXPRESSION,
            NULL,
        },
        [TYPE_F] = (uint32_t []){       // sizeof ( type-name )
            DTOKEN_SIZEOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            NULL,
        },
        [TYPE_G] = (uint32_t []){       // alignof ( type-name )
            DTOKEN_ALIGNOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            NULL,
        },
        NULL,
    },

    // unary-operator
    [MATCH_UNARY_OPERATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ UTOKEN_AND     | MF_UTOKEN, NULL }, // &
        [TYPE_B] = (uint32_t []){ UTOKEN_STAR    | MF_UTOKEN, NULL }, // *
        [TYPE_C] = (uint32_t []){ UTOKEN_PLUS    | MF_UTOKEN, NULL }, // +
        [TYPE_D] = (uint32_t []){ UTOKEN_MINUS   | MF_UTOKEN, NULL }, // -
        [TYPE_E] = (uint32_t []){ UTOKEN_TILDE   | MF_UTOKEN, NULL }, // ~
        [TYPE_F] = (uint32_t []){ UTOKEN_EXCLAIM | MF_UTOKEN, NULL }, // !
        NULL,
    },

    // cast-expression
    [MATCH_CAST_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // unary-expression
            MATCH_UNARY_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // ( type-name ) cast-expression
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // multiplicative-expression
    [MATCH_MULTIPLICATIVE_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // cast-expression
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // multiplicative-expression * cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_STAR | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // multiplicative-expression / cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_FSLASH | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_D] = (uint32_t []){       // multiplicative-expression % cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_PERCENT | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // additive-expression
    [MATCH_ADDITIVE_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // multiplicative-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // additive-expression + multiplicative-expression
            MATCH_ADDITIVE_EXPRESSION,
            UTOKEN_PLUS | MF_UTOKEN,
            MATCH_MULTIPLICATIVE_EXPRESSION
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // additive-expression - multiplicative-expression
            MATCH_ADDITIVE_EXPRESSION,
            UTOKEN_MINUS | MF_UTOKEN,
            MATCH_MULTIPLICATIVE_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // shift-expression
    [MATCH_SHIFT_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // additive-expression
            MATCH_ADDITIVE_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // shift-expression << additive-expression
            MATCH_SHIFT_EXPRESSION,
            UTOKEN_LSHIFT | MF_UTOKEN,
            MATCH_ADDITIVE_EXPRESSION
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // shift-expression >> additive-expression
            MATCH_SHIFT_EXPRESSION,
            UTOKEN_RSHIFT | MF_UTOKEN,
            MATCH_ADDITIVE_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // relational-expression
    [MATCH_RELATIONAL_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // shift-expression
            MATCH_SHIFT_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // relational-expression < shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_L_ABRACKET | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // relational-expression > shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_R_ABRACKET | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        [TYPE_D] = (uint32_t []){       // relational-expression <= shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_LEQUAL | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        [TYPE_E] = (uint32_t []){       // relational-expression >= shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_GEQUAL | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // equality-expression
    [MATCH_EQUALITY_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // relational-expression
            MATCH_RELATIONAL_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // equality-expression == relational-expression
            MATCH_EQUALITY_EXPRESSION,
            UTOKEN_EQUALS | MF_UTOKEN,
            MATCH_RELATIONAL_EXPRESSION
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // equality-expression != relational-expression
            MATCH_EQUALITY_EXPRESSION,
            UTOKEN_NEQUALS | MF_UTOKEN,
            MATCH_RELATIONAL_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // AND-expression
    [MATCH_AND_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // equality-expression
            MATCH_EQUALITY_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // AND-expression & equality-expression
            MATCH_AND_EXPRESSION,
            UTOKEN_AND | MF_UTOKEN,
            MATCH_EQUALITY_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // exclusive-OR-expression
    [MATCH_EXCLUSIVE_OR_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // AND-expression
            MATCH_AND_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // exclusive-OR-expression ^ AND-expression
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            UTOKEN_XOR | MF_UTOKEN,
            MATCH_AND_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // inclusive-OR-expression
    [MATCH_INCLUSIVE_OR_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // exclusive-OR-expression
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // inclusive-OR-expression | exclusive-OR-expression
            MATCH_INCLUSIVE_OR_EXPRESSION,
            UTOKEN_OR | MF_UTOKEN,
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // logical-AND-expression
    [MATCH_LOGICAL_AND_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // inclusive-OR-expression
            MATCH_INCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // logical-AND-expression && inclusive-OR-expression
            MATCH_LOGICAL_AND_EXPRESSION,
            UTOKEN_OR | MF_UTOKEN,
            MATCH_INCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // logical-OR-expression
    [MATCH_LOGICAL_OR_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // logical-AND-expression
            MATCH_LOGICAL_AND_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // logical-OR-expression || logical-AND-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            UTOKEN_LOR | MF_UTOKEN,
            MATCH_LOGICAL_AND_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // conditional-expression
    [MATCH_CONDITIONAL_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // logical-OR-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // logical-OR-expression ? expression : conditional-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            UTOKEN_QUESTION | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_COLON | MF_UTOKEN,
            MATCH_CONDITIONAL_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // assignment-expression
    [MATCH_ASSIGNMENT_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // conditional-expression
            MATCH_CONDITIONAL_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // unary-expression assignment-operator assignment-expression
            MATCH_UNARY_EXPRESSION,
            MATCH_ASSIGNMENT_OPERATOR,
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // assignment-operator
    [MATCH_ASSIGNMENT_OPERATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ UTOKEN_ASSIGN | MF_UTOKEN, NULL }, // =
        [TYPE_B] = (uint32_t []){ UTOKEN_MUL_EQ | MF_UTOKEN, NULL }, // *=
        [TYPE_C] = (uint32_t []){ UTOKEN_DIV_EQ | MF_UTOKEN, NULL }, // /=
        [TYPE_D] = (uint32_t []){ UTOKEN_MOD_EQ | MF_UTOKEN, NULL }, // %=
        [TYPE_E] = (uint32_t []){ UTOKEN_ADD_EQ | MF_UTOKEN, NULL }, // +=
        [TYPE_I] = (uint32_t []){ UTOKEN_AND_EQ | MF_UTOKEN, NULL }, // &=
        [TYPE_J] = (uint32_t []){ UTOKEN_XOR_EQ | MF_UTOKEN, NULL }, // ^=
        [TYPE_K] = (uint32_t []){ UTOKEN_OR_EQ  | MF_UTOKEN, NULL }, // |=
        [TYPE_F] = (uint32_t []){ UTOKEN_SUB_EQ | MF_UTOKEN, NULL }, // -=
        [TYPE_G] = (uint32_t []){ UTOKEN_LSHIFT_EQ | MF_UTOKEN, NULL }, // <<=
        [TYPE_H] = (uint32_t []){ UTOKEN_RSHIFT_EQ | MF_UTOKEN, NULL }, // >>=
        NULL,
    },
    
    // expression
    [MATCH_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // assignment-expression
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // expression , assignment-expression
            MATCH_EXPRESSION,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // constant-expression
    [MATCH_CONSTANT_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // conditional-expression
            MATCH_CONDITIONAL_EXPRESSION,
            NULL,
        },
        NULL,
    },


    //////////////////////////////////////////////////////
    //            DECLARATIONS                          ##
    //####################################################

    // declaration
    [MATCH_DECLARATION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // declaration-specifiers init-declarator-listopt ;
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_INIT_DECLARATOR_LIST | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (uint32_t []){ // attribute-specifier-sequence declaration-specifiers init-declarator-list ;
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_INIT_DECLARATOR_LIST,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // static_assert-declaration
            MATCH_STATIC_ASSERT_DECLARATION,
            NULL,
        },
        [TYPE_D] = (uint32_t []){       // attribute-declaration
            MATCH_ATTRIBUTE_DECLARATION,
            NULL,
        },
        NULL,
    },

    // declaration-specifiers
    [MATCH_DECLARATION_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // declaration-specifier attribute-specifier-sequenceopt
            MATCH_DECLARATION_SPECIFIER,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // declaration-specifier declaration-specifiers
            MATCH_DECLARATION_SPECIFIER,
            MATCH_DECLARATION_SPECIFIERS,
            NULL,
        },
        NULL,
    },

    // declaration-specifier
    [MATCH_DECLARATION_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ MATCH_STORAGE_CLASS_SPECIFIER, NULL },  // storage-class-specifier
        [TYPE_B] = (uint32_t []){ MATCH_TYPE_SPECIFIER_QUALIFIER, NULL }, // type-specifier-qualifier
        [TYPE_C] = (uint32_t []){ MATCH_FUNCTION_SPECIFIER, NULL },       // function-specifier
        NULL,
    },

    // init-declarator-list
    [MATCH_INIT_DECLARATOR_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // init-declarator
            MATCH_INIT_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // init-declarator-list , init-declarator
            MATCH_INIT_DECLARATOR_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_INIT_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // init-declarator
    [MATCH_INIT_DECLARATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // declarator
            MATCH_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // declarator = initializer
            MATCH_DECLARATOR,
            UTOKEN_ASSIGN | MF_UTOKEN,
            MATCH_INITIALIZER,
            NULL,
        },
        NULL,
    },

    // TODO: implement later
    // attribute-declaration
    [MATCH_ATTRIBUTE_DECLARATION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // storage-class-specifier
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ DTOKEN_AUTO         | MF_DTOKEN, NULL },  // auto
        [TYPE_B] = (uint32_t []){ DTOKEN_CONSTEXPR    | MF_DTOKEN, NULL },  // constexpr
        [TYPE_C] = (uint32_t []){ DTOKEN_EXTERN       | MF_DTOKEN, NULL },  // extern
        [TYPE_D] = (uint32_t []){ DTOKEN_REGISTER     | MF_DTOKEN, NULL },  // register
        [TYPE_E] = (uint32_t []){ DTOKEN_STATIC       | MF_DTOKEN, NULL },  // static
        [TYPE_F] = (uint32_t []){ DTOKEN_THREAD_LOCAL | MF_DTOKEN, NULL },  // thread_local
        [TYPE_G] = (uint32_t []){ DTOKEN_TYPEDEF      | MF_DTOKEN, NULL },  // typedef
        NULL,
    },

    // type-specifier
    [MATCH_TYPE_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ DTOKEN_VOID       | MF_DTOKEN, NULL },    // void
        [TYPE_B] = (uint32_t []){ DTOKEN_CHAR       | MF_DTOKEN, NULL },    // char
        [TYPE_C] = (uint32_t []){ DTOKEN_SHORT      | MF_DTOKEN, NULL },    // short
        [TYPE_D] = (uint32_t []){ DTOKEN_INT        | MF_DTOKEN, NULL },    // int
        [TYPE_E] = (uint32_t []){ DTOKEN_LONG       | MF_DTOKEN, NULL },    // long
        [TYPE_F] = (uint32_t []){ DTOKEN_FLOAT      | MF_DTOKEN, NULL },    // float
        [TYPE_G] = (uint32_t []){ DTOKEN_DOUBLE     | MF_DTOKEN, NULL },    // double
        [TYPE_H] = (uint32_t []){ DTOKEN_SIGNED     | MF_DTOKEN, NULL },    // signed
        [TYPE_I] = (uint32_t []){ DTOKEN_UNSIGNED   | MF_DTOKEN, NULL },    // unsigned
        [TYPE_J] = (uint32_t []){ DTOKEN_BOOL       | MF_DTOKEN, NULL },    // bool
        [TYPE_K] = (uint32_t []){ DTOKEN_COMPLEX    | MF_DTOKEN, NULL },    // _Complex
        [TYPE_L] = (uint32_t []){ DTOKEN_DECIMAL32  | MF_DTOKEN, NULL },    // _Decimal32
        [TYPE_M] = (uint32_t []){ DTOKEN_DECIMAL64  | MF_DTOKEN, NULL },    // _Decimal64
        [TYPE_N] = (uint32_t []){ DTOKEN_DECIMAL128 | MF_DTOKEN, NULL },    // _Decimal128
        [TYPE_O] = (uint32_t []){ MATCH_ENUM_SPECIFIER,    NULL },          // enum-specifier
        [TYPE_P] = (uint32_t []){ MATCH_TYPEDEF_NAME,      NULL },          // typedef-name
        [TYPE_Q] = (uint32_t []){ MATCH_TYPEOF_SPECIFIER , NULL },          // typeof-specifier
        [TYPE_R] = (uint32_t []){ MATCH_ATOMIC_TYPE_SPECIFIER,     NULL },  // atomic-type-specifier
        [TYPE_S] = (uint32_t []){ MATCH_STRUCT_OR_UNION_SPECIFIER, NULL },  // struct-or-union-specifier
        [TYPE_T] = (uint32_t []){ 
            DTOKEN_BITINT | MF_DTOKEN,          // _BitInt ( constant-expression )
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // struct-or-union-specifier
    [MATCH_STRUCT_OR_UNION_SPECIFIER] = (uint32_t *[]){
            // struct-or-union attribute-specifier-sequenceopt identifieropt { member-declaration-list }
        [TYPE_A] = (uint32_t []){
            MATCH_STRUCT_OR_UNION,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_IDENTIFIER | MF_OPTIONAL,
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_MEMBER_DECLARATION_LIST,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (uint32_t []){   // struct-or-union attribute-specifier-sequenceopt identifier
            MATCH_STRUCT_OR_UNION,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_IDENTIFIER,
            NULL,
        },
        NULL,
    },

    // struct-or-union
    [MATCH_STRUCT_OR_UNION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){  DTOKEN_STRUCT | MF_DTOKEN, NULL }, // struct
        [TYPE_B] = (uint32_t []){  DTOKEN_UNION  | MF_DTOKEN, NULL }, // union
        NULL,
    },

    // member-declaration-list
    [MATCH_MEMBER_DECLARATION_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // member-declaration
            MATCH_MEMBER_DECLARATION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // member-declaration-list member-declaration
            MATCH_MEMBER_DECLARATION_LIST,
            MATCH_MEMBER_DECLARATION,
            NULL,
        },
        NULL,
    },

    // member-declaration
    [MATCH_MEMBER_DECLARATION] = (uint32_t *[]){
            // attribute-specifier-sequenceopt specifier-qualifier-list member-declarator-listopt ;
        [TYPE_A] = (uint32_t []){
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_SPECIFIER_QUALIFIER_LIST,
            MATCH_MEMBER_DECLARATOR_LIST | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // static_assert-declaration
            MATCH_STATIC_ASSERT_DECLARATION,
            NULL,
        },
        NULL,
    },

    // specifier-qualifier-list
    [MATCH_SPECIFIER_QUALIFIER_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // type-specifier-qualifier attribute-specifier-sequenceop
            MATCH_TYPE_SPECIFIER_QUALIFIER,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // type-specifier-qualifier specifier-qualifier-list
            MATCH_TYPE_SPECIFIER_QUALIFIER,
            MATCH_SPECIFIER_QUALIFIER_LIST,
            NULL,
        },
        NULL,
    },

    // type-specifier-qualifier
    [MATCH_TYPE_SPECIFIER_QUALIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ MATCH_TYPE_SPECIFIER, NULL },         // type-specifier
        [TYPE_B] = (uint32_t []){ MATCH_TYPE_QUALIFIER, NULL, },        // type-qualifier
        [TYPE_C] = (uint32_t []){ MATCH_ALIGNMENT_SPECIFIER, NULL, },   // alignment-specifier
        NULL,
    },

    // member-declarator-list
    [MATCH_MEMBER_DECLARATOR_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // member-declarator
            MATCH_MEMBER_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // member-declarator-list , member-declarator
            MATCH_MEMBER_DECLARATOR_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_MEMBER_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // member-declarator
    [MATCH_MEMBER_DECLARATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // declarator
            MATCH_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // declaratoropt : constant-expression
            MATCH_DECLARATOR | MF_OPTIONAL,
            UTOKEN_COLON | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // enum-specifier
    [MATCH_ENUM_SPECIFIER] = (uint32_t *[]){
            // enum attribute-specifier-sequenceopt identifieropt enum-type-specifieropt { enumerator-list }
        [TYPE_A] = (uint32_t []){
            DTOKEN_ENUM | MF_DTOKEN,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_IDENTIFIER | MF_OPTIONAL,
            MATCH_ENUM_TYPE_SPECIFIER | MF_OPTIONAL,
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_ENUMERATOR_LIST,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },  // enum attribute-specifier-sequenceopt identifieropt enum-type-specifieropt { enumerator-list , }
        [TYPE_B] = (uint32_t []){
            DTOKEN_ENUM | MF_DTOKEN,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_IDENTIFIER | MF_OPTIONAL,
            MATCH_ENUM_TYPE_SPECIFIER | MF_OPTIONAL,
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_ENUMERATOR_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // enum identifier enum-type-specifieropt
            DTOKEN_ENUM | MF_DTOKEN,
            MATCH_IDENTIFIER,
            MATCH_ENUM_TYPE_SPECIFIER | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // enumerator-list
    [MATCH_ENUMERATOR_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // enumerator
            MATCH_ENUMERATOR,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // enumerator-list , enumerator
            MATCH_ENUMERATOR_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_ENUMERATOR,
            NULL,
        },
        NULL,
    },

    // enumerator
    [MATCH_ENUMERATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // enumeration-constant attribute-specifier-sequenceopt
            MATCH_ENUMERATION_CONSTANT,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (uint32_t []){ // enumeration-constant attribute-specifier-sequenceopt = constant-expression
            MATCH_ENUMERATION_CONSTANT,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            UTOKEN_ASSIGN | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // enum-type-specifier
    [MATCH_ENUM_TYPE_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // : specifier-qualifier-list
            UTOKEN_COLON | MF_UTOKEN,
            MATCH_SPECIFIER_QUALIFIER_LIST,
            NULL,
        },
        NULL,
    },

    // atomic-type-specifier
    [MATCH_ATOMIC_TYPE_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // _Atomic ( type-name )
            DTOKEN_ATOMIC | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // typeof-specifier
    [MATCH_TYPEOF_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // typeof ( typeof-specifier-argument )
            DTOKEN_TYPEOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPEOF_SPECIFIER_ARGUMENT,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // typeof_unqual ( typeof-specifier-argument )
            DTOKEN_TYPEOF_UNQUAL | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPEOF_SPECIFIER_ARGUMENT,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // typeof-specifier-argument
    [MATCH_TYPEOF_SPECIFIER_ARGUMENT] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ MATCH_EXPRESSION, NULL }, // expression
        [TYPE_B] = (uint32_t []){ MATCH_TYPE_NAME,  NULL }, // type-name
        NULL,
    },

    // type-qualifier
    [MATCH_TYPE_QUALIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ DTOKEN_CONST    | MF_DTOKEN, NULL }, // const
        [TYPE_B] = (uint32_t []){ DTOKEN_RESTRICT | MF_DTOKEN, NULL }, // restrict
        [TYPE_C] = (uint32_t []){ DTOKEN_VOLATILE | MF_DTOKEN, NULL }, // volatile
        [TYPE_D] = (uint32_t []){ DTOKEN_ATOMIC   | MF_DTOKEN, NULL }, // _Atomic
        NULL,
    },

    // function-specifier
    [MATCH_FUNCTION_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ DTOKEN_INLINE   | MF_DTOKEN, NULL }, // inline
        [TYPE_B] = (uint32_t []){ DTOKEN_NORETURN | MF_DTOKEN, NULL }, // _Noreturn
        NULL,
    },

    // alignment-specifier
    [MATCH_ALIGNMENT_SPECIFIER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // alignas ( type-name )
            DTOKEN_ALIGNAS | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // alignas ( constant-expression )
            DTOKEN_ALIGNAS | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // declarator
    [MATCH_DECLARATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // pointeropt direct-declarator
            MATCH_POINTER | MF_OPTIONAL,
            MATCH_DIRECT_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // direct-declarator
    [MATCH_DIRECT_DECLARATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // identifier attribute-specifier-sequenceopt
            MATCH_IDENTIFIER,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // ( declarator )
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_DECLARATOR,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // array-declarator attribute-specifier-sequenceopt
            MATCH_ARRAY_DECLARATOR,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_D] = (uint32_t []){       // function-declarator attribute-specifier-sequenceopt
            MATCH_FUNCTION_DECLARATOR,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // array-declarator
    [MATCH_ARRAY_DECLARATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ // direct-declarator [ type-qualifier-listopt assignment-expressionopt ]
            MATCH_DIRECT_DECLARATOR,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_ASSIGNMENT_EXPRESSION | MF_OPTIONAL,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (uint32_t []){ // direct-declarator [ static type-qualifier-listopt assignment-expression ]
            MATCH_DIRECT_DECLARATOR,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            DTOKEN_STATIC | MF_DTOKEN,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_ASSIGNMENT_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (uint32_t []){ // direct-declarator [ type-qualifier-list static assignment-expression ]
            MATCH_DIRECT_DECLARATOR,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_TYPE_QUALIFIER_LIST,
            DTOKEN_STATIC | MF_DTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_D] = (uint32_t []){       // direct-declarator [ type-qualifier-listopt * ]
            MATCH_DIRECT_DECLARATOR,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            UTOKEN_STAR | MF_UTOKEN,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // function-declarator
    [MATCH_FUNCTION_DECLARATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // direct-declarator ( parameter-type-listopt )
            DIRECT_DECLARATOR,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_PARAMETER_TYPE_LIST | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // pointer
    [MATCH_POINTER] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // * attribute-specifier-sequenceopt type-qualifier-listopt
            UTOKEN_STAR | MF_UTOKEN,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // * attribute-specifier-sequenceopt type-qualifier-listopt pointer
            UTOKEN_STAR | MF_UTOKEN,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_POINTER,
            NULL,
        },
        NULL,
    },

    // type-qualifier-list
    [MATCH_TYPE_QUALIFIER_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // type-qualifier
            MATCH_TYPE_QUALIFIER,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // type-qualifier-list type-qualifier
            MATCH_TYPE_QUALIFIER_LIST,
            MATCH_TYPE_QUALIFIER,
            NULL,
        },
        NULL,
    },

    // parameter-type-list
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // parameter-list
            MATCH_PARAMETER_LIST,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // parameter-list, ...
            MATCH_PARAMETER_LIST,
            UTOKEN_VARGS | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (uint32_t []){       // ...
            UTOKEN_VARGS | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // parameter-list
    [MATCH_PARAMETER_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // parameter-declaration
            MATCH_PARAMETER_DECLARATION,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // parameter-list , parameter-declaration
            MATCH_PARAMETER_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_PARAMETER_DECLARATION,
            NULL,
        },
        NULL,
    },

    // parameter-declaration
    [MATCH_PARAMETER_DECLARATION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // attribute-specifier-sequenceopt declaration-specifiers declarator
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_DECLARATOR,
            NULL,
        },      // attribute-specifier-sequenceopt declaration-specifiers abstract-declaratoropt
        [TYPE_B] = (uint32_t []){
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // type-name
    [MATCH_TYPE_NAME] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // specifier-qualifier-list abstract-declaratoropt
            MATCH_SPECIFIER_QUALIFIER_LIST,
            MATCH_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // abstract-declarator
    [MATCH_ABSTRACT_DECLARATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // pointer
            MATCH_GENERIC,
            NULL,
        },
        [TYPE_B] = (uint32_t []){       // pointeropt direct-abstract-declarator
            MATCH_POINTER | MF_OPTIONAL,
            MATCH_DIRECT_ABSTRACT_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // direct-abstract-declarator
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },
    
    // array-abstract-declarator
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },
    
    // function-abstract-declarator
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // typedef-name
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // braced-initializer
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // initializer
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // initializer-list
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // designation
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // designator-list
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // designator
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // static_assert-declaration
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

    // attribute-specifier-sequence
    // attribute-specifier
    // attribute-list
    // attribute
    // attribute-token
    // standard-attribute
    // attribute-prefixed-token
    // attribute-prefix
    // attribute-argument-clause
    // balanced-token-sequence
    // balanced-token
};


/*

[MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

*/
