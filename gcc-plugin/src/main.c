// Reference
// https://en.cppreference.com/w/c/language.html


#include <stdio.h>
#include <stbool.h>
#include <string.h>


#include "hyper-c.h"




const char *test_code = "

";


/*
	- Keywords
	- Identifiers
	- Literals
	- Operators (e.g., `+`, `-`, `*`, `/`, etc.)
	- Punctuation (e.g., `;`, `,`, `()`, `{}`, etc.)
*/


/*
    Alright. This is how I think tokens can be parsed.
    First, we see if it matches one of the special strings that can be recognized without delimiters.
    Next we apply delimiters, including the special string matchers as delimiters, in order to extract a token.
    Perhaps also add some reserved tokens as well
    And then we process the token to see what kind of token it is, if it wasn't already recognized.
*/


// maybe instead of assigning token groups, I should just assign token names.




// NOTE: in the test code, pay attention to all the spaces that can be deleted, and how that kind of
// group parsing might work. For instance,
// int temp = *x;
// int temp=*x;





// keep track of:
//  - program state
//  - state changes

int main(int argc, char *argv[]) {

    state_t state;
    state_init(&state, test_code);

    parse_scope_recursive(&state);

    return 0;
}


/*
    So statements should have sub-statements. Because otherwise a function will all be considered
    One giant statement, and will easily exceed the token limit.
    So instead I should create a token type that points to another statement.
    Generally speaking, there should be no co-dependancy between each statement that 
    isn't covered by the state machine structure.

    And then in special cases like switch case, I can just process them independantly, and then
    When it comes to compilation, I can just have the switch search the sub-statement for the cases.
    Alternatively I can make it a flag in the global state, so that it can be done with the cases
    Rather than the switch.
    This way we can also just process assembly as we go rather than at the end of tokenizing the function.
*/





void state_init(state_t *state, char *source) {
    *state = (state_t){
        .buffer = source;
        .blen = 0;
        .root_scope = NULL;
        .scope = NULL;
    };
}



//void tokenize_statement(state_t *state, 




int parse_scope_recursive(state_t *state) {

    char *buffer;
    int   blen;
    char *c;
    scope_state_t scope;
    int retval = 0;


    // some qol setup
    buffer = c = state->buffer;
    blen   = 0;


    // set up new scope state
    scope = (scope_state_t){
        .parent = state->scope;
    };

    state->scope = &scope;
    

    // If root scope is null, then set the current scope to root scope
    if (state->root_scope == NULL)
        state->root_scope = &scope;
    
    
    for(;;) {
    
        // some qol setup
        c++;
        blen++;

        // string safety check
        if (*c == '\0') {
            retval = -1;
            break;
        }

        // we are going to parse this token by token
        // so we will basically loop until we hit a token delimiter.
        switch

        // ignore comments

        // ignore leading whitespace
        
        // ignore preprocessor statements
    }


    // restore previous scope
    state->scope = scope.parent;

    return retval;
}



// TODO: optimize this by creating a whitespace table that is 256 characters
// long, and you simply index the character into a bool table to determine if 
// it is whitespace. It will be much faster, and can be compressed into 32 bytes
// because they are all bools. Though the fastest will be uncompressed if you
// want to sacrifice 256 bytes of text memory.
/*bool is_whitespace(char c) {
    for (int i = 0; i < lenof(whitespace); i++)
        if (c == whitespace[i])
            return true;
    return false;
}*/


/*typedef int (*match_call_t)(const char *restrict word, const char *restrict sentence, int max);

typedef typeof(*(match_call_t)(NULL)) match_call_enforce_t;

match_call_enforce_t match_utoken;
match_call_enforce_t match_dtoken;*/


// returns length
__inline__ int match_utoken(const char *restrict word, const char *restrict sentence, int max) {
    int i;
    for (i = 0; (word[i] != '\0') && (i < max); i++)
        if ((word[i] != sentence[i]) || (sentence[i] == '\0'))
            return 0;
    return i;
}

// returns length
// replace with simple string matching
/*int match_dtoken(const char *restrict word, const char *restrict sentence, int max) {
    int i;
    for (i = 0; (word[i] != '\0') && (i < max); i++) {
        if ((word[i] != sentence[i]) || (sentence[i] == '\0'))
            return 0;
        if (is_alphanumeric(sentence[i]) && word[i+1] == '\0')
            break;
    }
    return i;
}*/


// returns length
__inline__ int prod_alphanumeric(const char *buffer) {
    int i;
    for(i = 0; is_alphanumeric(buffer[i]) && (buffer[i] != '\0'); i++);
    return i;
}



// returns length
__inline__ int prod_whitespace(const char *buffer) {
    int i;
    for(i = 0; is_whitespace(buffer[i]) && (buffer[i] != '\0'); i++);
    return i;
}


