#ifdef OLD_CODE_DO_NOT_COMPILE

/*

Alright, with my lexer done, it is now up to the parser.

I was thinking of just having different format descriptors it would try to match to, as well as a contextual recursive grouping system.

And I could perhaps have subcontext matching that would be able to group the subgroups within a larger whole in order to classify the larger group. And then perhaps the parent group can run recontextualizing functions on the subgroups that were created in order to classify itself. 

So basically, initial subclassification, which leads to parental classification, and then ends with sub-reclassification.


So what should the initial sub-classifier do?
Well, it could start with grouping things. But since grouping can be contextual, it would need to do this incrementally. So we would need to be running a context checker alongside the initial sub-grouper. 
And then when the contextualizer only returns one match, then we finish the subgrouper until the matched statement ends (like a semicolon or a statement)


An alternative is to have the contextualizer run a test grouper while trying to match, and if the test grouper fails, then it realizes it isn't a match.
This actually seems like a much cleaner way to do this. Much easier to organize too.

So lets say for instance we have, 

int tokenize_buffer(token_array_t *restrict array, const char *restrict buff) {...}


So we start the match sequence. We see the int, it is a complete group, move on to the identifier, we don't see it in our list of variables, so it is a new_identifier, move on to the open bracket


extern int tokenize_buffer(token_array_t *restrict array, const char *restrict buff);

So lets say we are testing for a normal variable declaration. 
First of, ignoring the extern because that is easy to handle, as it is unordered, but how do we handle 
sequences with multiple possible orders? Should I just force the programmer to create all possible orderings?
I mean, I might as well. It doesn't sound like it should be a problem for c-like languages.
Though I could potentially add to the descriptor set an open and closer for unordered groups.

Anywya, with those considerations in mind, the function poiner on hand.

Lets say we test for normal variable declaration.

We see an extern. It is unordered and added to the statement state table. Next one. We see an int, which matches the type, we then see a new_identifier that is not currently defined in the current scope, a match. We then see an opening parenthesis. Lets assume normal variable declarations have nothing to do with grouping parenthesis past this point, so it is marked a failure.

Now then we test the function predeclaration descriptor.

int is a match, new_identifier is a match, then we hit the parenthesis and trigger....
Well, I am treating this like I am trying to match the tokens to the descriptor, but really I suppose this should be the other way around.

Test first token for return type. Int. Test next token for new_identifier. tokenize_buffer. Test next token for round bracket enclosed parameter GROUP. Success. Test for semicolon. Success. Done, a full statement processed into a tree.

And of course if we were testing for curly brackets rather than semicolon, then that would be the next check.

Each statement should be treated as a group, I suspect. So the group test function for a type would be it's own function that would test for a type, and it would prod the first and then perhaps the next few until it is certain it has the whole type, and then it will return the group, as well as a pass or fail.



Note that the semicolon is not a universal ending statement, but is instead specifically defined in it's use for every match sequence in the descriptor list. And each match sequence indidivually uses the semicolon to determine when their statement is ended.

Make sure to order statements from easiest to process to hardest to process. There are probably some that would attempt to match to the entire document as a statement before reaching a semicolon.

*/





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






/*

Alright, so how will this descriptor stuff work? We need a descriptor type, and then multiple variations of that type.

*/



// TODO IMPORTANT!!! TODO
// These descriptors are literally all based on these
//############################################################
// https://en.cppreference.com/w/c/language/declarations.html
// https://en.cppreference.com/w/c/language/statements.html
// perhaps a better reference 
// https://port70.net/~nsz/c/c11/n1570.html



#include <stdint.h>



// typedef struct {
//     //int32_t class;
//     //int32_t len;
//     int32_t flags;
//     int32_t *layout;
// } matcher_t;


typedef struct {
    //int32_t class;
    //matcher_t *matchers;
    // I could just have this handler be for stuff that cant be determined
    // by the descriptor, such as tokens or more advanced behavior
    // which can then manually invoke the default class function again.
    int (handler*)();
    uint32_t flags;
    uint32_t **layouts;
} mclass_t;




