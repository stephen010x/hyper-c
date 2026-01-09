// parser.c

#include <stdint.h>
#include <stdio.h>


#include "utils/macros.h"
#include "utils/debug.h"
#include "hyperc/parser.h"
#include "hyperc/match/parser.h"



// #ifdef __DEBUG__
// 
// #include "hyperc/lexer.h"
// #include "hyperc/match/lexer.h"
// #include "hyperc/match/parser.h"
// 
// #endif /* #ifdef __DEBUG__ */








// TODO rename to new_token
ptoken_t *ptoken_new_user(int tindex, void *token) {
    ptoken_t *ptoken = malloc(sizeof(ptoken_t));
    if (ptoken != NULL)
        *ptoken = (ptoken_t){
            .type = PFLAG_USER,
            .user.tindex = tindex,
            .user.token = token,
        };
    return ptoken;
}



// assume the children are detached, and we can directly inherit the list
ptoken_t *ptoken_new_match(int tid, int rid, int count, ptoken_t **children) {
    ptoken_t *token = malloc(sizeof(ptoken_t));

    if (token != NULL) {
        *token = (ptoken_t){
            .type = PFLAG_MATCH,
            //.match.mid = mid,
            .match.targetid = tid,
            .match.ruleid = rid,
            .match.count = count,
            //.match.children = malloc(count * sizeof(void*));
            .match.children = children,
        };
    }
    
    // for (int i = 0; i < count; i++)
    //     token->match.children[i] = children[i];
    
    return token;
}



void ptoken_free(ptoken_t *token) {
    switch (token->type) {
    
        case PFLAG_MATCH:
            for (int i = 0; i < token->match.count; i++) {
                ptoken_free(token->match.children[i]);
                DEBUG(token->match.children[i] = NULL;);
            }
            free(token->match.children);
            DEBUG(token->match.children = NULL;);
            free(token);
            break;
            
        case PFLAG_USER:
            free(token);
            break;
            
        default:
            segfault();
    }
}




// recursive
void _print_ptoken(ptoken_t *token, ptoken_name_handler_t print_handler, int depth) {
    for (int i = 0; i < depth; i++)
        //(i % 2) ? printf("|") : printf(" ");
        printf("  | ");

    switch (token->type) {
        case PFLAG_USER:
            printf("TOKEN ");
            print_handler(token);
            printf("\n");
            break;
            
        case PFLAG_MATCH:
            print_handler(token);
            printf("\n");
            for (int i = 0; i < token->match.count; i++)
                _print_ptoken(token->match.children[i], print_handler, depth+1);
            break;

        default:
            printf("INVALID []\n");
    }
}


void print_ptoken(ptoken_t *token, ptoken_name_handler_t handler) {
     _print_ptoken(token, handler, 0);
}













// the golden ratio
#define GOLDEN_ALLOC_MULT 1.61803


int pstack_init(pstack_t *stack) {
    *stack = (pstack_t){
        .index = 0,
        .alloc = 16,
        .data = malloc(16 * sizeof(void*)),
    };

    if (stack->data == NULL)
        return -1;
    return 0;
}


void pstack_free(pstack_t *stack) {
    for (; stack->index > 0; stack->index--) {
        ptoken_free(stack->data[stack->index-1]);
        DEBUG(stack->data[stack->index-1] = NULL;);
    }
    free(stack->data);
    DEBUG(
        *stack =(pstack_t) {0};
    );
}


void pstack_push(pstack_t *stack, ptoken_t *token) {
    if (stack->index >= stack->alloc) {
        stack->alloc *= GOLDEN_ALLOC_MULT;
        stack->data = realloc(stack->data, stack->alloc * sizeof(void*));
    }

    stack->data[stack->index++] = token;
}


// returns a malloc'ed list of pointers to ptoken_t's that will need to be freed.
// usually this will be inserted into a ptoken, where the ptoken will free 
// it for us when we free the token.
ptoken_t **pstack_pop(pstack_t *stack, int count) {
    ptoken_t **group;

    if (count > stack->index)
        return NULL;

    group = malloc(count * sizeof(void*));

    for (;count > 0; count--)
        group[count-1] = stack->data[--stack->index];

    return group;
}


// moves pstack data into a ptoken
ptoken_t *pstack_to_ptoken(pstack_t *stack) {
    ptoken_t *ret;

    if (stack->length == 0)
        ret = NULL;
    if (stack->length == 1)
        ret = stack->data[0];
    else
        ret = ptoken_new_match(0, 0, stack->length, stack->data);
        
    //free(stack->data);
    // this needs to be here in case someone calls free on this pstack
    // so we don't have the new ptoken children freed as well
    stack->data = NULL;
    
    return ret;
}


void print_pstack(pstack_t *stack, ptoken_name_handler_t print_handler) {
    printf("STACK\n");
    for (int i = 0; i < stack->index; i++)
        _print_ptoken(stack->data[i], print_handler, 1);
}






