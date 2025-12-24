// parser.c

#include <stdint.h>
#include <stdio.h>


#include "utils/macros.h"
#include "utils/debug.h"
#include "hyperc/parser.h"



#ifdef __DEBUG__


typedef void *(*input_handler_t)(void *, uint32_t);
typedef void *(*output_handler_t)(void *, uint32_t);





#include "hyperc/lexer.h"
#include "hyperc/match/lexer.h"
#include "hyperc/match/parser.h"






typedef struct {
    
} test_state_t;


enum {
    OUTPUT_NULL = 0,
    OUTPUT_TOKEN,
    OUTPUT_GROUP,
}
typedef int_t output_type_t;



//#define OUTPUT_MAX_ARGS 10


struct output;
typedef struct output output_t;

struct output {

    output_type_t type;

    uint32_t match_flags;
    
    union {
        token_t *token;
        
        struct {
            int_t gcount;
            output_t **group;
        };
    };
    
};


void test_free_outputs(output_t *out) {
    switch (out->type) {
        // if null then do nothing
        case OUTPUT_NULL:
            return;
    
        // zero out and only free self
        case OUTPUT_TOKEN:
            *out = {0};
            free(out);
            return;

        // recursively free all subobjects
        // then free subobject pointer list
        // then zero out and free self
        case OUTPUT_GROUP:
            for (int i = 0; i < out->gcount; i++)
                if (out->group[i] != NULL)
                    test_free_outputs(out->group[i]);
            free(out->group);
            *out = {0};
            free(out);
            return;

        // if not recognized, then segfault
        default:
            segfault();
            return;
    }
}


// assume tokens are always accessable, and are apart of a larger
// separate list of tokens
output_t *test_alloc_outtoken(uint32_t flags, token_t *token) {
    output_t *out = malloc(sizeof(output_t));

    *out = {
        .type = OUTPUT_TOKEN,
        .match_flags = flags,
        .token = token,
    };

    return out;
}

// assume outputs are already allocated, and are simply passed to this group to
// later free by itself.
output_t *test_alloc_outgroup(uint32_t flags, output_t **outputs, int_t out_count) {
    output_t *out = malloc(sizeof(output_t));

    *out = {
        .type = OUTPUT_GROUP,
        .match_flags = flags,
        .gcount = out_count,
        .group = malloc(out_count * sizeof(void*)),
    };

    for (int i = 0; i < out_count; i++)
        out->group[i] = outputs[i];

    return out;
}