typedef struct {

    void (*input_handler)(void);    // TODO: Use different function prototype
    void (*output_handler)(void);   // TODO: Use different function prototype

    mclass_t *mtree;

} match_tree_t;




#define NEWMATCH(_class, __literal, __flags) \
    ((matcher_t){.class = (__class), .len = lenof(__literal), .layout = (__literal)}) 


// TODO: 
// for this cast (int32_t *[]), I think rules of precidence dictate the pivot


// int match_specifiers_and_qualifiers(token_t *tokens, size_t len) {
//     for (int i = 0; i < len; i++) {
//         if token()
//     }
//     matcher();
// }



typedef uint32_t match_flags_t;


#define MF_DEFAULT      ( 0x0 )


#define MF_UNORDERED     ( 0x1<<1 ) /* not related to the following two flags */
#define MF_LEFT_TO_RIGHT ( 0x1<<2 )
#define MF_RIGHT_TO_LEFT ( 0x1<<3 )
// grouping now determined by the handler callback
//#define MF_IS_GROUP     ( 0x1<<2 )


//#define MF_FLAG_MASK    ( 0xFFFF0000 )
#define MF_MATCH_MASK   ( 0x0000FFFF )

#define MF_ZERO_OR_ONE  ( 0x1<<16 )
#define MF_ZERO_OR_MORE ( 0x1<<17 )
#define MF_ONE_OR_MORE  ( 0x1<<18 )
#define MF_UTOKEN       ( 0x1<<19 )
#define MF_DTOKEN       ( 0x1<<20 )
#define MF_TOKEN_TYPE   ( 0x1<<21 )
//#define MF_NUMERIC_WILDCARD         ( 0x1<<21 )
//#define MF_ALPHANUMERIC_WILDCARD    ( 0x1<<22 )



// TODO: Order these from easiest to determine/match to hardest
enum {
    MATCH_END = 0,
    MATCH_PRIMARY_EXPRESSION,
    MATCH_DECLARATION,
    MATCH_SPECIFIERS_AND_QUALIFIERS,
    MATCH_DECLARATOR_AND_INITIALIZER,
    MATCH_DECLARATOR_AND_INITIALIZER_LIST,
    MATCH_DECLARATOR,
    MATCH_ATTRIBUTE,
    MATCH_INITIALIZER,
    MATCH_INITIALIZER_LIST,
    MATCH_QUALIFIER,
    
    // make sure expression here is last
    // as it will have 15 different subcategories
    MATCH_EXPRESSION,
};
typedef uint32_t match_class_t;




// TODO:
/*
MATCH_PARAMETER_LIST
MATCH_PARAMETER
MATCH_COMPOUND_LITERAL
MATCH_CAST_TYPE
MATCH_TYPE_SPECIFIER
MATCH_STORAGE_SPECIFIER
MATCH_TYPE_QUALIFIER
MATCH_FUNCTION_SPECIFIER
MATCH_ALIGNMENT_SPECIFIER
MATCH_NO_PTR_DECLARATOR
MATCH_EXPRESSION_LIST
*/



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




// // Order these based off of operator precidence
// enum {
//     OP_NULLARY = 0,
//     OP_PARENTH,
// 
//     // left to right
//     OP_COMMA,
// 
//     // right to left
//     OP_BIT_ASSIGN_AND,
//     OP_BIT_ASSIGN_OR,
//     OP_BIT_ASSIGN_XOR,
//     OP_BIT_ASSIGN_SHR,
//     OP_BIT_ASSIGN_SHL,
// };
// typedef operator_type_t;


// How would I do right to left associativity?
// a = b = c
// I suppose this would be maximum grouping vs minimum grouping.
// I might just be able to add a flag for this.



// TODO:
// Compound literal:            https://port70.net/~nsz/c/c23/n3220.html#6.5.3.6
// Argument expression list:    !https://port70.net/~nsz/c/c11/n1570.html#6.5.2
// Cast expression:             https://port70.net/~nsz/c/c23/n3220.html#6.5.5
// Comma operator:              !https://port70.net/~nsz/c/c11/n1570.html#6.5.17
// Constant expression:         !https://port70.net/~nsz/c/c11/n1570.html#6.6



