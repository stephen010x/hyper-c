
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
        [TYPE_A] = (uint32_t []){ TOKEN_IDENTIFIER | MF_TOKEN_TYPE, MATCH_END },    // identifier
    },
    
    [MATCH_CONSTANT] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ TOKEN_NUMERIC | MF_TOKEN_TYPE, MATCH_END },       // constant
    },
    
    [MATCH_STRING_LITERAL] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ TOKEN_STRING | MF_TOKEN_TYPE, MATCH_END },        // string-literal
    },

    // primary-expression
    [MATCH_PRIMARY_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ MATCH_IDENTIFIER, MATCH_END },        // identifier
        [TYPE_B] = (uint32_t []){ MATCH_CONSTANT, MATCH_END },          // constant
        [TYPE_C] = (uint32_t []){ MATCH_STRING_LITERAL, MATCH_END },    // string-literal
        [TYPE_D] = (uint32_t []){
            UTOKEN_L_RBRACKET | MF_UTOKEN,                              // ( expression )
            MATCH_IDENTIFIER,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_END,
        },
    },

    // postfix-expression
    [MATCH_POSTFIX_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // primary-expression
            MATCH_PRIMARY_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // postfix-expression [ expression ]
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_L_SBRACKET | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_R_SBRACKET | MF_UTOKEN,
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // postfix-expression ( argument-expression-listopt )
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_ARGUMENT_EXRESSION_LIST | MF_OPTIONAL,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_END,
        },
        [TYPE_D] = (uint32_t []){       // postfix-expression . identifier
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_PERIOD | MF_UTOKEN,
            MATCH_IDENTIFIER,
            MATCH_END,
        },
        [TYPE_E] = (uint32_t []){       // postfix-expression -> identifier
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_R_ARROW | MF_UTOKEN,
            MATCH_IDENTIFIER,
            MATCH_END,
        },
        [TYPE_F] = (uint32_t []){       // postfix-expression ++
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_INCREMENT | MF_UTOKEN,
            MATCH_END,
        },
        [TYPE_G] = (uint32_t []){       // postfix-expression --
            MATCH_POSTFIX_EXPRESSION,
            UTOKEN_DECREMENT | MF_UTOKEN,
            MATCH_END,
        },
        [TYPE_H] = (uint32_t []){       // compound-literal
            MATCH_COMPOUND_LITERAL,
            MATCH_END,
        },
    },

    // argument-expression-list
    [MATCH_ARGUMENT_EXPRESSION_LIST] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // assignment-expression
            MATCH_ASSIGNMENT_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // argument-expression-list , assignment-expression
            MATCH_EXRESSION_LIST,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            MATCH_END,
        },
    },

    // compound-literal
    [MATCH_COMPOUND_LITERAL] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // ( storage-class-specifiersopt type-name ) braced-initializer
            UTOKEN_L_RBRACKET | MF_UTOKEN,
            MATCH_STORAGE_CLASS_SPECIFIERS | MF_OPTIONAL,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_UTOKEN,
            MATCH_BRACED_INITIALIZER,
            MATCH_END,
        },
    },
    
    // storage-class-specifiers
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // storage-class-specifier
            MATCH_STORAGE_CLASS_SPECIFIER,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // storage-class-specifiers storage-class-specifier
            MATCH_STORAGE_CLASS_SPECIFIERS,
            MATCH_STORAGE_CLASS_SPECIFIER,
            MATCH_END,
        },
    },

    // unary-expression
    [MATCH_UNARY_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // postfix-expression
            MATCH_POSTFIX_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // ++ unary-expression
            UTOKEN_INCREMENT | MF_UTOKEN
            MATCH_UNARY_EXPRESSION,
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // -- unary-expression
            UTOKEN_DECREMENT | MF_UTOKEN
            MATCH_GENERIC,
            MATCH_END,
        },
        [TYPE_D] = (uint32_t []){       // unary-operator cast-expression
            MATCH_UNARY_OPERATOR,
            MATCH_CAST_EXPRESSION,
            MATCH_END,
        },
        [TYPE_E] = (uint32_t []){       // sizeof unary-expression
            DTOKEN_SIZEOF | MF_DTOKEN,
            MATCH_UNARY_EXPRESSION,
            MATCH_END,
        },
        [TYPE_F] = (uint32_t []){       // sizeof ( type-name )
            DTOKEN_SIZEOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            MATCH_END,
        },
        [TYPE_G] = (uint32_t []){       // alignof ( type-name )
            DTOKEN_ALIGNOF | MF_DTOKEN,
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            MATCH_END,
        },
    },

    // unary-operator
    [MATCH_UNARY_OPERATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ UTOKEN_AND     | MF_UTOKEN, MATCH_END }, // &
        [TYPE_B] = (uint32_t []){ UTOKEN_STAR    | MF_UTOKEN, MATCH_END }, // *
        [TYPE_C] = (uint32_t []){ UTOKEN_PLUS    | MF_UTOKEN, MATCH_END }, // +
        [TYPE_D] = (uint32_t []){ UTOKEN_MINUS   | MF_UTOKEN, MATCH_END }, // -
        [TYPE_E] = (uint32_t []){ UTOKEN_TILDE   | MF_UTOKEN, MATCH_END }, // ~
        [TYPE_F] = (uint32_t []){ UTOKEN_EXCLAIM | MF_UTOKEN, MATCH_END }, // !
    },

    // cast-expression
    [MATCH_CAST_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // unary-expression
            MATCH_UNARY_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // ( type-name ) cast-expression
            UTOKEN_L_RBRACKET | MF_DTOKEN,
            MATCH_TYPE_NAME,
            UTOKEN_R_RBRACKET | MF_DTOKEN,
            MATCH_CAST_EXPRESSION,
            MATCH_END,
        },
    },

    // multiplicative-expression
    [MATCH_MULTIPLICATIVE_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // cast-expression
            MATCH_CAST_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // multiplicative-expression * cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_STAR | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // multiplicative-expression / cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_FSLASH | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            MATCH_END,
        },
        [TYPE_D] = (uint32_t []){       // multiplicative-expression % cast-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            UTOKEN_PERCENT | MF_UTOKEN,
            MATCH_CAST_EXPRESSION,
            MATCH_END,
        },
    },

    // additive-expression
    [MATCH_ADDITIVE_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // multiplicative-expression
            MATCH_MULTIPLICATIVE_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // additive-expression + multiplicative-expression
            MATCH_ADDITIVE_EXPRESSION,
            UTOKEN_PLUS | MF_UTOKEN,
            MATCH_MULTIPLICATIVE_EXPRESSION
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // additive-expression - multiplicative-expression
            MATCH_ADDITIVE_EXPRESSION,
            UTOKEN_MINUS | MF_UTOKEN,
            MATCH_MULTIPLICATIVE_EXPRESSION
            MATCH_END,
        },
    },
    
    // shift-expression
    [MATCH_SHIFT_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // additive-expression
            MATCH_ADDITIVE_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // shift-expression << additive-expression
            MATCH_SHIFT_EXPRESSION,
            UTOKEN_LSHIFT | MF_UTOKEN,
            MATCH_ADDITIVE_EXPRESSION
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // shift-expression >> additive-expression
            MATCH_SHIFT_EXPRESSION,
            UTOKEN_RSHIFT | MF_UTOKEN,
            MATCH_ADDITIVE_EXPRESSION
            MATCH_END,
        },
    },
    
    // relational-expression
    [MATCH_RELATIONAL_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // shift-expression
            MATCH_SHIFT_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // relational-expression < shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_L_ABRACKET | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // relational-expression > shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_R_ABRACKET | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            MATCH_END,
        },
        [TYPE_D] = (uint32_t []){       // relational-expression <= shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_LEQUAL | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            MATCH_END,
        },
        [TYPE_E] = (uint32_t []){       // relational-expression >= shift-expression
            MATCH_RELATIONAL_EXPRESSION,
            UTOKEN_GEQUAL | MF_UTOKEN,
            MATCH_SHIFT_EXPRESSION
            MATCH_END,
        },
    },
    
    // equality-expression
    [MATCH_EQUALITY_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // relational-expression
            MATCH_RELATIONAL_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // equality-expression == relational-expression
            MATCH_EQUALITY_EXPRESSION,
            UTOKEN_EQUALS | MF_UTOKEN,
            MATCH_RELATIONAL_EXPRESSION
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // equality-expression != relational-expression
            MATCH_EQUALITY_EXPRESSION,
            UTOKEN_NEQUALS | MF_UTOKEN,
            MATCH_RELATIONAL_EXPRESSION
            MATCH_END,
        },
    },
    
    // AND-expression
    [MATCH_AND_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // equality-expression
            MATCH_EQUALITY_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // AND-expression & equality-expression
            MATCH_AND_EXPRESSION,
            UTOKEN_AND | MF_UTOKEN,
            MATCH_EQUALITY_EXPRESSION,
            MATCH_END,
        },
    },
    
    // exclusive-OR-expression
    [MATCH_EXCLUSIVE_OR_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // AND-expression
            MATCH_AND_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // exclusive-OR-expression ^ AND-expression
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            UTOKEN_XOR | MF_UTOKEN,
            MATCH_AND_EXPRESSION,
            MATCH_END,
        },
    },
    
    // inclusive-OR-expression
    [MATCH_INCLUSIVE_OR_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // exclusive-OR-expression
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // inclusive-OR-expression | exclusive-OR-expression
            MATCH_INCLUSIVE_OR_EXPRESSION,
            UTOKEN_OR | MF_UTOKEN,
            MATCH_EXCLUSIVE_OR_EXPRESSION,
            MATCH_END,
        },
    },
    
    // logical-AND-expression
    [MATCH_LOGICAL_AND_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // inclusive-OR-expression
            MATCH_INCLUSIVE_OR_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // logical-AND-expression && inclusive-OR-expression
            MATCH_LOGICAL_AND_EXPRESSION,
            UTOKEN_OR | MF_UTOKEN,
            MATCH_INCLUSIVE_OR_EXPRESSION,
            MATCH_END,
        },
    },
    
    // logical-OR-expression
    [MATCH_LOGICAL_OR_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // logical-AND-expression
            MATCH_LOGICAL_AND_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // logical-OR-expression || logical-AND-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            UTOKEN_LOR | MF_UTOKEN,
            MATCH_LOGICAL_AND_EXPRESSION,
            MATCH_END,
        },
    },
    
    // conditional-expression
    [MATCH_CONDITIONAL_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // logical-OR-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // logical-OR-expression ? expression : conditional-expression
            MATCH_LOGICAL_OR_EXPRESSION,
            UTOKEN_QUESTION | MF_UTOKEN,
            MATCH_EXPRESSION,
            UTOKEN_COLON | MF_UTOKEN,
            MATCH_CONDITIONAL_EXPRESSION,
            MATCH_END,
        },
    },

    // assignment-expression
    [MATCH_ASSIGNMENT_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // conditional-expression
            MATCH_CONDITIONAL_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // unary-expression assignment-operator assignment-expression
            MATCH_UNARY_EXPRESSION,
            MATCH_ASSIGNMENT_OPERATOR,
            MATCH_ASSIGNMENT_EXPRESSION,
            MATCH_END,
        },
    },
    
    // assignment-operator
    [MATCH_ASSIGNMENT_OPERATOR] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){ UTOKEN_ASSIGN | MF_UTOKEN, MATCH_END }, // =
        [TYPE_B] = (uint32_t []){ UTOKEN_MUL_EQ | MF_UTOKEN, MATCH_END }, // *=
        [TYPE_C] = (uint32_t []){ UTOKEN_DIV_EQ | MF_UTOKEN, MATCH_END }, // /=
        [TYPE_D] = (uint32_t []){ UTOKEN_MOD_EQ | MF_UTOKEN, MATCH_END }, // %=
        [TYPE_E] = (uint32_t []){ UTOKEN_ADD_EQ | MF_UTOKEN, MATCH_END }, // +=
        [TYPE_I] = (uint32_t []){ UTOKEN_AND_EQ | MF_UTOKEN, MATCH_END }, // &=
        [TYPE_J] = (uint32_t []){ UTOKEN_XOR_EQ | MF_UTOKEN, MATCH_END }, // ^=
        [TYPE_K] = (uint32_t []){ UTOKEN_OR_EQ  | MF_UTOKEN, MATCH_END }, // |=
        [TYPE_F] = (uint32_t []){ UTOKEN_SUB_EQ | MF_UTOKEN, MATCH_END }, // -=
        [TYPE_G] = (uint32_t []){ UTOKEN_LSHIFT_EQ | MF_UTOKEN, MATCH_END }, // <<=
        [TYPE_H] = (uint32_t []){ UTOKEN_RSHIFT_EQ | MF_UTOKEN, MATCH_END }, // >>=
    },
    
    // expression
    [MATCH_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // assignment-expression
            MATCH_ASSIGNMENT_EXPRESSION,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){       // expression , assignment-expression
            MATCH_EXPRESSION,
            UTOKEN_COMMA | MF_UTOKEN,
            MATCH_ASSIGNMENT_EXPRESSION,
            MATCH_END,
        },
    },
    
    // constant-expression
    [MATCH_CONSTANT_EXPRESSION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // conditional-expression
            MATCH_CONDITIONAL_EXPRESSION,
            MATCH_END,
        },
    },


    //////////////////////////////////////////////////////
    //            DECLARATIONS                          ##
    //####################################################

    // declaration
    [MATCH_DECLARATION] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // declaration-specifiers init-declarator-listopt ;
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_INIT_DECLARATOR_LIST | MF_OPTIONAL,
            MATCH_END,
        },
        [TYPE_B] = (uint32_t []){ // attribute-specifier-sequence declaration-specifiers init-declarator-list ;
            MATCH_ATTRIBUTE_SPECIFIER_SEQUENCE,
            MATCH_DECLARATION_SPECIFIERS,
            MATCH_INIT_DECLARATOR_LIST,
            MATCH_END,
        },
        [TYPE_C] = (uint32_t []){       // static_assert-declaration
            MATCH_GENERIC,
            MATCH_END,
        },
        [TYPE_D] = (uint32_t []){       // attribute-declaration
            MATCH_GENERIC,
            MATCH_END,
        },
    },

    // declaration-specifiers
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            MATCH_END,
        },
    },

    // declaration-specifier
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            MATCH_END,
        },
    },

    // init-declarator-list
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            MATCH_END,
        },
    },

    // init-declarator
    [MATCH_GENERIC] = (uint32_t *[]){
        [TYPE_A] = (uint32_t []){       // generic
            MATCH_GENERIC,
            MATCH_END,
        },
    },

    // attribute-declaration

    // storage-class-specifier

    // type-specifier

    // struct-or-union-specifier

    // struct-or-union

    // member-declaration-list

    // member-declaration

    // specifier-qualifier-list

    // type-specifier-qualifier

    // member-declarator-list

    // member-declarator

    // enum-specifier

    // enumerator-list

    // enumerator

    // enum-type-specifier

    // atomic-type-specifier

    // typeof-specifier

    // typeof-specifier-argument

    // type-qualifier

    // function-specifier

    // alignment-specifier

    // declarator

    // direct-declarator

    // array-declarator

    // function-declarator

    // pointer

    // type-qualifier-list

    // parameter-type-list

    // parameter-list

    // parameter-declaration

    // type-name

    // abstract-declarator

    // direct-abstract-declarator

    // array-abstract-declarator

    // function-abstract-declarator

    // typedef-name

    // braced-initializer

    // initializer

    // initializer-list

    // designation

    // designator-list

    // designator

    // static_assert-declaration

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
            MATCH_END,
        },
    },

*/
