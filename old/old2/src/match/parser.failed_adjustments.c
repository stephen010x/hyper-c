#ifdef OLD_CODE_DO_NOT_COMPILE


/*
 * == Note ==
 * These rules have been adjusted to fix left hand recursion
 * and are first-match-first-goes. Which just means that 
 * the instant a subrule matches, it doesn't bother checking
 * the others.
 */



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




static_tree_t c_mtree = (static_target_t){

    [MATCH_IDENTIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ TOKEN_IDENTIFIER | MF_TOKEN_TYPE, NULL },    // identifier
        NULL,
    },
    
    [MATCH_CONSTANT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ TOKEN_NUMERIC | MF_TOKEN_TYPE, NULL },       // constant
        NULL,
    },
    
    [MATCH_STRING_LITERAL] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ TOKEN_STRING | MF_TOKEN_TYPE, NULL },        // string-literal
        NULL,
    },


    //////////////////////////////////////////////////////
    //            EXPRESSIONS                           ##
    //####################################################

    // primary-expression
    [MATCH_PRIMARY_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_IDENTIFIER, NULL },        // identifier
        [TYPE_B] = (static_subrule_t){ MATCH_CONSTANT, NULL },          // constant
        [TYPE_C] = (static_subrule_t){ MATCH_STRING_LITERAL, NULL },    // string-literal
        [TYPE_D] = (static_subrule_t){
            UTOKEN_L_RBRACKET | MF_UTOKEN,                              // ( expression )
            MATCH_IDENTIFIER,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // // CUSTOM postfix-expression-alt
    // [MATCH_POSTFIX_EXPRESSION_ALT] = (static_rule_t){
    //     [TYPE_A] = (static_subrule_t){ MATCH_PRIMARY_EXPRESSION, NULL },    // primary-expression
    //     [TYPE_B] = (static_subrule_t){ MATCH_COMPOUND_LITERAL, NULL },      // compound-literal
    // },

    // // postfix-expression
    // [MATCH_POSTFIX_EXPRESSION] = (static_rule_t){
    //     [TYPE_A] = (static_subrule_t){       // primary-expression
    //         MATCH_PRIMARY_EXPRESSION,
    //         NULL,
    //     },
    //     [TYPE_B] = (static_subrule_t){       // postfix-expression [ expression ]
    //         MATCH_POSTFIX_EXPRESSION,
    //         UTOKEN_L_SBRACKET | MF_UTOKEN,
    //         MATCH_EXPRESSION,
    //         UTOKEN_R_SBRACKET | MF_UTOKEN,
    //         NULL,
    //     },
    //     [TYPE_C] = (static_subrule_t){       // postfix-expression ( argument-expression-listopt )
    //         MATCH_POSTFIX_EXPRESSION,
    //         UTOKEN_L_RBRACKET | MF_UTOKEN,
    //         MATCH_ARGUMENT_EXRESSION_LIST | MF_OPTIONAL,
    //         UTOKEN_R_RBRACKET | MF_UTOKEN,
    //         NULL,
    //     },
    //     [TYPE_D] = (static_subrule_t){       // postfix-expression . identifier
    //         MATCH_POSTFIX_EXPRESSION,
    //         UTOKEN_PERIOD | MF_UTOKEN,
    //         MATCH_IDENTIFIER,
    //         NULL,
    //     },
    //     [TYPE_E] = (static_subrule_t){       // postfix-expression -> identifier
    //         MATCH_POSTFIX_EXPRESSION,
    //         UTOKEN_R_ARROW | MF_UTOKEN,
    //         MATCH_IDENTIFIER,
    //         NULL,
    //     },
    //     [TYPE_F] = (static_subrule_t){       // postfix-expression ++
    //         MATCH_POSTFIX_EXPRESSION,
    //         UTOKEN_INCREMENT | MF_UTOKEN,
    //         NULL,
    //     },
    //     [TYPE_G] = (static_subrule_t){       // postfix-expression --
    //         MATCH_POSTFIX_EXPRESSION,
    //         UTOKEN_DECREMENT | MF_UTOKEN,
    //         NULL,
    //     },
    //     [TYPE_H] = (static_subrule_t){       // compound-literal
    //         MATCH_COMPOUND_LITERAL,
    //         NULL,
    //     },
    //     NULL,
    // },

    // CUSTOM postfix-operator
    [MATCH_POSTFIX_OPERATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // [ expression ]
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // ( argument-expression-listopt )
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_ARGUMENT_EXRESSION_LIST | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // . identifier
            UTOKEN_PERIOD | MF_UTOKEN,
            MATCH_IDENTIFIER,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // -> identifier
            UTOKEN_R_ARROW | MF_UTOKEN,
            MATCH_IDENTIFIER,
            NULL,
        },
        [TYPE_E] = (static_subrule_t){       // ++
            UTOKEN_INCREMENT | MF_UTOKEN,
            NULL,
        },
        [TYPE_F] = (static_subrule_t){       // --
            UTOKEN_DECREMENT | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // postfix-expression
    // fixed for recursion and first match
    [MATCH_POSTFIX_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // primary-expression
            MATCH_PRIMARY_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // postfix-expression postfix-operator
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_H] = (static_subrule_t){       // compound-literal
            MATCH_COMPOUND_LITERAL,
            NULL,
        },
        NULL,
    },

    // postfix-expression:
    //     postfix-expression postfix-operator
    //     primary-expression
    //     compound-literal

    // // argument-expression-list
    // [MATCH_ARGUMENT_EXPRESSION_LIST] = (static_rule_t){
    //     [TYPE_A] = (static_subrule_t){       // assignment-expression
    //         MATCH_ASSIGNMENT_EXPRESSION,
    //         NULL,
    //     },
    //     [TYPE_B] = (static_subrule_t){       // argument-expression-list , assignment-expression
    //         MATCH_EXRESSION_LIST,
    //         UTOKEN_COMMA | MF_UTOKEN,
    //         MATCH_ASSIGNMENT_EXPRESSION,
    //         NULL,
    //     },
    //     NULL,
    // },

    // argument-expression-list
    // fixed for recursion and first match
    [MATCH_ARGUMENT_EXPRESSION_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // argument-expression , assignment-expression-list
            MATCH_ASSIGNMENT_EXPRESSION,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_EXRESSION_LIST,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // assignment-expression
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // compound-literal
    [MATCH_COMPOUND_LITERAL] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // ( storage-class-specifiersopt type-name ) braced-initializer
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_STORAGE_CLASS_SPECIFIERS | MF_OPTIONAL,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_BRACED_INITIALIZER,
            NULL,
        },
        NULL,
    },
    
    // // storage-class-specifiers
    // [MATCH_STORAGE_CLASS_SPECIFIERS] = (static_rule_t){
    //     [TYPE_A] = (static_subrule_t){       // storage-class-specifier
    //         MATCH_STORAGE_CLASS_SPECIFIER,
    //         NULL,
    //     },
    //     [TYPE_B] = (static_subrule_t){       // storage-class-specifiers storage-class-specifier
    //         MATCH_STORAGE_CLASS_SPECIFIERS,
    //         MATCH_STORAGE_CLASS_SPECIFIER,
    //         NULL,
    //     },
    //     NULL,
    // },

    // storage-class-specifiers
    // fixed for recursion and first match
    [MATCH_STORAGE_CLASS_SPECIFIERS] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // storage-class-specifier storage-class-specifiers
            MATCH_STORAGE_CLASS_SPECIFIER,
            MATCH_STORAGE_CLASS_SPECIFIERS,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // storage-class-specifier
            MATCH_STORAGE_CLASS_SPECIFIER,
            NULL,
        },
        NULL,
    },

    // unary-expression
    [MATCH_UNARY_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // postfix-expression
            MATCH_POSTFIX_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // ++ unary-expression
            UTOKEN_INCREMENT | MF_UTOKEN,
            MATCH_UNARY_EXPRESSION,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // -- unary-expression
            UTOKEN_DECREMENT | MF_UTOKEN,
            MATCH_UNARY_EXPRESSION,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // unary-operator cast-expression
            MATCH_UNARY_OPERATOR,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_E] = (static_subrule_t){       // sizeof unary-expression
            DTOKEN_SIZEOF | MF_DTOKEN,
            MATCH_UNARY_EXPRESSION,
            NULL,
        },
        [TYPE_F] = (static_subrule_t){       // sizeof ( type-name )
            DTOKEN_SIZEOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            NULL,
        },
        [TYPE_G] = (static_subrule_t){       // alignof ( type-name )
            DTOKEN_ALIGNOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            NULL,
        },
        NULL,
    },

    // unary-operator
    [MATCH_UNARY_OPERATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ UTOKEN_AND     | MF_UTOKEN, NULL }, // &
        [TYPE_B] = (static_subrule_t){ UTOKEN_STAR    | MF_UTOKEN, NULL }, // *
        [TYPE_C] = (static_subrule_t){ UTOKEN_PLUS    | MF_UTOKEN, NULL }, // +
        [TYPE_D] = (static_subrule_t){ UTOKEN_MINUS   | MF_UTOKEN, NULL }, // -
        [TYPE_E] = (static_subrule_t){ UTOKEN_TILDE   | MF_UTOKEN, NULL }, // ~
        [TYPE_F] = (static_subrule_t){ UTOKEN_EXCLAIM | MF_UTOKEN, NULL }, // !
        NULL,
    },

    // cast-expression
    [MATCH_CAST_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // unary-expression
            MATCH_UNARY_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // ( type-name ) cast-expression
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // multiplicative-expression
    [MATCH_MULTIPLICATIVE_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // cast-expression
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // multiplicative-expression * cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_STAR | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // multiplicative-expression / cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_FSLASH | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // multiplicative-expression % cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_PERCENT | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // additive-expression
    [MATCH_ADDITIVE_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // multiplicative-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // additive-expression + multiplicative-expression
            MATCH_ADDITIVE_EXPRESSION,
            UTOKEN_PLUS | MF_UTOKEN,
            MATCH_MULTIPLICATIVE_EXPRESSION
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // additive-expression - multiplicative-expression
            MATCH_ADDITIVE_EXPRESSION,
            UTOKEN_MINUS | MF_UTOKEN,
            MATCH_MULTIPLICATIVE_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // shift-expression
    [MATCH_SHIFT_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // additive-expression
            MATCH_ADDITIVE_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // shift-expression << additive-expression
            MATCH_SHIFT_EXPRESSION,
            UTOKEN_LSHIFT | MF_UTOKEN,
            MATCH_ADDITIVE_EXPRESSION
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // shift-expression >> additive-expression
            MATCH_SHIFT_EXPRESSION,
            UTOKEN_RSHIFT | MF_UTOKEN,
            MATCH_ADDITIVE_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // relational-expression
    [MATCH_RELATIONAL_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // shift-expression
            MATCH_SHIFT_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // relational-expression < shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_L_ABRACKET | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // relational-expression > shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_R_ABRACKET | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // relational-expression <= shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_LEQUAL | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        [TYPE_E] = (static_subrule_t){       // relational-expression >= shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_GEQUAL | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // equality-expression
    [MATCH_EQUALITY_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // relational-expression
            MATCH_RELATIONAL_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // equality-expression == relational-expression
            MATCH_EQUALITY_EXPRESSION,
            UTOKEN_EQUALS | MF_UTOKEN,
            MATCH_RELATIONAL_EXPRESSION
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // equality-expression != relational-expression
            MATCH_EQUALITY_EXPRESSION,
            UTOKEN_NEQUALS | MF_UTOKEN,
            MATCH_RELATIONAL_EXPRESSION
            NULL,
        },
        NULL,
    },
    
    // AND-expression
    [MATCH_AND_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // equality-expression
            MATCH_EQUALITY_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // AND-expression & equality-expression
            MATCH_AND_EXPRESSION,
            UTOKEN_AND | MF_UTOKEN,
            MATCH_EQUALITY_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // exclusive-OR-expression
    [MATCH_EXCLUSIVE_OR_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // AND-expression
            MATCH_AND_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // exclusive-OR-expression ^ AND-expression
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            UTOKEN_XOR | MF_UTOKEN,
            MATCH_AND_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // inclusive-OR-expression
    [MATCH_INCLUSIVE_OR_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // exclusive-OR-expression
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // inclusive-OR-expression | exclusive-OR-expression
            MATCH_INCLUSIVE_OR_EXPRESSION,
            UTOKEN_OR | MF_UTOKEN,
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // logical-AND-expression
    [MATCH_LOGICAL_AND_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // inclusive-OR-expression
            MATCH_INCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // logical-AND-expression && inclusive-OR-expression
            MATCH_LOGICAL_AND_EXPRESSION,
            UTOKEN_OR | MF_UTOKEN,
            MATCH_INCLUSIVE_OR_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // logical-OR-expression
    [MATCH_LOGICAL_OR_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // logical-AND-expression
            MATCH_LOGICAL_AND_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // logical-OR-expression || logical-AND-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            UTOKEN_LOR | MF_UTOKEN,
            MATCH_LOGICAL_AND_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // conditional-expression
    [MATCH_CONDITIONAL_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // logical-OR-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // logical-OR-expression ? expression : conditional-expression
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
    [MATCH_ASSIGNMENT_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // conditional-expression
            MATCH_CONDITIONAL_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // unary-expression assignment-operator assignment-expression
            MATCH_UNARY_EXPRESSION,
            MATCH_ASSIGNMENT_OPERATOR,
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // assignment-operator
    [MATCH_ASSIGNMENT_OPERATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ UTOKEN_ASSIGN | MF_UTOKEN, NULL }, // =
        [TYPE_B] = (static_subrule_t){ UTOKEN_MUL_EQ | MF_UTOKEN, NULL }, // *=
        [TYPE_C] = (static_subrule_t){ UTOKEN_DIV_EQ | MF_UTOKEN, NULL }, // /=
        [TYPE_D] = (static_subrule_t){ UTOKEN_MOD_EQ | MF_UTOKEN, NULL }, // %=
        [TYPE_E] = (static_subrule_t){ UTOKEN_ADD_EQ | MF_UTOKEN, NULL }, // +=
        [TYPE_I] = (static_subrule_t){ UTOKEN_AND_EQ | MF_UTOKEN, NULL }, // &=
        [TYPE_J] = (static_subrule_t){ UTOKEN_XOR_EQ | MF_UTOKEN, NULL }, // ^=
        [TYPE_K] = (static_subrule_t){ UTOKEN_OR_EQ  | MF_UTOKEN, NULL }, // |=
        [TYPE_F] = (static_subrule_t){ UTOKEN_SUB_EQ | MF_UTOKEN, NULL }, // -=
        [TYPE_G] = (static_subrule_t){ UTOKEN_LSHIFT_EQ | MF_UTOKEN, NULL }, // <<=
        [TYPE_H] = (static_subrule_t){ UTOKEN_RSHIFT_EQ | MF_UTOKEN, NULL }, // >>=
        NULL,
    },
    
    // expression
    [MATCH_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // assignment-expression
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // expression , assignment-expression
            MATCH_EXPRESSION,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            NULL,
        },
        NULL,
    },
    
    // constant-expression
    [MATCH_CONSTANT_EXPRESSION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // conditional-expression
            MATCH_CONDITIONAL_EXPRESSION,
            NULL,
        },
        NULL,
    },


    //////////////////////////////////////////////////////
    //            DECLARATIONS                          ##
    //####################################################

    // declaration
    [MATCH_DECLARATION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // declaration-specifiers init-declarator-listopt ;
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_INIT_DECLARATOR_LIST | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){ // attribute-specifier-sequence declaration-specifiers init-declarator-list ;
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_INIT_DECLARATOR_LIST,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // static_assert-declaration
            MATCH_STATIC_ASSERT_DECLARATION,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // attribute-declaration
            MATCH_ATTRIBUTE_DECLARATION,
            NULL,
        },
        NULL,
    },

    // declaration-specifiers
    [MATCH_DECLARATION_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // declaration-specifier attribute-specifier-sequenceopt
            MATCH_DECLARATION_SPECIFIER,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // declaration-specifier declaration-specifiers
            MATCH_DECLARATION_SPECIFIER,
            MATCH_DECLARATION_SPECIFIERS,
            NULL,
        },
        NULL,
    },

    // declaration-specifier
    [MATCH_DECLARATION_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_STORAGE_CLASS_SPECIFIER, NULL },  // storage-class-specifier
        [TYPE_B] = (static_subrule_t){ MATCH_TYPE_SPECIFIER_QUALIFIER, NULL }, // type-specifier-qualifier
        [TYPE_C] = (static_subrule_t){ MATCH_FUNCTION_SPECIFIER, NULL },       // function-specifier
        NULL,
    },

    // init-declarator-list
    [MATCH_INIT_DECLARATOR_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // init-declarator
            MATCH_INIT_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // init-declarator-list , init-declarator
            MATCH_INIT_DECLARATOR_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_INIT_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // init-declarator
    [MATCH_INIT_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // declarator
            MATCH_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // declarator = initializer
            MATCH_DECLARATOR,
            UTOKEN_ASSIGN | MF_UTOKEN,
            MATCH_INITIALIZER,
            NULL,
        },
        NULL,
    },

    // TODO: implement later
    // attribute-declaration
    [MATCH_ATTRIBUTE_DECLARATION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // not implemented yet
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // storage-class-specifier
    [MATCH_STORAGE_CLASS_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ DTOKEN_AUTO         | MF_DTOKEN, NULL },  // auto
        [TYPE_B] = (static_subrule_t){ DTOKEN_CONSTEXPR    | MF_DTOKEN, NULL },  // constexpr
        [TYPE_C] = (static_subrule_t){ DTOKEN_EXTERN       | MF_DTOKEN, NULL },  // extern
        [TYPE_D] = (static_subrule_t){ DTOKEN_REGISTER     | MF_DTOKEN, NULL },  // register
        [TYPE_E] = (static_subrule_t){ DTOKEN_STATIC       | MF_DTOKEN, NULL },  // static
        [TYPE_F] = (static_subrule_t){ DTOKEN_THREAD_LOCAL | MF_DTOKEN, NULL },  // thread_local
        [TYPE_G] = (static_subrule_t){ DTOKEN_TYPEDEF      | MF_DTOKEN, NULL },  // typedef
        NULL,
    },

    // type-specifier
    [MATCH_TYPE_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ DTOKEN_VOID       | MF_DTOKEN, NULL },    // void
        [TYPE_B] = (static_subrule_t){ DTOKEN_CHAR       | MF_DTOKEN, NULL },    // char
        [TYPE_C] = (static_subrule_t){ DTOKEN_SHORT      | MF_DTOKEN, NULL },    // short
        [TYPE_D] = (static_subrule_t){ DTOKEN_INT        | MF_DTOKEN, NULL },    // int
        [TYPE_E] = (static_subrule_t){ DTOKEN_LONG       | MF_DTOKEN, NULL },    // long
        [TYPE_F] = (static_subrule_t){ DTOKEN_FLOAT      | MF_DTOKEN, NULL },    // float
        [TYPE_G] = (static_subrule_t){ DTOKEN_DOUBLE     | MF_DTOKEN, NULL },    // double
        [TYPE_H] = (static_subrule_t){ DTOKEN_SIGNED     | MF_DTOKEN, NULL },    // signed
        [TYPE_I] = (static_subrule_t){ DTOKEN_UNSIGNED   | MF_DTOKEN, NULL },    // unsigned
        [TYPE_J] = (static_subrule_t){ DTOKEN_BOOL       | MF_DTOKEN, NULL },    // bool
        [TYPE_K] = (static_subrule_t){ DTOKEN_COMPLEX    | MF_DTOKEN, NULL },    // _Complex
        [TYPE_L] = (static_subrule_t){ DTOKEN_DECIMAL32  | MF_DTOKEN, NULL },    // _Decimal32
        [TYPE_M] = (static_subrule_t){ DTOKEN_DECIMAL64  | MF_DTOKEN, NULL },    // _Decimal64
        [TYPE_N] = (static_subrule_t){ DTOKEN_DECIMAL128 | MF_DTOKEN, NULL },    // _Decimal128
        [TYPE_O] = (static_subrule_t){ MATCH_ENUM_SPECIFIER,    NULL },          // enum-specifier
        [TYPE_P] = (static_subrule_t){ MATCH_TYPEDEF_NAME,      NULL },          // typedef-name
        [TYPE_Q] = (static_subrule_t){ MATCH_TYPEOF_SPECIFIER , NULL },          // typeof-specifier
        [TYPE_R] = (static_subrule_t){ MATCH_ATOMIC_TYPE_SPECIFIER,     NULL },  // atomic-type-specifier
        [TYPE_S] = (static_subrule_t){ MATCH_STRUCT_OR_UNION_SPECIFIER, NULL },  // struct-or-union-specifier
        [TYPE_T] = (static_subrule_t){ 
            DTOKEN_BITINT | MF_DTOKEN,          // _BitInt ( constant-expression )
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // struct-or-union-specifier
    [MATCH_STRUCT_OR_UNION_SPECIFIER] = (static_rule_t){
            // struct-or-union attribute-specifier-sequenceopt identifieropt { member-declaration-list }
        [TYPE_A] = (static_subrule_t){
            MATCH_STRUCT_OR_UNION,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_IDENTIFIER | MF_OPTIONAL,
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_MEMBER_DECLARATION_LIST,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){   // struct-or-union attribute-specifier-sequenceopt identifier
            MATCH_STRUCT_OR_UNION,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_IDENTIFIER,
            NULL,
        },
        NULL,
    },

    // struct-or-union
    [MATCH_STRUCT_OR_UNION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){  DTOKEN_STRUCT | MF_DTOKEN, NULL }, // struct
        [TYPE_B] = (static_subrule_t){  DTOKEN_UNION  | MF_DTOKEN, NULL }, // union
        NULL,
    },

    // member-declaration-list
    [MATCH_MEMBER_DECLARATION_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // member-declaration
            MATCH_MEMBER_DECLARATION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // member-declaration-list member-declaration
            MATCH_MEMBER_DECLARATION_LIST,
            MATCH_MEMBER_DECLARATION,
            NULL,
        },
        NULL,
    },

    // member-declaration
    [MATCH_MEMBER_DECLARATION] = (static_rule_t){
            // attribute-specifier-sequenceopt specifier-qualifier-list member-declarator-listopt ;
        [TYPE_A] = (static_subrule_t){
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_SPECIFIER_QUALIFIER_LIST,
            MATCH_MEMBER_DECLARATOR_LIST | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // static_assert-declaration
            MATCH_STATIC_ASSERT_DECLARATION,
            NULL,
        },
        NULL,
    },

    // specifier-qualifier-list
    [MATCH_SPECIFIER_QUALIFIER_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // type-specifier-qualifier attribute-specifier-sequenceop
            MATCH_TYPE_SPECIFIER_QUALIFIER,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // type-specifier-qualifier specifier-qualifier-list
            MATCH_TYPE_SPECIFIER_QUALIFIER,
            MATCH_SPECIFIER_QUALIFIER_LIST,
            NULL,
        },
        NULL,
    },

    // type-specifier-qualifier
    [MATCH_TYPE_SPECIFIER_QUALIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_TYPE_SPECIFIER, NULL },         // type-specifier
        [TYPE_B] = (static_subrule_t){ MATCH_TYPE_QUALIFIER, NULL, },        // type-qualifier
        [TYPE_C] = (static_subrule_t){ MATCH_ALIGNMENT_SPECIFIER, NULL, },   // alignment-specifier
        NULL,
    },

    // member-declarator-list
    [MATCH_MEMBER_DECLARATOR_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // member-declarator
            MATCH_MEMBER_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // member-declarator-list , member-declarator
            MATCH_MEMBER_DECLARATOR_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_MEMBER_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // member-declarator
    [MATCH_MEMBER_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // declarator
            MATCH_DECLARATOR,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // declaratoropt : constant-expression
            MATCH_DECLARATOR | MF_OPTIONAL,
            UTOKEN_COLON | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // enum-specifier
    [MATCH_ENUM_SPECIFIER] = (static_rule_t){
            // enum attribute-specifier-sequenceopt identifieropt enum-type-specifieropt { enumerator-list }
        [TYPE_A] = (static_subrule_t){
            DTOKEN_ENUM | MF_DTOKEN,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_IDENTIFIER | MF_OPTIONAL,
            MATCH_ENUM_TYPE_SPECIFIER | MF_OPTIONAL,
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_ENUMERATOR_LIST,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },  // enum attribute-specifier-sequenceopt identifieropt enum-type-specifieropt { enumerator-list , }
        [TYPE_B] = (static_subrule_t){
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
        [TYPE_C] = (static_subrule_t){       // enum identifier enum-type-specifieropt
            DTOKEN_ENUM | MF_DTOKEN,
            MATCH_IDENTIFIER,
            MATCH_ENUM_TYPE_SPECIFIER | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // enumerator-list
    [MATCH_ENUMERATOR_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // enumerator
            MATCH_ENUMERATOR,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // enumerator-list , enumerator
            MATCH_ENUMERATOR_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_ENUMERATOR,
            NULL,
        },
        NULL,
    },

    // enumerator
    [MATCH_ENUMERATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // enumeration-constant attribute-specifier-sequenceopt
            MATCH_ENUMERATION_CONSTANT,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){ // enumeration-constant attribute-specifier-sequenceopt = constant-expression
            MATCH_ENUMERATION_CONSTANT,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            UTOKEN_ASSIGN | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            NULL,
        },
        NULL,
    },

    // enum-type-specifier
    [MATCH_ENUM_TYPE_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // : specifier-qualifier-list
            UTOKEN_COLON | MF_UTOKEN,
            MATCH_SPECIFIER_QUALIFIER_LIST,
            NULL,
        },
        NULL,
    },

    // atomic-type-specifier
    [MATCH_ATOMIC_TYPE_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // _Atomic ( type-name )
            DTOKEN_ATOMIC | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // typeof-specifier
    [MATCH_TYPEOF_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // typeof ( typeof-specifier-argument )
            DTOKEN_TYPEOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPEOF_SPECIFIER_ARGUMENT,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // typeof_unqual ( typeof-specifier-argument )
            DTOKEN_TYPEOF_UNQUAL | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPEOF_SPECIFIER_ARGUMENT,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // typeof-specifier-argument
    [MATCH_TYPEOF_SPECIFIER_ARGUMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_EXPRESSION, NULL }, // expression
        [TYPE_B] = (static_subrule_t){ MATCH_TYPE_NAME,  NULL }, // type-name
        NULL,
    },

    // type-qualifier
    [MATCH_TYPE_QUALIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ DTOKEN_CONST    | MF_DTOKEN, NULL }, // const
        [TYPE_B] = (static_subrule_t){ DTOKEN_RESTRICT | MF_DTOKEN, NULL }, // restrict
        [TYPE_C] = (static_subrule_t){ DTOKEN_VOLATILE | MF_DTOKEN, NULL }, // volatile
        [TYPE_D] = (static_subrule_t){ DTOKEN_ATOMIC   | MF_DTOKEN, NULL }, // _Atomic
        NULL,
    },

    // function-specifier
    [MATCH_FUNCTION_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ DTOKEN_INLINE   | MF_DTOKEN, NULL }, // inline
        [TYPE_B] = (static_subrule_t){ DTOKEN_NORETURN | MF_DTOKEN, NULL }, // _Noreturn
        NULL,
    },

    // alignment-specifier
    [MATCH_ALIGNMENT_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // alignas ( type-name )
            DTOKEN_ALIGNAS | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // alignas ( constant-expression )
            DTOKEN_ALIGNAS | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // declarator
    [MATCH_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // pointeropt direct-declarator
            MATCH_POINTER | MF_OPTIONAL,
            MATCH_DIRECT_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // direct-declarator
    [MATCH_DIRECT_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // identifier attribute-specifier-sequenceopt
            MATCH_IDENTIFIER,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // ( declarator )
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_DECLARATOR,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // array-declarator attribute-specifier-sequenceopt
            MATCH_ARRAY_DECLARATOR,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // function-declarator attribute-specifier-sequenceopt
            MATCH_FUNCTION_DECLARATOR,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // array-declarator
    [MATCH_ARRAY_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ // direct-declarator [ type-qualifier-listopt assignment-expressionopt ]
            MATCH_DIRECT_DECLARATOR,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_ASSIGNMENT_EXPRESSION | MF_OPTIONAL,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){ // direct-declarator [ static type-qualifier-listopt assignment-expression ]
            MATCH_DIRECT_DECLARATOR,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            DTOKEN_STATIC | MF_DTOKEN,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_ASSIGNMENT_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){ // direct-declarator [ type-qualifier-list static assignment-expression ]
            MATCH_DIRECT_DECLARATOR,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_TYPE_QUALIFIER_LIST,
            DTOKEN_STATIC | MF_DTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // direct-declarator [ type-qualifier-listopt * ]
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
    [MATCH_FUNCTION_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // direct-declarator ( parameter-type-listopt )
            DIRECT_DECLARATOR,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_PARAMETER_TYPE_LIST | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // pointer
    [MATCH_POINTER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // * attribute-specifier-sequenceopt type-qualifier-listopt
            UTOKEN_STAR | MF_UTOKEN,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // * attribute-specifier-sequenceopt type-qualifier-listopt pointer
            UTOKEN_STAR | MF_UTOKEN,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_POINTER,
            NULL,
        },
        NULL,
    },

    // type-qualifier-list
    [MATCH_TYPE_QUALIFIER_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // type-qualifier
            MATCH_TYPE_QUALIFIER,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // type-qualifier-list type-qualifier
            MATCH_TYPE_QUALIFIER_LIST,
            MATCH_TYPE_QUALIFIER,
            NULL,
        },
        NULL,
    },

    // parameter-type-list
    [MATCH_PARAMETER_TYPE_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // parameter-list
            MATCH_PARAMETER_LIST,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // parameter-list, ...
            MATCH_PARAMETER_LIST,
            UTOKEN_VARGS | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // ...
            UTOKEN_VARGS | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // parameter-list
    [MATCH_PARAMETER_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // parameter-declaration
            MATCH_PARAMETER_DECLARATION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // parameter-list , parameter-declaration
            MATCH_PARAMETER_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_PARAMETER_DECLARATION,
            NULL,
        },
        NULL,
    },

    // parameter-declaration
    [MATCH_PARAMETER_DECLARATION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // attribute-specifier-sequenceopt declaration-specifiers declarator
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_DECLARATOR,
            NULL,
        },      // attribute-specifier-sequenceopt declaration-specifiers abstract-declaratoropt
        [TYPE_B] = (static_subrule_t){
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // type-name
    [MATCH_TYPE_NAME] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // specifier-qualifier-list abstract-declaratoropt
            MATCH_SPECIFIER_QUALIFIER_LIST,
            MATCH_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },

    // abstract-declarator
    [MATCH_ABSTRACT_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // pointer
            MATCH_POINTER,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // pointeropt direct-abstract-declarator
            MATCH_POINTER | MF_OPTIONAL,
            MATCH_DIRECT_ABSTRACT_DECLARATOR,
            NULL,
        },
        NULL,
    },

    // direct-abstract-declarator
    [MATCH_DIRECT_ABSTRACT_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // ( abstract-declarator )
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_ABSTRACT_DECLARATOR,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // array-abstract-declarator attribute-specifier-sequenceopt
            MATCH_ARRAY_ABSTRACT_DECLARATOR,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // function-abstract-declarator attribute-specifier-sequenceopt
            MATCH_FUNCTION_ABSTRACT_DECLARATOR,
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            NULL,
        },
        NULL,
    },
    
    // array-abstract-declarator
    [MATCH_ARRAY_ABSTRACT_DECLARATOR] = (static_rule_t){
                // direct-abstract-declaratoropt [ type-qualifier-listopt assignment-expressionopt ]
        [TYPE_A] = (static_subrule_t){ 
            MATCH_DIRECT_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_ASSIGNMENT_EXPRESSION | MF_OPTIONAL,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },      // direct-abstract-declaratoropt [ static type-qualifier-listopt assignment-expression ]
        [TYPE_B] = (static_subrule_t){
            MATCH_DIRECT_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            DTOKEN_STATIC | MF_DTOKEN,
            MATCH_TYPE_QUALIFIER_LIST | MF_OPTIONAL,
            MATCH_ASSIGNMENT_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },      // direct-abstract-declaratoropt [ type-qualifier-list static assignment-expression ]
        [TYPE_C] = (static_subrule_t){
            MATCH_DIRECT_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_TYPE_QUALIFIER_LIST,
            DTOKEN_STATIC | MF_DTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },      // direct-abstract-declaratoropt [ * ]
        [TYPE_D] = (static_subrule_t){
            MATCH_DIRECT_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            UTOKEN_STAR | MF_UTOKEN,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },
    
    // function-abstract-declarator
    [MATCH_FUNCTION_ABSTRACT_DECLARATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // direct-abstract-declaratoropt ( parameter-type-listopt )
            MATCH_DIRECT_ABSTRACT_DECLARATOR | MF_OPTIONAL,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_PARAMETER_TYPE_LIST | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // typedef-name
    [MATCH_TYPEDEF_NAME] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_IDENTIFIER, NULL }, // identifier
        NULL,
    },

    // braced-initializer
    [MATCH_BRACED_INITIALIZER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // { }
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // { initializer-list }
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_INITIALIZER_LIST,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // { initializer-list , }
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_INITIALIZER_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // initializer
    [MATCH_INITIALIZER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_ASSIGNMENT_EXPRESSION, NULL },  // assignment-expression
        [TYPE_B] = (static_subrule_t){ MATCH_BRACED_INITIALIZER,    NULL },  // braced-initializer
        NULL,
    },

    // initializer-list
    [MATCH_INITIALIZER_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // designationopt initializer
            MATCH_DESIGNATION | MF_OPTIONAL,
            MATCH_INITIALIZER,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // initializer-list , designationopt initializer
            MATCH_INITIALIZER_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_DESIGNATION | MF_OPTIONAL,
            MATCH_INITIALIZER,
            NULL,
        },
        NULL,
    },

    // designation
    [MATCH_DESIGNATION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // designator-list =
            MATCH_DESIGNATOR_LIST,
            UTOKEN_ASSIGN | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // designator-list
    [MATCH_DESIGNATOR_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // designator
            MATCH_DESIGNATOR,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // designator-list designator
            MATCH_DESIGNATOR_LIST,
            MATCH_DESIGNATOR,
            NULL,
        },
        NULL,
    },

    // MATCH_DESIGNATOR
    [MATCH_DESIGNATOR] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // [ constant-expression ]
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // . identifier
            UTOKEN_PERIOD | MF_UTOKEN,
            MATCH_IDENTIFIER,
            NULL,
        },
        NULL,
    },

    // static_assert-declaration
    [MATCH_STATIC_ASSERT_DECLARATION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // static_assert ( constant-expression , string-literal ) ;
            MATCH_STATIC_ASSERT,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_STRING_LITERAL,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // static_assert ( constant-expression ) ;
            MATCH_STATIC_ASSERT,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // attribute-specifier-sequence
    [MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },
    
    // attribute-specifier
    [MATCH_ATTRIBUTE_SPECIFIER] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },
    
    // attribute-list
    [MATCH_ATTRIBUTE_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // attribute
    [MATCH_ATTRIBUTE] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // attribute-token
    [MATCH_ATTRIBUTE_TOKEN] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // standard-attribute
    [MATCH_STANDARD_ATTRIBUTE] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // attribute-prefixed-token
    [MATCH_ATTRIBUTE_PREFIX_TOKEN] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // attribute-prefix
    [MATCH_ATTRIBUTE_PREFIX] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // attribute-argument-clause
    [MATCH_ATTRIBUTE_ARGUMENT_CLAUSE] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },

    // balanced-token-sequence
    [MATCH_BALANCED_TOKEN_SEQUENCE] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },
    // balanced-token
    [MATCH_BALANCED_TOKEN] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // !! not yet implemented !!
            MF_NOT_IMPLEMENTED,
            NULL,
        },
        NULL,
    },


    //////////////////////////////////////////////////////
    //            STATEMENTS                            ##
    //####################################################

    // statement
    [MATCH_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_LABELED_STATEMENT,   NULL },    // labeled-statement
        [TYPE_B] = (static_subrule_t){ MATCH_UNLABELED_STATEMENT, NULL },    // unlabeled-statement
        NULL,
    },

    // unlabeled-statement
    [MATCH_UNLABELED_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // expression-statement
            MATCH_EXPRESSION_STATEMENT,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // attribute-specifier-sequenceopt primary-block
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_PRIMARY_BLOCK,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // attribute-specifier-sequenceopt jump-statement
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_JUMP_STATEMENT,
            NULL,
        },
        NULL,
    },

    // primary-block
    [MATCH_PRIMARY_BLOCK] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_COMPOUND_STATEMENT,  NULL },    // compound-statement
        [TYPE_B] = (static_subrule_t){ MATCH_SELECTION_STATEMENT, NULL },    // selection-statement
        [TYPE_C] = (static_subrule_t){ MATCH_ITERATION_STATEMENT, NULL },    // iteration-statement
        NULL,
    },

    // secondary-block
    [MATCH_SECONDARY_BLOCK] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_STATEMENT,  NULL },     // statement
        NULL,
    },

    // label
    [MATCH_LABEL] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // attribute-specifier-sequenceopt identifier :
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTINOAL,
            MATCH_IDENTIFIER,
            UTOKEN_COLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // attribute-specifier-sequenceopt case constant-expression :
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTINOAL,
            DTOKEN_CASE | MF_DTOKEN,
            MATCH_CONSTANT_EXPRESSION,
            UTOKEN_COLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // attribute-specifier-sequenceopt default :
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTINOAL,
            DTOKEN_DEFAULT | MF_DTOKEN,
            UTOKEN_COLON | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // labeled-statement
    [MATCH_LABELED_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // label statement
            MATCH_LABEL,
            MATCH_STATEMENT,
            NULL,
        },
        NULL,
    },

    // compound-statement
    [MATCH_COMPOUND_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // { block-item-listopt }
            UTOKEN_L_CBRACKET | MF_UTOKEN,
            MATCH_BLOCK_ITEM_LIST | MF_OPTIONAL,
            UTOKEN_R_CBRACKET | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // block-item-list
    [MATCH_BLOCK_ITEM_LIST] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // block-item
            MATCH_BLOCK_ITEM,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // block-item-list block-item
            MATCH_BLOCK_ITEM_LIST,
            MATCH_BLOCK_ITEM
            NULL,
        },
        NULL,
    },

    // block-item
    [MATCH_BLOCK_ITEM] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_DECLARATION,         NULL },    // declaration
        [TYPE_B] = (static_subrule_t){ MATCH_UNLABELED_STATEMENT, NULL },    // unlabeled-statement
        [TYPE_C] = (static_subrule_t){ MATCH_LABEL,               NULL },    // label
        NULL,
    },

    // expression-statement
    [MATCH_EXPRESSION_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // expressionopt ;
            MATCH_EXPRESSION | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // attribute-specifier-sequence expression ;
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE,
            MATCH_EXPRESSION,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        NULL,
    },

    // selection-statement
    [MATCH_SELECTION_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // if ( expression ) secondary-block
            DTOKEN_IF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_SECONDARY_BLOCK,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // if ( expression ) secondary-block else secondary-block
            DTOKEN_IF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_SECONDARY_BLOCK,
            DTOKEN_ELSE | MF_DTOKEN,
            MATCH_SECONDARY_BLOCK,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // switch ( expression ) secondary-block
            DTOKEN_SWITCH | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_SECONDARY_BLOCK,
            NULL,
        },
        NULL,
    },

    // iteration-statement
    [MATCH_ITERATION_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // while ( expression ) secondary-block
            DTOKEN_WHILE | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_SECONDARY_BLOCK,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // do secondary-block while ( expression ) ;
            DTOKEN_DO | MF_DTOKEN,
            MATCH_SECONDARY_BLOCK,
            DTOKEN_WHILE | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){   // for ( expressionopt ; expressionopt ; expressionopt ) secondary-block
            DTOKEN_FOR | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            MATCH_EXPRESSION | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            MATCH_EXPRESSION | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_SECONDARY_BLOCK,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // for ( declaration expressionopt ; expressionopt ) secondary-block
            DTOKEN_FOR | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_DECLARATION,              // declarations end with a semicolon, so still two
            MATCH_EXPRESSION | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_UTOKEN,
            MATCH_EXPRESSION | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_SECONDARY_BLOCK,
            NULL,
        },
        NULL,
    },

    // jump-statement
    [MATCH_JUMP_STATEMENT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // goto identifier ;
            DTOKEN_GOTO | MF_DTOKEN,
            MATCH_IDENTIFIER,
            UTOKEN_SEMICOLON | MF_SEMICOLON,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // continue ;
            DTOKEN_CONTINUE | MF_DTOKEN,
            UTOKEN_SEMICOLON | MF_SEMICOLON,
            NULL,
        },
        [TYPE_C] = (static_subrule_t){       // break ;
            DTOKEN_BREAK | MF_DTOKEN,
            UTOKEN_SEMICOLON | MF_SEMICOLON,
            NULL,
        },
        [TYPE_D] = (static_subrule_t){       // return expressionopt ;
            DTOKEN_RETURN | MF_DTOKEN,
            MATCH_EXPRESSION | MF_OPTIONAL,
            UTOKEN_SEMICOLON | MF_SEMICOLON,
            NULL,
        },
        NULL,
    },


    //////////////////////////////////////////////////////
    //            EXTERNAL DEFINITIONS                  ##
    //####################################################

            // THIS IS THE PRIME MATCHING RULE
            
    // translation-unit
    [MATCH_TRANSLATION_UNIT] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // external-declaration
            MATCH_EXTERNAL_DECLARATION,
            NULL,
        },
        [TYPE_B] = (static_subrule_t){       // translation-unit external-declaration
            MATCH_TRANSLATION_UNIT,
            MATCH_EXTERNAL_DECLARATION,
            NULL,
        },
        NULL,
    },
    
    // external-declaration
    [MATCH_EXTERNAL_DECLARATION] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_FUNCTION_DEFINITION, NULL },    // function-definition
        [TYPE_B] = (static_subrule_t){ MATCH_DECLARATION,         NULL },    // declaration
        NULL,
    },
    
    // function-definition
    [MATCH_FUNCTION_DEFINITION] = (static_rule_t){
                // attribute-specifier-sequenceopt declaration-specifiers declarator function-body
        [TYPE_A] = (static_subrule_t){
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE | MF_OPTIONAL,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_DECLARATOR,
            MATCH_FUNCTION_BODY,
            NULL,
        },
        NULL,
    },
    
    // function-body
    [MATCH_FUNCTION_BODY] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){ MATCH_COMPOUND_STATEMENT, NULL }, // compound-statement
        NULL,
    },
};