// returns length
__inline__ int prod_comment(const char *buffer) {
    int i;
    if (**(short**)buffer == '/*') {}
        for(i = 0; *(short*)(*buffer+i) == '
    
    if (**(short**)buffer == '//') {}

    if ((buffer[0][0] == '/') && (buffer[0][1] == '*')) {}
    for(i = 0; is_whitespace(buffer[i]) && (buffer[i] != '\0'); i++);
    return i;
}




// will modify buffer pointer
// basically will return a token type, and any extra data about a token
// will not process sub-tokens
int advance_token(token_t *restrict token, state_t *restrict state, char *restrict *restrict buffer) {

    /*inline int prod_call(match_call_t match, const char **list, int llen, token_id_t *id) {
        int len;
        for(int i = 0; i < llen; i++)
            if ((len = match(list[i], *buffer, 256))
                return len;
        return 0;
    }*/

    /*int prod_dtoken(token_id_t *id) {
        int len;
        for(int i = 0; i < len(dtokens); i++)
            if ((len = match_dtoken(dtokens[i], *buffer, 256)))
                return len;
        return 0;
    }

    int prod_alphanumeric(token_id_t *id) {
        int len;
        for(int i = 0; i < len(dtokens); i++)
            if ((len = match_dtoken(dtokens[i], *buffer, 256)))
                return len;
        return 0;
    }*/

    //for(;;) {
    int len;
    token_id_t id;
    

    // parse whitespace
    //for(;is_whitespace(**buffer); *buffer++);
    //for(len = 0; is_whitespace(buffer[0][len]); len++);
    if ((len = prod_whitespace(*buffer)) > 0) {
        token->type = (token_t){
            .type = TOKEN_TYPE_WHITESPACE,
            .string = *buffer,
            .slen = len,
        };
        *buffer += len;
        return 0;
    }
    


    // parse comments
    if ((len = prod_comment(*buffer)) > 0) {
        token->type = (token_t){
            .type = TOKEN_TYPE_COMMENT,
            .string = *buffer,
            .slen = len,
        };
        *buffer += len;
        return 0;
    }


    // parse preprocessor directives


    // detect if utoken
    len = prod_utoken(&id);
    
    if (len > 0) {

        *buffer += len;

        token->type = TOKEN_TYPE_TEMP_UNDELIMITED;
        token->id = id;

        return 0;
    }


    // detect if dtoken
    len = prod_dtoken(&id);
    
    if (len > 0) {

        *buffer += len;

        token->type = TOKEN_TYPE_TEMP_DELIMITED;
        token->id = id;

        return 0;
    }


    // detect if unknown alphanumeric sequence
    len = prod_alphanumeric();

    if (len > 0)
    //}
}




int parse_statement(state_t *state, char *buffer, int blen) { // is the blen needed?

} 




int parse_comment(state_t *state, char *buffer, int blen) { // is the blen needed?
    
}










/*
    Lets ignore preprocessor for now.
    So what are the big things here? 
    We have variables, which are apart of statements, which are semicolon separated
    And then we have scopes, and behaved scopes, which are curly bracket separated.
    Some behaved scopes also behave like a statement.

    Although I guess I forgot that some of the things I grouped in behaved scopes don't 
    actually always require scopes. Or I guess most of them really. In that case, they need semicolon
    separation as well.

    Hmm. So how would I group this then?
    I suppose brackets could just be a sort of void statement or something that don't need semicolons.

    Alright, in that case, scopes aren't statements. However, they will be free of the semicolon rule,
    And I suppose what generates an exception to that is when a statement follows it that is expected.

    oooooh. Or.... I would have the do while() be two different statements, with a condition that they
    Bust always be following one after the other. Like, perhaps that could be a behavioral condition set
    by the "do", which is that it requires a compatable statement as the conditional.

    Anyway, what are the different statements in C? (some of them are void, and others return a value)

    - variable assignments
    - variable definitions
    - function call
    - function definition
    - operators
    - loops and conditionals

    It is worth noting that there are sub-statements too. In fact, every operator creates a new
    sub-statement, that is generated according to the order of operators. But basically, the order
    of operators creates implicit parenthesis. Which is also a form of scoping. Including function
    parameters, I believe, if I can find a way to justify the function parameters as being an operator.
    Perhaps it creates a tuple, which functions can accept as parameters. But things that don't expect
    A tuple will only select the last item. Tuples are created with commas, and are delimited by parenthesis.
    (In other words, parenthesis are not required to create a tuple).

    There are some rare things that aren't statements, such as labels.

    Anyway, since this is C, lets do a single pass generation, rather than multi-pass.

    According to gpt, comments are ignored by the preprocessor and the compiler, and are removed
    in the lexical analysis phase

    LOOK UP FINITE AUTOMATA FOR LEXICAL ANALYSIS
    Which basically seems to be about keeping track of states and of a state transition table
    (which is basically program flow).
    And then each statement will modify the state of the program. And each transition can be modified by
    a statement.

    Of course, I should also keep track of scopes. 

    As a sort of pre-assembly thing, lets include in our state the current line number.
    And lets also keep track, probably not in our state but just in general, the current byte count
    of the program so that we can generate assembly.

    I think I am going to write off the statements with the for() parenthesis as specific behavior 
    defined by "for", which works because "for" is the first statement seen in such a case.

    I think how I will handle this is I will have a general parser that populates the general 
    status of the current statement. And then once the type of statement is known, it passes off
    the parsing to a specilized function it calls that will continue on where it left off, 
    Including the buffer.

    Actually, new plan.
    While perhaps slightly less efficient, it will probably be way easier to organize
    But basically call a function that will identify what the statement is, and then 
    Call a function to parse that statement.
    It should also take care of knowing where a statement ends, and feeding the length of the statement
    back to the caller, including the semicolon. And then semicolons will just be ignored by the rest of the
    parser.

    Actually, no. The scope should determine what kind of statement it is, and then pass that off
    to a function to process that statement.
*/