// return output if match
// return NULL if no match
// only used for singular token matching
// needs to step to next token (not stepping if no match is optional)
// set *t to null if an error has occured or EOF reached
output_t *test_input_handler(uint32_t matchid, token_t **t) {
    uint32_t flags = mid;
    uint32_t mid = matchid & MF_MATCH_MASK;

    token_t *token = *t;

    output_t *out = NULL;

    bool is_match = false;

    // if EOF, then set **t to NULL

    // if utoken
    if (flags & MF_UTOKEN)
        if ((mid == token->tid) && (token->type == TOKEN_UTOKEN))
            is_match = true;

    // if dtoken
    else if (flags & MF_DTOKEN)
        if ((mid  == token->tid) && (token->type == TOKEN_DTOKEN))
            is_match = true;

    // if token_type
    else if (flags & MF_TOKEN_TYPE
        if (mid == token->type)
            is_match = true;

    if (is_match) {
        out = test_alloc_outtoken(matchid, token);
        *t += 1;
    }


    return out;
}



// will condense an ouput if only one in group.
// manages allocations throughout this
// non recursive. only a depth of one.
// output_t *deglove_output(output_t *out) {
//     if (out->gcount != 1)
//         return out;
// }



// returns grouped object
// caller handles the allocations for inputs lists, but not the inputs
// themselves, except for those passed back out as an output
// passes in the flags set by the group or'ed with flags set by the group reference
output_t *test_output_handler(uint32_t flags, output_t **inputs, int_t in_count) {

    // deglove redundant groups, favoring the deepest group
    // unless it is a token
    if ((in_count == 1) && (inputs[0].type != OUTPUT_TOKEN))
        return inputs[0];

    // create new group
    output_t *out = test_alloc_outgroup(flags, inputs, in_count);

    return out;
}



void _print_output_tree_rec(output_t *output, int_t depth) {

    // add indentation based on depth
    for (int i = 0; i < depth; i++)
        printf("  ");

    switch (output->type) {
        case OUTPUT_NULL:
            printf("NULL\n");
            return;
    
        case OUTPUT_TOKEN:
            printf("TOKEN ");
            print_token(token_t *token);
            printf("\n");
            return;

        case OUTPUT_GROUP:
            match_rule_str[MATCH_RULE_LEN]
            // skip the "MATCH_" part of the string
            printf("GROUP [%s]\n", get_output_type_str(output)+6);
            for (int i = 0; i < output->gcount; i++)
                _print_output_tree_rec(output->group[i], depth+1);
            return;

        default:
            printf("INVALID_OUTPUT\n");
            return;
    }
}



void print_output_tree(output_t *output) {
    _print_output_tree_rec(output, 0);
}



static match_t test_match = {
    .input_handler = test_input_handler,
    .output_handler = test_output_handler,
    .free_handler = test_free_outputs,
    .tree = c_mtree,
    .tree_len = sizeof(c_mtree),
};


// int test_match_tree_init(void) {
// 
//     assert((sizeof(c_mtree) == MATCH_RULE_LEN), -1);
//     
//     test_match->input_handler = test_input_handler;
//     test_match->output_handler = test_output_handler;
//     test_match->tree = c_mtree;
//     test_match->tree_len = sizeof(c_mtree);
// 
//     return 0;
// }


int test_match_tree(token_t *tokens, int_t token_count) {
    assert((sizeof(c_mtree) == MATCH_RULE_LEN), -1);

    output_t *out = match(test_match, MATCH_TRANSLATION_UNIT, tokens, token_count, token_size);

    print_output_tree(out);

    match_free(out);
}



#endif /* #ifdef __DEBUG__ */



// return value needs to be freed by match_free(...)
// return is determined by output_handler
// NOTE: INPUT MUST HAVE SOME SORT OF NULL TERMINATOR OR INDICATOR THAT intput_handler
//       CAN USE!
void *_match(match_t *m, match_rule_t rule, void **input) {
    uint32_t rflags, **rtable;
    int largest_off = 0;
    void *out = NULL;
    void *input_origin = *input;

    DEBUG( assert((rule < m->tree_len), NULL); );

    rtable = m->tree[rule];
    // first entry reserved for rule flags
    rflags = (uint32_t)rtable[0];

    // get number of rules in a target
    // for (out_len = 0; rtable[out_len+1] != NULL; out_len++);
    // 
    // void *out_list[out_len];
    // int byte_list[out_len];


    // loop through rules until null terminator found
    for (int irule = 1; rtable[irule] != NULL; irule++) {
        void **in_list = NULL;
        int in_len = 0;
        void *out;
        int off;

        // loop through subrules until null terminator found or first no-match hit
        for (int isub = 1; rtable[irule][isub] != NULL; isub++) {
            uint32_t mid;
            void *saved_input;
            void *in;

            mid = rtable[irule][isub];
            saved_input = *input;

            if (mid & MF_UNRES_MASK)
                // if unreserved flags detected, then pass to the input handler
                in = m->input_handler(input, mid);
                
            else
                // otherwise, recursively call match on the subrule 'mid'
                // also LOR with flags from first rule entry
                // also readjust input length
                in = _match(m, mid | rflags, input, input_len - (int)(input_origin - ));

            // if *input was set to NULL, then an error has occured and we must return
            if 

            if (in != NULL) {

                // if match, allocate or reallocate space on list for next entry
                in_list = realloc(in_list, ++in_len * sizeof(void*));
                if (in_list == NULL) fatal(-1);
                in_list[in_len-1] = in;
                
            } else {
                // if no match, then revert back to previous input offset,
                // deallocate input list and all inputs, set to NULL, and break
                *input = saved_input;
                for (int i = 0; i < in_len; i++)
                    match_free(m, in_list[i]);
                free(in_list);
                in_list = NULL;
                break;
            }
        }

        // if in_list is NULL, then skip this rule
        if (in_list == NULL)
            continue;
        
        // get offset from original input position
        off = *input - input_origin;

        // select the rule that consumes the most tokens
        // if new offset is larger than the previous largest offset, then set it to be
        // the new output
        if (off > largest_off) {
            largest_off = off;

            // free old out
            match_free(out);

            // output_t *test_output_handler(uint32_t flags, output_t **inputs, int_t in_count)
            // set new largest out
            out = m->output_handler();
        }
    }

    return out;
}


void *match(match_t *m, match_rule_t rule, void *input, int in_len) {
    return _match(m, rule, &input, in_len);
}