/*
We check the newest token in the stack with the last match member of all of the rules. If it matches, then we match the second to last match member with the second to last in the stack, and so on. If it is a full match, then we pop those tokens from the stack, and replace it with a single new match token. If there are no matches, then we simply keep adding tokens until there is.
But now, what about handling match conflicts? Is there a way to have the desciptor set in a way that it can be first match?
Lets just assume so for now until we actually run into problems. Because generally I think that the descriptor set is built with this in mind, or at least can be built with this in mind. And that conflicts should generally be rare based on what I have seen from my own C descriptor table.


Alright, how should input be handled for this? 
I was thinking perhaps we pass to the input the mid, and it will return either
a -1, as in no match, or it will return an index into a list it controls.
Though, I suppose I don't even need that. I can insert user tokens into the stack
without the need for user input. So I just need a match handler

*/





// TODO: comment this out. for debugging
char *test_ptoken_name_handler(ptoken_t *ptoken);



// TODO: move this to the macros file
#define XOR(__a, __b) (!(__a) ^ !(__b))



// this is a bottom-up parser
ptoken_t *match(match_t *m, int rule, void *input) {
    pstack_t stack;
    match_tree_t tree = m->tree;
    //bool is_eof = false;

    // an artifact of the original top-down parser
    // can maybe be used to detect error.
    (void)rule;

    // create the stack
    pstack_init(&stack);

    // loop until eof is found. The do/while loop should
    // then condense the entire stack to a single translation unit token
    // before reaching the conditional of this for loop again, unless
    // there is a syntax error. In any case, we want to abort here.
    for (int index = 0;; index++) {
        //void *in;
        ptoken_t *token;
        bool matchflag;

        // abort if next token is eof
        if (m->eof_handler(input, index) == true)
            break;
        
        // push token onto stack
        //tindex = m->input_handler(input, index, mid);
        token = ptoken_new_user(index, m->tget_handler(input, index));
        pstack_push(&stack, token);

        //printf("token %d\n", index);

        // TODO: comment this out. for debugging
        print_pstack(&stack, test_ptoken_name_handler);

        // loop until no matches found
        do {
            matchflag = false;
            //match_target_t *target;
            //int ti;
            
            // check every match rule. If no match, then push token onto the stack
            // loop through every match target
            //for (target = m->tree; *target != NULL; target++) {
            for (int ti = 1; tree[ti] != NULL; ti++) {
                //match_rule_t *rule;
                
                // if unimplemented, then continue to next target
                if (tree[ti][1][0] & MF_NOT_IMPLEMENTED)
                    continue;
                
                // loop through every match rule
                //for (rule = *target; *rule != NULL; rule++) {
                for (int ri = TYPE_A; tree[ti][ri] != NULL; ri++) {
                    int end, i, j;
                    bool ismatch = false;
                    
                    // get length of subrule
                    // TODO: optimize this so we dont have to count each time
                    for (end = 0; tree[ti][ri][end] != 0; end++);

                    // if length of subrule exceeds stack index, then continue to
                    //     next rule
                    if (end > stack.index)
                        continue;
                    
                    // loop through every match subrule until mismatch found
                    //     starting from the end
                    // if subrule is optional and mismatch, then don't fail and
                    //     instead only increment the subrule i and not the stack j
                    for (i = 1, j = 1; end - i >= 0; i++) {
                        uint32_t mid;
                        ptoken_t *stoken;

                        mid = tree[ti][ri][end-i];
                        stoken = stack.data[stack.index - j];
                        
                        if (mid & MF_UNRES_MASK) {
                            // match only if he stoken type is a user type
                            if (stoken->type == PFLAG_USER)
                                ismatch = m->tmatch_handler(stoken->user.token, mid);

                        } else {
                            // match if stoken mid is equal to the rule mid
                            ismatch = ((uint32_t)stoken->match.targetid == (mid & MF_MATCH_MASK));
                        }

                        // if  optional and  ismatch, then increment both and no abort
                        // if !optional and  ismatch, then increment both and no abort
                        // if  optional and !ismatch, then increment i    and no abort
                        // if !optional and !ismatch, then abort

                        // TODO: this will fail if a rule is solely optional subrules.
                        if (ismatch) {
                            printf("match (t=%d r=%d i=%d j=%d)\n", ti, ri, i, j);
                            j++;
                        } else if (!(mid & MF_OPTIONAL)) {
                            printf("nomatch (t=%d r=%d i=%d j=%d)\n", ti, ri, i, j);
                            break;
                        }
                    }

                    // if the subrule loop breaks with ismatch = true, then reduce stack
                    // and break out of target loop
                    if (ismatch) {
                        ptoken_t **group;
                        ptoken_t *newtoken;

                        matchflag = true;
                    
                        // pop stack into group equal to the length of the rule (end)
                        group = pstack_pop(&stack, end);
                        
                        // create token from group
                        newtoken = ptoken_new_match(ti, ri, end, group);
                        
                        // push new group token into stack
                        pstack_push(&stack, newtoken);

                        // TODO: comment this out. for debugging
                        print_pstack(&stack, test_ptoken_name_handler);

                        goto _exit_target_loop;
                    }
                }
            }
            _exit_target_loop:

        // exit the loop once there are no matches
        } while (matchflag == true);
    }

    return pstack_to_ptoken(&stack);
}