// Oh wait, it is all compiled here
// https://port70.net/~nsz/c/c23/n3220.html#A.1
// and it looks like it represents all lists with recursion.

// I am actually seriously impressed. The matching list described here
// handles operator precidence and associativity all by itself



mclass_t mclasses[] = {

    // https://port70.net/~nsz/c/c23/n3220.html#A.3.1
    // TODO: Handle expressions later.
    // [MATCH_PRIMARY_EXPRESSION] = { // declares types and variables
    //     .flags = MF_DEFAULT,
    //     .layouts = (uint32_t *[]){
    //         (uint32_t []){ MATCH_IDENTIFIER,        MATCH_END }, // identifier
    //         (uint32_t []){ MATCH_CONSTANT,          MATCH_END }, // constant
    //         (uint32_t []){ MATCH_STRING_LITERAL,    MATCH_END }, // string literal
    //         //(uint32_t []){ MATCH_GENERIC_SELECTION, MATCH_END }, // generic-selection
    //         (uint32_t []){          // ( expression )
    //             UTOKEN_L_RBRACKET | MF_UTOKEN,
    //             MATCH_EXPRESSION, 
    //             MATCH_END
    //         },
    //     },
    // },
    
    // Skipping generic selection for now

    
    
    [MATCH_DECLARATION] = { // declares types and variables
    
        .handler = tbd /*(to be determined)*/,
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){          // attr-spec-seq specifiers-and-qualifiers declarators-and-initializers ;
                MATCH_ATTRIBUTE | MF_ZERO_OR_MORE,
                MATCH_SPECIFIERS_AND_QUALIFIERS,
                MATCH_DECLARATOR_AND_INITIALIZER_LIST,
                UTOKEN_SEMICOLON | MF_UTOKEN,
                MATCH_END,
            },
            (uint32_t []){          // specifiers-and-qualifiers declarators-and-initializers(optional) ;
                MATCH_SPECIFIERS_AND_QUALIFIERS, 
                MATCH_DECLARATOR_AND_INITIALIZER_LIST | MF_ZERO_OR_ONE,
                UTOKEN_SEMICOLON | MF_UTOKEN,
                MATCH_END,
            },
        },
    },
    
    [MATCH_SPECIFIERS_AND_QUALIFIERS] = {

        .flags = MF_UNORDERED,
        .layouts = (uint32_t *[]){
            (uint32_t []){
                MATCH_TYPE_SPECIFIER,
                MATCH_STORAGE_SPECIFIER | MF_ZERO_OR_ONE,
                MATCH_TYPE_QUALIFIER | MF_ZERO_OR_MORE,
                MATCH_FUNCTION_SPECIFIER | MF_ZERO_OR_MORE,
                MATCH_ALIGNMENT_SPECIFIER | MF_ZERO_OR_MORE,
                MATCH_END,
            },
        },
    },

    // I assume function declarator with body is something not included in this, 
    // but as another match type
    [MATCH_DECLARATOR_AND_INITIALIZER_LIST] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){      // infinite recursion fixed here
            (uint32_t []){              // declarator-and-initializer, declarators-and-initializers
                MATCH_DECLARATOR_AND_INITIALIZER,
                UTOKEN_COMMA | MF_UTOKEN,
                MATCH_DECLARATOR_AND_INITIALIZER_LIST,
                MATCH_END,
            },
        },
    },

    [MATCH_DECLARATOR_AND_INITIALIZER] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){      // declarator includes struct definitions in the identifier
            (uint32_t []){              // declarator
                MATCH_DECLARATOR,
                MATCH_END,
            },
            (uint32_t []){              // declarator = initilizer
                MATCH_DECLARATOR,
                UTOKEN_EQUALS | MF_UTOKEN,
                MATCH_INITIALIZER,
                MATCH_END,
            },
        },
    },

    // TODO: Make sure to order these according to order of evaluation
    [MATCH_DECLARATOR] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // identifier attr-spec-seq(optional)
                TOKEN_IDENTIFIER | MF_TOKEN_TYPE,
                //MATCH_IDENTIFIER,
                MATCH_ATTRIBUTE | MF_ZERO_OR_MORE,
                MATCH_END,
            },
            (uint32_t []){              // ( declarator )
                UTOKEN_L_RBRACKET | MF_UTOKEN,
                MATCH_DECLARATOR,
                UTOKEN_R_RBRACKET | MF_UTOKEN,
                MATCH_END,
            },                          // pointer declarator
            (uint32_t []){              // * attr-spec-seq(optional) qualifiers(optional) declarator
                UTOKEN_STAR | MF_UTOKEN,
                MATCH_ATTRIBUTE | MF_ZERO_OR_MORE,
                MATCH_QUALIFIER | MF_ZERO_OR_MORE,
                MATCH_DECLARATOR,
                MATCH_END,
            },                          // array declarator
            (uint32_t []){              // noptr-declarator [ static(optional) qualifiers(optional) expression ]
                MATCH_NO_PTR_DECLARATOR,
                UTOKEN_L_SBRACKET | MF_UTOKEN,
                DTOKEN_STATIC | MF_DTOKEN | MF_ZERO_OR_ONE,
                MATCH_QUALIFIER | MF_ZERO_OR_MORE,
                MATCH_EXPRESSION,
                UTOKEN_R_SBRACKET | MF_UTOKEN,
                MATCH_END,
            },                          // array declarator
            (uint32_t []){              // noptr-declarator [ qualifiers(optional) * ]
                MATCH_NO_PTR_DECLARATOR,
                UTOKEN_L_SBRACKET
                MATCH_QUALIFIER | MF_ZERO_OR_MORE,
                UTOKEN_STAR | MF_UTOKEN,
                UTOKEN_R_SBRACKET
                MATCH_END,
            },                          // function declarator
            (uint32_t []){              // noptr-declarator ( parameters ) attr-spec-seq(optional)
                MATCH_NO_PTR_DECLARATOR,
                UTOKEN_L_RBRACKET,
                MATCH_PARAMETER_LIST,
                UTOKEN_R_RBRACKET,
                MATCH_ATTRIBUTE | MF_ZERO_OR_MORE,
                MATCH_END,
            },                          // function declarator
            (uint32_t []){              // noptr-declarator ( ) attr-spec-seq(optional)
                MATCH_NO_PTR_DECLARATOR,
                UTOKEN_L_RBRACKET,
                MATCH_PARAMETER_LIST,
                UTOKEN_R_RBRACKET,
                MATCH_ATTRIBUTE | MF_ZERO_OR_MORE,
                MATCH_END,
            },
        },
    },

    [MATCH_ATTRIBUTE] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // __attribute__ (( attribute ))
                DTOKEN_ATTRIBUTE | MF_DTOKEN,
                UTOKEN_L_RBRACKET | MF_UTOKEN,
                UTOKEN_L_RBRACKET | MF_UTOKEN,
                TOKEN_IDENTIFIER | MF_TOKEN_TYPE, // not actually identifier. Just alphanumeric
                UTOKEN_R_RBRACKET | MF_UTOKEN,
                UTOKEN_R_RBRACKET | MF_UTOKEN,
                MATCH_END,
            },
        },
    },

    [MATCH_INITIALIZER] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression
                MATCH_EXPRESSION,       // an expression may be a compound literal
                MATCH_END,
            },
            (uint32_t []){              // { initilizer-list } or { }
                UTOKEN_L_CBRACKET | MF_UTOKEN,
                MATCH_INITIALIZER_LIST | MF_OPTIONAL
                UTOKEN_R_CBRACKET | MF_UTOKEN,
                MATCH_END,
            },
        },
    },

    // This avoids infinite recursion because it checks for an initilizer first
    // before the recursive check for initilizer-list
    // thank goodness the previous MATCH_INITILIZER has curly brackets before
    // checking for initilizer-list, otherwise it would result in infinite 
    // recursion again.
    [MATCH_INITIALIZER_LIST] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // designator-list = initializer
                MATCH_INITIALIZER,
                MATCH_END
            },                          // this should go first so that it matches first
            (uint32_t []){              // initiliazer, initiliazer-list
                MATCH_EXPRESSION,
                UTOKEN_COMMA | MF_UTOKEN,
                MATCH_INITIALIZER_LIST,
                MATCH_END,
            },
            (uint32_t []){              // initiliazer
                MATCH_INITIALIZER,
                MATCH_END
            },
        },
    },

    [MATCH_QUALIFIER] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // const
                DTOKEN_CONST,
                MATCH_END,
            },
            (uint32_t []){              // volatile
                DTOKEN_VOLATILE,
                MATCH_END,
            },
            (uint32_t []){              // restrict
                DTOKEN_RESTRICT,
                MATCH_END,
            },
            (uint32_t []){              // _Atomic
                DTOKEN_ATOMIC,
                MATCH_END,
            },
        },
    },

    // TODO: order of operations may actually be achievable with this
    // instead of `op binary-operator op`, which will match for any operator
    // we can instead do `op * op` which will search for that sequence first
    // with early termination if no match for a valid operand.
    // 1 + 2 * 3 + 4 * 5
    // here the multiplication check it will fail on 1 + 2, because 1 is a valid expression.
    // so in order for this to work, we would need to keep prodding for the check until we
    // reach a mismatch. But incomplete matches will simply end the prod early and start with the 
    // next prod.
    // I suspect we would need to make this the default behavior for the matcher. Because otherwise
    // it would always abort too early.
    // but now the real question, should we go for max prod, or min prod.
    // also operators with higher precidence should do less grouping, so plus would actually
    // be checked before multiplication.

    // TODO: alright, slightly new plan due to associativity
    // instead we choose prod sizes first, and for each prod size we check each layout.
    // this way we can group operators by precidence, as well as rely on associativity within
    // precedence groups where operators all have the same precidence.
    // and the associativity direction is determined by matching with either the biggest prod group
    // or with the smallest prod group.
    // Actually, only do this if associativity is set. Otherwise we do it the original way to
    // ensure pattern matches are done in a specific order, which will help speed up recursion
    // and other things.
    // Also it will make precedence ordering possible still.
    // Yeah, so left-to-right and right-to-left processes in order of associativity
    // whereas the default will process in order of precedence, or order in the list.

    // TODO: Also, should primary expressions go last?
    // I guess it might not matter since they are nullary expressions
    // So it might just make matching faster to put them up front

    // TODO: I need to make sure that an expression alone will try to select the smallest
    // working expression for operators such as sizeof.
    // Perhaps to do this we would try to either find the smallest left hand match for
    // default or left-to-right, or we would find the biggest left-hand match that 
    // results in the smallest right-hand match for right-to-left matches.

    // also assume casts are forms of expressions.
    [MATCH_EXPRESSION] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // ( expression )
                UTOKEN_L_RBRACKET | MF_UTOKEN,
                MATCH_EXPRESSION,
                UTOKEN_L_RBRACKET | MF_UTOKEN,
                MATCH_END,
            },
            (uint32_t []){              // numeric
                TOKEN_NUMERIC | MF_TOKEN_TYPE,
                MATCH_END,
            },
            (uint32_t []){              // identifier
                TOKEN_IDENTIFIER | MF_TOKEN_TYPE,
                MATCH_END,
            },
            (uint32_t []){ MATCH_EXPRESSION + 15, MATCH_END, }, // Precedence 15
            (uint32_t []){ MATCH_EXPRESSION + 14, MATCH_END, }, // Precedence 14
            (uint32_t []){ MATCH_EXPRESSION + 13, MATCH_END, }, // Precedence 13
            (uint32_t []){ MATCH_EXPRESSION + 12, MATCH_END, }, // Precedence 12
            (uint32_t []){ MATCH_EXPRESSION + 11, MATCH_END, }, // Precedence 11
            (uint32_t []){ MATCH_EXPRESSION + 10, MATCH_END, }, // Precedence 10
            (uint32_t []){ MATCH_EXPRESSION +  9, MATCH_END, }, // Precedence 9
            (uint32_t []){ MATCH_EXPRESSION +  8, MATCH_END, }, // Precedence 8
            (uint32_t []){ MATCH_EXPRESSION +  7, MATCH_END, }, // Precedence 7
            (uint32_t []){ MATCH_EXPRESSION +  6, MATCH_END, }, // Precedence 6
            (uint32_t []){ MATCH_EXPRESSION +  5, MATCH_END, }, // Precedence 5
            (uint32_t []){ MATCH_EXPRESSION +  4, MATCH_END, }, // Precedence 4
            (uint32_t []){ MATCH_EXPRESSION +  3, MATCH_END, }, // Precedence 3
            (uint32_t []){ MATCH_EXPRESSION +  2, MATCH_END, }, // Precedence 2
            (uint32_t []){ MATCH_EXPRESSION +  1, MATCH_END, }, // Precedence 1
        },
    },

    // remember, a cast is still an operation, so a variable is generalized to an expression
    // which means that dereferencing and member access are effectively ordinary operators
    [MATCH_EXPRESSION + 1] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression ++
                MATCH_EXPRESSION,
                UTOKEN_INCREMENT | MF_UTOKEN,
                MATCH_END,
            },
            (uint32_t []){              // expression --
                MATCH_EXPRESSION,
                UTOKEN_DECREMENT | MF_UTOKEN,
                MATCH_END,
            },                          // function call
            (uint32_t []){              // expression ( expression-list )
                MATCH_EXPRESSION,
                UTOKEN_L_RBRACKET | MF_UTOKEN,
                MATCH_EXPRESSION_LIST,
                UTOKEN_R_RBRACKET | MF_UTOKEN,
                MATCH_END,
            },
            (uint32_t []){              // expression [ expression ]
                MATCH_EXPRESSION,
                UTOKEN_L_SBRACKET | MF_UTOKEN,
                MATCH_EXPRESSION,
                UTOKEN_R_SBRACKET | MF_UTOKEN,
                MATCH_END,
            },
            (uint32_t []){              // expression . expression
                MATCH_EXPRESSION,
                UTOKEN_PERIOD | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression -> expression
                MATCH_EXPRESSION,
                UTOKEN_R_ARROW | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // compound-literal
                MATCH_COMPOUND_LITERAL,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 2] = {
        .flags = MF_RIGHT_TO_LEFT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // ++ expression
                UTOKEN_INCREMENT | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // -- expression
                UTOKEN_DECREMENT | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // + expression
                UTOKEN_PLUS | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // - expression
                UTOKEN_MINUS | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // ! expression
                UTOKEN_EXCLAIM | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // ~ expression
                UTOKEN_TILDE | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },                          // type cast
                                        // TODO: cast-type must be a scalar type or void
            (uint32_t []){              // ( cast-type ) expression
                UTOKEN_L_RBRACKET | MF_UTOKEN,
                MATCH_CAST_TYPE,
                UTOKEN_R_RBRACKET | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // * expression
                UTOKEN_STAR | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // & expression
                UTOKEN_AND | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // sizeof expression
                DTOKEN_SIZEOF | MF_DTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // alignof expression
                DTOKEN_ALIGNOF | MF_DTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 3] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression * expression
                MATCH_EXPRESSION,
                UTOKEN_STAR | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression / expression
                MATCH_EXPRESSION,
                UTOKEN_FSLASH | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression % expression
                MATCH_EXPRESSION,
                UTOKEN_PERCENT | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 4] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression + expression
                MATCH_EXPRESSION,
                UTOKEN_PLUS | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression - expression
                MATCH_EXPRESSION,
                UTOKEN_MINUS | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 5] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression << expression
                MATCH_EXPRESSION,
                UTOKEN_LSHIFT | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression >> expression
                MATCH_EXPRESSION,
                UTOKEN_RSHIFT | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 6] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression > expression
                MATCH_EXPRESSION,
                UTOKEN_R_ABRACKET | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression < expression
                MATCH_EXPRESSION,
                UTOKEN_L_ABRACKET | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression >= expression
                MATCH_EXPRESSION,
                UTOKEN_GEQUAL | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression <= expression
                MATCH_EXPRESSION,
                UTOKEN_LEQUAL | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 7] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression == expression
                MATCH_EXPRESSION,
                UTOKEN_EQUALS | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression != expression
                MATCH_EXPRESSION,
                UTOKEN_NEQUALS | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 8] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression & expression
                MATCH_EXPRESSION,
                UTOKEN_AND | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 9] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression ^ expression
                MATCH_EXPRESSION,
                UTOKEN_XOR | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 10] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression | expression
                MATCH_EXPRESSION,
                UTOKEN_OR | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 11] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression && expression
                MATCH_EXPRESSION,
                UTOKEN_LAND | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 12] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression || expression
                MATCH_EXPRESSION,
                UTOKEN_LOR | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 13] = {
        .flags = MF_RIGHT_TO_LEFT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression ? expression : expression
                MATCH_EXPRESSION,
                UTOKEN_QUESTION | MF_UTOKEN,
                MATCH_EXPRESSION,
                UTOKEN_COLON | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 14] = {
        .flags = MF_RIGHT_TO_LEFT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression = expression
                MATCH_EXPRESSION,
                UTOKEN_ASSIGN | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression += expression
                MATCH_EXPRESSION,
                UTOKEN_ADD_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression -= expression
                MATCH_EXPRESSION,
                UTOKEN_SUB_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression *= expression
                MATCH_EXPRESSION,
                UTOKEN_MUL_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression /= expression
                MATCH_EXPRESSION,
                UTOKEN_DIV_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression %= expression
                MATCH_EXPRESSION,
                UTOKEN_MOD_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression <<= expression
                MATCH_EXPRESSION,
                UTOKEN_LSHIFT_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression >>= expression
                MATCH_EXPRESSION,
                UTOKEN_RSHIFT_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression &= expression
                MATCH_EXPRESSION,
                UTOKEN_AND_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression ^= expression
                MATCH_EXPRESSION,
                UTOKEN_XOR_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
            (uint32_t []){              // expression |= expression
                MATCH_EXPRESSION,
                UTOKEN_OR_EQ | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },
    
    [MATCH_EXPRESSION + 15] = {
        .flags = MF_LEFT_TO_RIGHT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // expression , expression
                MATCH_EXPRESSION,
                UTOKEN_COMMA | MF_UTOKEN,
                MATCH_EXPRESSION,
                MATCH_END,
            },
        },
    },


    // TODO: move everything below this above exressions

    [MATCH_PARAMETER_LIST] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // void
                DTOKEN_VOID | MF_DTOKEN,
                MATCH_END,
            },
            (uint32_t []){              // void
                DTOKEN_VOID | MF_DTOKEN,
                MATCH_END,
            },
        },
    },

    [MATCH_PARAMETER] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // specifiers-and-qualifiers
                MATCH_SPECIFIERS_AND_QUALIFIERS,
                MATCH_END,
            },
            (uint32_t []){              // specifiers-and-qualifiers declarator
                MATCH_SPECIFIERS_AND_QUALIFIERS,
                MATCH_END,
            },
        },
    },
};





/* example matcher class

    [MATCH_NIL] = {
        .flags = MF_DEFAULT,
        .layouts = (uint32_t *[]){
            (uint32_t []){              // null format
                MATCH_NIL,
                MATCH_END,
            },
        },
    },


*/




/*| MF_ONE_OR_MORE_COMMA_SEPARATED,*/



    // Just search for token identifiers instead of this.
    // [MATCH_IDENTIFIER] = {
    //     .flags = MF_IS_GROUP,
    //     .layouts = (uint32_t *[]){
    //         (uint32_t []){              // alphanumeric-wildcard (includes underscore)
    //             TOKEN_IDENTIFIER | MF_TOKEN_TYPE,
    //             MATCH_END,
    //         },
    //     },
    // },


#endif /* #ifdef OLD_CODE_DO_NOT_COMPILE */
