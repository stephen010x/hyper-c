
#include <stdio.h>
#include <stbool.h>
#include <string.h>



#define MAX_TOKEN_SIZE      256
#define MAX_STATEMENT_SIZE  1024



const char *test_code = "
    //#include <stdio.h>
	//#include <string.h>
	
	// Function prototype
	void swap(int *x, int *y);
	
	int main() {
	    // Basic variable declaration and initialization
	    int a = 5, b = 10;
	    char str[50];
	
	    // Control structure: if-else
	    if (a < b) {
	        printf(\"a is less than b\\n\");
	    } else {
	        printf(\"a is not less than b\\n\");
	    }
	
	    // Loop: for
	    printf(\"Counting down: \");
	    for (int i = 5; i > 0; i--) {
	        printf(\"%d \", i);
	    }
	    printf(\"\\n\");
	
	    // Arrays and user input
	    printf(\"Enter a string: \");
	    fgets(str, 50, stdin);  // Read line including spaces
	
	    // Remove newline character, if present
	    size_t len = strlen(str);
	    if (len > 0 && str[len-1] == '\\n') {
	        str[len-1] = '\\0';
	    }
	
	    printf(\"You entered: %s\\n\", str);
	
	    // Pointers and function call
	    printf(\"Values before swap: a = %d, b = %d\\n\", a, b);
	    swap(&a, &b);
	    printf(\"Values after swap: a = %d, b = %d\\n\", a, b);
	
	    // Control structure: switch-case
	    char letter = 'A';
	    switch (letter) {
	        case 'A':
	            printf(\"Letter is A\\n\");
	            break;
	        case 'B':
	            printf(\"Letter is B\\n\");
	            break;
	        default:
	            printf(\"Letter is not A or B\\n\");
	            break;
	    }
	
	    return 0;
	}
	
	// Function definition with pointers
	void swap(int *x, int *y) {
	    int temp = *x;
	    *x = *y;
	    *y = temp;
	}
";



/*enum token_types {
    TOKEN_UNDEFINED = 0,
    
    TOKEN_TYPE_SPECIFIER,
    TOKEN_STORAGE_CLASS_SPECIFIER,
    TOKEN_TYPE_QUALIFIER,
    TOKEN_CONTROL_STATEMENT,
    TOKEN_FUNCTION_SPECIFIER,
    TOKEN_STRUCTURE_KEYWORD,
    TOKEN_MISC_KEYWORD,
    
    TOKEN_OPENING_BRACE,
    TOKEN_CLOSING_BRACE,
    TOKEN_OPENING_PARENTH,
    TOKEN_CLOSING_PARENTH,
    TOKEN_OPENING_BRACKET,
    TOKEN_CLOSING_BRACKET,

    TOKEN_ABSTRACT,         // for symbols and special characters
    TOKEN_STRING,
    TOKEN_VALUE,            // for chars, ints, floats, etc with a value.
};*/



char delimiters[] = {' ', '\t', '\n'};


//token_def_t token_defs[] = {
/*token_t token_defs[] = {
    {TOKEN_TYPE_SPECIFIER, "void"},
    {TOKEN_TYPE_SPECIFIER, "char"},
    {TOKEN_TYPE_SPECIFIER, "int"},
    {TOKEN_TYPE_SPECIFIER, "float"},
    {TOKEN_TYPE_SPECIFIER, "double"},
    {TOKEN_TYPE_SPECIFIER, "short"},
    {TOKEN_TYPE_SPECIFIER, "long"},
    {TOKEN_TYPE_SPECIFIER, "signed"},
    {TOKEN_TYPE_SPECIFIER, "unsigned"},

    {TOKEN_STORAGE_CLASS_SPECIFIER, "register"},
    {TOKEN_STORAGE_CLASS_SPECIFIER, "static"},
    {TOKEN_STORAGE_CLASS_SPECIFIER, "extern"},
    
    {TOKEN_TYPE_QUALIFIER, "const"},
    {TOKEN_TYPE_QUALIFIER, "volatile"},
    {TOKEN_TYPE_QUALIFIER, "restrict"},
    
    {TOKEN_CONTROL_STATEMENT, "if"},
    {TOKEN_CONTROL_STATEMENT, "else"},
    {TOKEN_CONTROL_STATEMENT, "switch"},
    {TOKEN_CONTROL_STATEMENT, "for"},
    {TOKEN_CONTROL_STATEMENT, "while"},
    {TOKEN_CONTROL_STATEMENT, "do"},
    {TOKEN_CONTROL_STATEMENT, "break"},
    {TOKEN_CONTROL_STATEMENT, "continue"},
    {TOKEN_CONTROL_STATEMENT, "goto"},
    
    {TOKEN_FUNCTION_SPECIFIER, "inline"},
    
    {TOKEN_FUNCTION_SPECIFIER, "inline"},
};*/


// maybe instead of assigning token groups, I should just assign token names.


token_t delimited_tokens[] = {
    {TOKEN_VOID, "void"},
    {TOKEN_CHAR, "char"},
    {TOKEN_INT,  "int"},
};

token_t packed_tokens[] = {
    {TOKEN_ASTERISK,   "*" },
    {TOKEN_ASSIGNMENT, "=" },
    {TOKEN_EQUALS,     "=="},
    {TOKEN_ARROW,      "->"},
};




// NOTE: in the test code, pay attention to all the spaces that can be deleted, and how that kind of
// group parsing might work. For instance,
// int temp = *x;
// int temp=*x;


// scopes will be maintained in the stack when parsing, so no allocations for 
// this struct are needed
typedef struct scope_state {
    struct scope_state *parent;
} scope_state_t;


typedef struct {
    //char *buffer;   // instead of allocating a buffer, we just slice the original string
    //int   blen;
    scope_state_t *root_scope;
    scope_state_t *scope;       // current scope
} state_t;



typedef struct {
    int   type;
    union {
        char *name;
        //char  text[MAX_TOKEN_SIZE];
        struct {
            char *string;
            int   strlen;
        };
    }
} token_t;


/*typedef struct {
    int   type;
    char *name;
} token_def_t;*/





// keep track of:
//  - program state
//  - state changes

int main(int argc, char *argv[]) {

    state_t state;
    state_init(&state, test_code);

    parse_scope_recursive(&state);

    return 0;
}




void state_init(state_t *state, char *source) {
    *state = (state_t){
        .buffer = source;
        .blen = 0;
        .root_scope = NULL;
        .scope = NULL;
    };
}




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




// will modify buffer
// basically will return a token type, and any extra data about a token
int advance_token(token_t *token, state_t *state, char **buffer) {
    
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