/*

[MATCH_GENERIC] = (static_rule_t){
        [TYPE_A] = (static_subrule_t){       // generic
            MATCH_GENERIC,
            NULL,
        },
        NULL,
    },

*/






const char *const match_type_str[TYPE_Z+1] = {
    [TYPE_A] = "TYPE_A",
    [TYPE_B] = "TYPE_B",
    [TYPE_C] = "TYPE_C",
    [TYPE_D] = "TYPE_D",
    [TYPE_E] = "TYPE_E",
    [TYPE_F] = "TYPE_F",
    [TYPE_G] = "TYPE_G",
    [TYPE_H] = "TYPE_H",
    [TYPE_I] = "TYPE_I",
    [TYPE_J] = "TYPE_J",
    [TYPE_K] = "TYPE_K",
    [TYPE_L] = "TYPE_L",
    [TYPE_M] = "TYPE_M",
    [TYPE_N] = "TYPE_N",
    [TYPE_O] = "TYPE_O",
    [TYPE_P] = "TYPE_P",
    [TYPE_Q] = "TYPE_Q",
    [TYPE_R] = "TYPE_R",
    [TYPE_S] = "TYPE_S",
    [TYPE_T] = "TYPE_T",
    [TYPE_U] = "TYPE_U",
    [TYPE_V] = "TYPE_V",
    [TYPE_W] = "TYPE_W",
    [TYPE_X] = "TYPE_X",
    [TYPE_Y] = "TYPE_Y",
    [TYPE_Z] = "TYPE_Z",
}





