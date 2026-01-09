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
                    for (i = 1, j = 1;;) {
                        bool ismatch = false;
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

                        if (ismatch) {
                            printf("match (t=%d r=%d i=%d j=%d)\n", ti, ri, i, j);
                            j++;
                        } else if (!(mid & MF_OPTIONAL)) {
                            //printf("nomatch (t=%d r=%d i=%d j=%d)\n", ti, ri, i, j);
                            matchflag = false;
                            break;
                        }
                        
                        i++;

                        // if we get to the end of the subrules, and no mismatches, then it is
                        // a match
                        if (end - i < 0) {
                            matchflag = true;
                            break;
                        }
                    }

                    // if the subrule loop breaks with ismatch = true, then reduce stack
                    // and break out of target loop
                    //if (ismatch) {
                    if (matchflag) {
                        ptoken_t **group;
                        ptoken_t *newtoken;
                        int length;

                        //matchflag = true;
                        // length is the number of tokens that matched with rule, (j-1)
                        length = j-1;
                    
                        // pop stack into group equal to the number of matched tokens
                        group = pstack_pop(&stack, length);
                        
                        // create token from group
                        newtoken = ptoken_new_match(ti, ri, length, group);
                        
                        // push new group token into stack
                        pstack_push(&stack, newtoken);

                        // TODO: comment this out. for debugging
                        //print_pstack(&stack, test_ptoken_name_handler);

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





/*

Alright, my current bottom up approach is running into problems. Namely, it creates matches too quickly
where it will basically not take into account tokens that haven't made it into the stack yet, which 
is particularly problematic for optional trailing subrules.
But moreover, there is a deeper underlying problem for cases where matches lead to dead ends instead
of the translation unit due to selecting the wrong match rule.

A few possible solutions:
Select the match that uses the most tokens.

Another idea was to simply fork them, where I would check the stack with every single rule, and put all successful matches into a table of possible matches, where now I have to check each possible match to gain even more matches, which will further fork it, etc. As you can see this has several problems. But according to a wiki page that seems to have a similar idea, it seems like it is possible to optimize.

One solution I had though is rather interesting. It is a hybrid top-down bottom-up parser.
Basically, the tokens are parsed bottom up, but the matches we check are top-down.

So, like, we push some tokens into the stack, To then check them, we start at the prime target, which will
be the translation unit, and we check that. If it doesn't match, we go down.
Now, once a match is finally made, we traverse back up, and then we assert that the next match must be the next subrule of the parent (sibling to the currently matched rule) of the rule that we successfully matched with. And if that fails, then we systematically undue our progress on the stack by one step by traversing up once, and then we repeat this process.

This is actually genius. The parsing is done bottom-up, but the rule checking and backtracking is done top-down. Intuitively it feels mathematically sound to yield me a guarenteed parse that results in a single translation unit match. (Though if there are multiple matches, then it will just be biased towards one of them, in which case, you just have bad grammer or something. Perhaps this is what they mean by a contextless parsing language.)




Alright, so, how do we handle lists?
The idea is to use the bottom-up scheme to avoid recursion.
Also, how do I handle token count? I suppose the current prediction could be used to assert a token count, but then this just goes back to the problem of recursion, I think.


One idea for speedups though is to use a 2D chart where for every target we list which targets are an eventual child, and which aren't.

The actual descriptor language should be written like it is a simple bottom-up parser, except that it guarentees the match that will result in a final translation unit. And that if multiple translation units are possible, then the algorithm will yield an arbitrary one. So I guess it isn't purely deterministic. But I will still label that fault onto the descriptors, not the algorithm.

So what about if multiple rules within a target match, which will generally be the case for lists.
I suppose for that, we would want to prove each one with the parent rule. Though if both match that would still be a bit troublesome. Because that could mean that one might lead to a full translation unit, while the other won't.

Also, I suddenly want a handler free system, where you can input either a binary list or a token list
to the matching function. And the first list is simply the tokens + the stuff that is guarenteed to match,
so as to take that pressure off the next stage of the parser. And then it would output a token that is either the prime token you are trying to match for, or it will return a token of the type "incomplete match" or something.
Although, perhaps I should have three different functions.
tokenize_bytes
match_linear
match_tree
Either that, or I should just have a list of things the match tree should try to match with for the tokenizer, which I guess works better.

Functions:
tokenize
match

Hmm... But this will result in a token per byte, which is unideal. I could have both the tokenizer and the matcher both use the tree. But at this point I should just have the single MATCH function, and have the input be a struct that will specify if bytes or tokens. And then match will call one of the two subfunctions to parse that.

Also, add these flags:
MF_DISCARD          // when this match occurs, it will discard it in the output. Meant for subrules.
                    //     Works in both subrules and targets.
MF_COLLAPSE         // this is a rule not meant to appear in the final output tree.
                    //     it will insert it's children into it's parent's children in it's place.
                    //     I think this will mainly be a post-processing flag so as to not mess with
                    //     the top-down parse rule matcher.
                    //     Works in both subrules and targets.


Anyway, back to the algorithm at hand. We have two matches. What do we do?
Well, I suppose the combination of the top-down stack and the bottom-up stack would give us enough information to get back to this state if a parent match failure occurs later down the line. Lets say we match with the first rule, we reduce the bottom stack to that match, we recurse back up. Then the next rule of the parent fails to match. If the child rule of a parent target fails to match, and it isn't the first child rule, then we go back down the previous child rule, and we undo the bottom stack, but use the old match of the bottom stack we undoed to figure out which rule to test next of the previous child rule. To speed this up we can add a flag to the token indicating whether all of the rules of the matched target have been tested yet.

This could be sped up with a match cache, and with the target-has-ancestor 2d table.
Though save those optimizations for when you can actually verify this idea works.

And as for recursion, I think the bottom-up part of the algorithm is the saving grace for this one.
Since the actual matches are done bottom-up via a separate stack, we only ever need to visit each rule once during a recursive dive. This can be easily checked by having a boolean table of every target that we set as we enter the target, and unset as we leave the target. This way we are guarenteed to avoid any and all recursion. Even unobvious ones.

Lets say we have this rule:

A:
    B * x
    x

B:
    A * x
    A

We enter A first. We check the first rule, which then leads us to entering B. But then we see A again. 
Alright, we know that we have already visted A, so now instead of a recursive check, we do an ordinary bottom-up rule check. If A * x, nor A are on the top of the stack, then we simply recurse back up, failing the check.

Actually, this brings up a problem I hadn't considered yet, which is when do we insert new tokens into the stack? In a normal bottom-up, we insert a new token once we verify that there are no valid matches with the current stack. But we can't exactly start over in a top-down aspect. Though I suppose with the bottom-up stack, we could use that to retain a reversable state that can still guarentee mathematical truth. Although I would prefer not to have to recurse down the tree every time, as it is slower than the now potentially less viable method I had originally envisioned. Honestly, restarting the top-down checks sounds rather redundant, because we generally should back up in the same general area as before when doing these matches.
Then again, changing the stack can potentially completely change the whole system. The bottom stack is basically a state machine for the top stack. But this suggests that we should just be parsing the entire text as a whole rather than one at a time, because the only true state would be the one that includes all of the tokens, as our condition for this is to find the prime match that includes all of the tokens. But that would get rid of the bottom-up aspect of it, and generally make this a purely top-down parser, which is by itself problematic due to things like recursion.

I suppose we can just have it so that we push things into the stack whenever a match rule calls for it.
Looking back at the bottom-up aspect of it, we check the current rule to see if it matches with anything in the stack. If not, then it might be a child rule that needs to be matched instead. And when that returns, that means that the stack has since been modified, and it's child is now potentially in there. It will check again. If it fails, it will then check that child again until that child verifies that it has tried all of it's possabilities. These possabilities may include recursions. In a recursive scenario, we can go down recursively once for each time a parent match fails and queries us again. Although instead of recursively, since we only ever need to check any target once, we just loop over all our children with a normal non-recursive stack rule check, and we perhaps do this n times. Though I feel like this could be simplified by doing an actual recursion. If we know we are currently in a recursion, we can use a recursion counter, and we prod recursively down one more than we previously have. But at least one of our bottom-up rules must match at least once. This is starting to get a little abstract though, and I am not confident this is robust. Especially with non-obvious recursion. I think indirect recursion is why we don't want to recurse, and need to stay in the same rule. So lets try to do that unless some elegant solution can be discovered.

Alright, what about this? For each target with a match, we generate a token that will be pushed into the stack.
What we do though is we check every single rule in a target with a simple non-recursive bottom-up stack check. And we record which ones matched, and which ones didn't, and which ones don't currently match, but potentially could if we recursed down them. We then pick the first match, and reduce the stack using that, and returning. But if there are no matches, but there are some potential matches, we then recurse down.
This will bais towards first matches rather than long recursive matches, so that recursion only happens incrementally whenever a parent match fails.
To do this though, I guess we would not initially recurse down. If there are no direct matches, then we would probably just retreat back up. Perhaps we can set a recursion depth limit that increments upon every fail, and this happens until a match is found, or a path is proven to be invalid.
Notice the possability of a recusive match, where say we have an entire function definition, but it is surrounded by operators. Say this is an inline function definition with an included invocation. So it will behave as a statement, which means it can be surrounded by recursive operators. This is a recursive hell hole. Especially since these operators can be found within the function, in which case recursion is already there. 

To fix this, we have a recursion budget, which will bias smaller over larger. But that will avoid infinite recursion.

Another idea is to use a wrongness system. Where basically we construct bottom-up stack with top-down... meh this is just a backtracking method for bottom-up parsers, only with a top-down check speedup.


Well, lets go back to the original idea, as I feel like it was extremely effective, and very fast, aside from a few of the problems that currently prevent it from working. Mainly conflict resolution and recursion. Where the focus was bottom-up parser, with a top-down rule selecter. As far as conflict resolution, I think we should just have tokens that are reduced and added to the stack have some way to keep track of a position in this tree check that can be resumed if a token on the bottom stack is deemed to be invalid.
Basically, imagine a system that checks every single possible combination, except we can skip certain branches of that, and come back to them later. Though recursion creates infinite combinations, so we need a way to skip those early, and come back to them to resume them incrementally.
I think ideally we need to revolve this system around the state of the stack, and we want to reduce the processing for every token added onto the stack as much as possible.

Hmm... One interesting idea is to create a recursion budget based off of the current size of the stack.
Ideally I think we want to avoid ever backtracking the bottom stack. As that puts reliance on the top-down process that we really only want to give to the bottom-up. Really this should still be a bottom-up parser, with a top-down match helper.

But anyhow, lets have two things here. First, is the recursion budget, which is dependant on the current size of the stack, and then we also have a direct match table cache. The latter to speed up redundant matches for the current stack.
But the recursion budget, or I guess really it is a token budget that will determine if our next check should recurse up, or instead recurse down. But it will allow us to effectively check matches tangentially on the tree,

To explain how this works, lets say we have a token budget of 5 tokens, as that is how many is currently on the stack. We would then compare that to how many tokens we are matching with in total in our current path. Lets say the translation unit is one token, the child of that is 3 tokens, and lets say we are currently in another one that is three tokens. We add the sum of all of those tokens, where each depth subtracts one token, excluding the depth you are currently on. So that is 1+3+3-2, which is 5 tokens. That means we are on the right check depth. Ish. I suppose we shouldn't be counting future sibling tokens. So lets say the token count of our current and future path is 1,3,3,2,5,1. But we are currently on token 1,2,1,2,1. That would be 3 tokens, which is too few. Although consider we also need to count the token count of previous siblings as well. Though maybe we don't, because remember this is a bottom-up parser. So all of the previous siblings should already be on the stack as a single token if we want to create a match.
The goal is to have each token be on the stack if we want a rule to actually match. So the tokens should be 1 to 1. The exception is any recursion in the top-down checker, as those are tokens that are yet to be consolidated on the stack.
So anyway, back to it. We have 1,2,1,2,1. That is three tokens out of five. That means that we have either not recursed deep enough. And if we can't recurse any more, then this match is invalid, and we need to go back up.
Now lets say our current token sum is larger than the current stack size of 5, like 7. That means that we are too deep, and need to go back up, marking this match as a fail. We record all of our match successes and fails in our table for each target to speed things up. This table is only valid for the current stack state.
Now say that a match occurs. This can only happen if the stack size matches the token sum. Because if they don't match, that means that it will not reduce into our prime match target. Though I suppose this might be problematic, as that means that we would never reduce until we can ensure it will reduce to the prime target.

Lets go back to our bottom-up parser. Which are the rules we want to check with for reduction?
We can potentially consider these nested rules as one large compound rule.
Due to left hand recursion though, we may also need to take into account the accumulative right hand token count with the token count of the remaining unconsumed tokens not yet in the stack, to create a depth limit there.

Alright then, what is the simplest version of this, and then maybe figure out optimizations later? Ideally whatever it is we only want to incrementally recurse for things that might cause infinite recursion.
I suppose it would be to have a recursion budget, where we iterate through a target match and incrementing the budget once every no-match until a match is found, or it is determined that no match is possible.
Once a match is found, it's parent is then the new prime target, and we do a budget recursion from that point until we either complete the entire match or we determine that there is no match, and receed. Matches will add to the stack while prime target failures will revert the stack to before it was selected as a prime target.

*/
