

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





/*

Alright, so how will this descriptor stuff work? We need a descriptor type, and then multiple variations of that type.

*/




// https://en.cppreference.com/w/c/language/declarations.html
// https://en.cppreference.com/w/c/language/statements.html#Iteration_statements



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
    int32_t flags;
    int32_t **layouts;
} mclass_t;



#define NEWMATCH(_class, __literal, __flags) \
    ((matcher_t){.class = (__class), .len = lenof(__literal), .layout = (__literal)}) 


// TODO: 
// for this cast (int32_t *[]), I think rules of precidence dictate the pivot


int match_specifiers_and_qualifiers(token_t *tokens, size_t len) {
    for (int i = 0; i < len; i++) {
        if token()
    }
    matcher()
}



mclass_t mclasses[] = {

    [MATCH_DECLARATION] = {
    
        .handler = tbd /*(to be determined)*/,
        .flags = MF_DEFAULT,
        .layouts = (int32_t *[]){
            (int32_t []){
                MATCH_ATTRIBUTES,
                MATCH_SPECIFIERS_AND_QUALIFIERS,
                MATCH_DECLARATORS_AND_INITIALIZERS,
                MATCH_SEMICOLON,
                MATCH_END,
            },
            (int32_t []){
                MATCH_SPECIFIERS_AND_QUALIFIERS, 
                MATCH_DECLARATORS_AND_INITIALIZERS,
                MATCH_SEMICOLON,
                MATCH_END,
            },
            (int32_t []){
                MATCH_SPECIFIERS_AND_QUALIFIERS, 
                MATCH_SEMICOLON,
                MATCH_END,
            },
        },
    },
    
    [MATCH_SPECIFIERS_AND_QUALIFIERS] = { // one or more

        .flags = MF_UNORDERED,
        .layouts = (int32_t *[]){
            (int32_t []){
                MATCH_SPECIFIERS_AND_QUALIFIERS_OPT,    // zero or more
                MATCH_TYPE_SPECIFIER,   // only one
                MATCH_END,
            },
        },
    },
    
    [MATCH_SPECIFIERS_AND_QUALIFIERS_OPT] = {   // zero or more

        .flags = MF_UNORDERED | MF_ZERO_OR_MORE_SPACE_SEPARATED,
        .layouts = (int32_t *[]){
            (int32_t []){
                MATCH_STORAGE_SPECIFIERS,
                MATCH_TYPE_QUALIFIER,
                MATCH_END,
            },
        },
    },
    {
        .class = MATCH_DECLARATORS_AND_INITIALIZERS,
        .flags = MF_ONE_OR_MORE_COMMA_SEPARATED,
        .layouts = (int32_t *[]){
            (int32_t []){
            },
        },
    },
};