const char *const match_rule_str[MATCH_RULE_LEN] = {
    [MATCH_END]                         = NULL,

    ///////////////////////
    [MATCH_IDENTIFIER]                  = "MATCH_IDENTIFIER",
    [MATCH_CONSTANT]                    = "MATCH_CONSTANT",
    [MATCH_STRING_LITERAL]              = "MATCH_STRING_LITERAL",

    // EXPRESSIONS
    [MATCH_PRIMARY_EXPRESSION]          = "MATCH_PRIMARY_EXPRESSION",
    [MATCH_POSTFIX_EXPRESSION]          = "MATCH_POSTFIX_EXPRESSION",
    [MATCH_ARGUMENT_EXPRESSION_LIST]    = "MATCH_ARGUMENT_EXPRESSION_LIST",
    [MATCH_COMPOUND_LITERAL]            = "MATCH_COMPOUND_LITERAL",
    [MATCH_STORAGE_CLASS_SPECIFIERS]    = "MATCH_STORAGE_CLASS_SPECIFIERS",
    [MATCH_UNARY_EXPRESSION]            = "MATCH_UNARY_EXPRESSION",
    [MATCH_UNARY_OPERATOR]              = "MATCH_UNARY_OPERATOR",
    [MATCH_CAST_EXPRESSION]             = "MATCH_CAST_EXPRESSION",
    [MATCH_MULTIPLICATIVE_EXPRESSION]   = "MATCH_MULTIPLICATIVE_EXPRESSION",
    [MATCH_ADDITIVE_EXPRESSION]         = "MATCH_ADDITIVE_EXPRESSION",
    [MATCH_SHIFT_EXPRESSION]            = "MATCH_SHIFT_EXPRESSION",
    [MATCH_RELATIONAL_EXPRESSION]       = "MATCH_RELATIONAL_EXPRESSION",
    [MATCH_EQUALITY_EXPRESSION]         = "MATCH_EQUALITY_EXPRESSION",
    [MATCH_AND_EXPRESSION]              = "MATCH_AND_EXPRESSION",
    [MATCH_EXCLUSIVE_OR_EXPRESSION]     = "MATCH_EXCLUSIVE_OR_EXPRESSION",
    [MATCH_INCLUSIVE_OR_EXPRESSION]     = "MATCH_INCLUSIVE_OR_EXPRESSION",
    [MATCH_LOGICAL_AND_EXPRESSION]      = "MATCH_LOGICAL_AND_EXPRESSION",
    [MATCH_LOGICAL_OR_EXPRESSION]       = "MATCH_LOGICAL_OR_EXPRESSION",
    [MATCH_CONDITIONAL_EXPRESSION]      = "MATCH_CONDITIONAL_EXPRESSION",
    [MATCH_ASSIGNMENT_EXPRESSION]       = "MATCH_ASSIGNMENT_EXPRESSION",
    [MATCH_ASSIGNMENT_OPERATOR]         = "MATCH_ASSIGNMENT_OPERATOR",
    [MATCH_EXPRESSION]                  = "MATCH_EXPRESSION",
    [MATCH_CONSTANT_EXPRESSION]         = "MATCH_CONSTANT_EXPRESSION",

    // DECLARATIONS
    [MATCH_DECLARATION]                 = "MATCH_DECLARATION",
    [MATCH_DECLARATION_SPECIFIER]       = "MATCH_DECLARATION_SPECIFIER",
    [MATCH_DECLARATION_SPECIFIER]       = "MATCH_DECLARATION_SPECIFIER",
    [MATCH_INIT_DECLARATOR_LIST]        = "MATCH_INIT_DECLARATOR_LIST",
    [MATCH_INIT_DECLARATOR]             = "MATCH_INIT_DECLARATOR",
    [MATCH_ATTRIBUTE_DECLARATION]       = "MATCH_ATTRIBUTE_DECLARATION",
    [MATCH_STORAGE_CLASS_SPECIFIER]     = "MATCH_STORAGE_CLASS_SPECIFIER",
    [MATCH_TYPE_SPECIFIER]              = "MATCH_TYPE_SPECIFIER",
    [MATCH_STRUCT_OR_UNION_SPECIFIER]   = "MATCH_STRUCT_OR_UNION_SPECIFIER",
    [MATCH_STRUCT_OR_UNION]             = "MATCH_STRUCT_OR_UNION",
    [MATCH_MEMBER_DECLARATION_LIST]     = "MATCH_MEMBER_DECLARATION_LIST",
    [MATCH_MEMBER_DECLARATION]          = "MATCH_MEMBER_DECLARATION",
    [MATCH_SPECIFIER_QUALIFIER_LIST]    = "MATCH_SPECIFIER_QUALIFIER_LIST",
    [MATCH_TYPE_SPECIFIER_QUALIFIER]    = "MATCH_TYPE_SPECIFIER_QUALIFIER",
    [MATCH_MEMBER_DECLARATOR_LIST]      = "MATCH_MEMBER_DECLARATOR_LIST",
    [MATCH_MEMBER_DECLARATOR]           = "MATCH_MEMBER_DECLARATOR",
    [MATCH_ENUM_SPECIFIER]              = "MATCH_ENUM_SPECIFIER",
    [MATCH_ENUMERATOR_LIST]             = "MATCH_ENUMERATOR_LIST",
    [MATCH_ENUMERATOR]                  = "MATCH_ENUMERATOR",
    [MATCH_ENUM_TYPE_SPECIFIER]         = "MATCH_ENUM_TYPE_SPECIFIER",
    [MATCH_ATOMIC_TYPE_SPECIFIER]       = "MATCH_ATOMIC_TYPE_SPECIFIER",
    [MATCH_TYPEOF_SPECIFIER]            = "MATCH_TYPEOF_SPECIFIER",
    [MATCH_TYPEOF_SPECIFIER_ARGUMENT]   = "MATCH_TYPEOF_SPECIFIER_ARGUMENT",
    [MATCH_TYPE_QUALIFIER]              = "MATCH_TYPE_QUALIFIER",
    [MATCH_FUNCTION_SPECIFIER]          = "MATCH_FUNCTION_SPECIFIER",
    [MATCH_ALIGNMENT_SPECIFIER]         = "MATCH_ALIGNMENT_SPECIFIER",
    [MATCH_DECLARATOR]                  = "MATCH_DECLARATOR",
    [MATCH_DIRECT_DECLARATOR]           = "MATCH_DIRECT_DECLARATOR",
    [MATCH_ARRAY_DECLARATOR]            = "MATCH_ARRAY_DECLARATOR",
    [MATCH_FUNCTION_DECLARATOR]         = "MATCH_FUNCTION_DECLARATOR",
    [MATCH_POINTER]                     = "MATCH_POINTER",
    [MATCH_TYPE_QUALIFIER_LIST]         = "MATCH_TYPE_QUALIFIER_LIST",
    [MATCH_PARAMETER_TYPE_LIST]         = "MATCH_PARAMETER_TYPE_LIST",
    [MATCH_PARAMETER_LIST]              = "MATCH_PARAMETER_LIST",
    [MATCH_PARAMETER_DECLARATION]       = "MATCH_PARAMETER_DECLARATION",
    [MATCH_TYPE_NAME]                   = "MATCH_TYPE_NAME",
    [MATCH_ABSTRACT_DECLARATOR]         = "MATCH_ABSTRACT_DECLARATOR",
    [MATCH_DIRECT_ABSTRACT_DECLARATOR]  = "MATCH_DIRECT_ABSTRACT_DECLARATOR",
    [MATCH_ARRAY_ABSTRACT_DECLARATOR]   = "MATCH_ARRAY_ABSTRACT_DECLARATOR",
    [MATCH_FUNCTION_ABSTRACT_DECLARATOR]= "MATCH_FUNCTION_ABSTRACT_DECLARATOR",
    [MATCH_TYPEDEF_NAME]                = "MATCH_TYPEDEF_NAME",
    [MATCH_BRACED_INITIALIZER]          = "MATCH_BRACED_INITIALIZER",
    [MATCH_INITIALIZER]                 = "MATCH_INITIALIZER",
    [MATCH_INITIALIZER_LIST]            = "MATCH_INITIALIZER_LIST",
    [MATCH_DESIGNATION]                 = "MATCH_DESIGNATION",
    [MATCH_DESIGNATOR_LIST]             = "MATCH_DESIGNATOR_LIST",
    [MATCH_DESIGNATOR]                  = "MATCH_DESIGNATOR",
    [MATCH_STATIC_ASSERT_DECLARATION]   = "MATCH_STATIC_ASSERT_DECLARATION",
    [MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE]= "MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE",
    [MATCH_ATTRIBUTE_SPECIFIER]         = "MATCH_ATTRIBUTE_SPECIFIER",
    [MATCH_ATTRIBUTE_LIST]              = "MATCH_ATTRIBUTE_LIST",
    [MATCH_ATTRIBUTE]                   = "MATCH_ATTRIBUTE",
    [MATCH_ATTRIBUTE_TOKEN]             = "MATCH_ATTRIBUTE_TOKEN",
    [MATCH_STANDARD_ATTRIBUTE]          = "MATCH_STANDARD_ATTRIBUTE",
    [MATCH_ATTRIBUTE_PREFIX_TOKEN]      = "MATCH_ATTRIBUTE_PREFIX_TOKEN",
    [MATCH_ATTRIBUTE_PREFIX]            = "MATCH_ATTRIBUTE_PREFIX",
    [MATCH_ATTRIBUTE_ARGUMENT_CLAUSE]   = "MATCH_ATTRIBUTE_ARGUMENT_CLAUSE",
    [MATCH_BALANCED_TOKEN_SEQUENCE]     = "MATCH_BALANCED_TOKEN_SEQUENCE",
    [MATCH_BALANCED_TOKEN]              = "MATCH_BALANCED_TOKEN",

    // STATEMENTS
    [MATCH_STATEMENT]                   = "MATCH_STATEMENT",
    [MATCH_UNLABELED_STATEMENT]         = "MATCH_UNLABELED_STATEMENT",
    [MATCH_PRIMARY_BLOCK]               = "MATCH_PRIMARY_BLOCK",
    [MATCH_SECONDARY_BLOCK]             = "MATCH_SECONDARY_BLOCK",
    [MATCH_LABEL]                       = "MATCH_LABEL",
    [MATCH_LABELED_STATEMENT]           = "MATCH_LABELED_STATEMENT",
    [MATCH_COMPOUND_STATEMENT]          = "MATCH_COMPOUND_STATEMENT",
    [MATCH_BLOCK_ITEM_LIST]             = "MATCH_BLOCK_ITEM_LIST",
    [MATCH_BLOCK_ITEM]                  = "MATCH_BLOCK_ITEM",
    [MATCH_EXPRESSION_STATEMENT]        = "MATCH_EXPRESSION_STATEMENT",
    [MATCH_SELECTION_STATEMENT]         = "MATCH_SELECTION_STATEMENT",
    [MATCH_ITERATION_STATEMENT]         = "MATCH_ITERATION_STATEMENT",
    [MATCH_JUMP_STATEMENT]              = "MATCH_JUMP_STATEMENT",

    // EXTERNAL DEFINITIONS
    [MATCH_TRANSLATION_UNIT]            = "MATCH_TRANSLATION_UNIT",
    [MATCH_EXTERNAL_DECLARATION]        = "MATCH_EXTERNAL_DECLARATION",
    [MATCH_FUNCTION_DEFINITION]         = "MATCH_FUNCTION_DEFINITION",
    [MATCH_FUNCTION_BODY]               = "MATCH_FUNCTION_BODY",
};


#endif /* #ifdef OLD_CODE_DO_NOT_COMPILE */
