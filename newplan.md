
Alright, I am looking at this, 

https://port70.net/~nsz/c/c23/n3220.html#A.1

And it seems to introduce the possability of including the lexer in the tree parser, which I find very interesting.

For instance, handling whitespace

whitespace:
    whitespace-char
    whitespace whitespace-char

whitespace-char:
    ' '
    '\t'
    '\n'

optional-whitespace:
    ''
    whitespace


Although I would want a way to separate tokens. Because including optional-whitespace between every token in a matcher would be a big pain.


token:
    dtoken
    utoken

dtoken:
    optional-whitespace alpha-series optional-whitespace

utoken:




Mayhaps if I bake that into the tokens themselves. Though that would end up including the whitespace unless I had a handler system that would be able to exclude that.

I would also ideally want a way to match something, but only return a portion of that match.
For instance, say for a certain keyword, it can be delimited with whitespace, or by a utoken.

Oh. Alternatively I don't need to delimit by whitespace at all. I could just do an alphanumeric match. Easy as that. Whitespace does delimit alphanumeric characters, but so does any other non-alphanumeric character. So whitespace doesn't even need to be considered here.
And then the optional whitespace can be used to handle whitespace that we want to discard between tokens. Unless of course it is whitespace that separates two alphanumeric tokens. I can either handle that by order of processing, or by having an option that separates two tokens by whitespace.

One possible option for handling stuff like this is making it multipass, where it can be called multiple times on different descriptor sets. I feel like this would be neccessary anyway for more complicated languages. So might as well have that be an option.


Also, I should treat function blocks as literals like strings, and allow for function blocks to just be literals, maybe.
Honestly, I would just like it if compound literals could be function types.


Anyway, on to the matter at hand.
What do we want these rules to return to parent groups?
Or I guess the better question, how do we determine their output?
I mean if this is a potential multipass system, where we eventually want to output assembly code, or maybe even optimize, then how do we do that?
First, we need a way for previous matches to send information to the next matches. Also, we might as well just go with a handler again that will write to an output using the state provided, which can be a text output, or it can be to a struct, etc etc.

Now, how would we match to a struct rather than a string? I suppose we could give a match pass a handler that will interpret things for matching for it. And then I guess each match class would have it's own handler for output. It is either that, or have a single handler for output, which might actually be better. It could just be one massive switch case statement. One that is designed with recursive processing in mind.
And perhaps it cannot pass information to previous iterations of it's recursion, but instead has to rely on a different pass stage. And then make it iterative so that multiple stages can work in tandem.


So lets see then. 



##  FEATURES
#################
 - multipass
 - keep track of line and column
 - Matches cant interact with stages of the same pass, 
   but they can with stages of previous passes
 - iterative
 - potentially parallelizable
 - token cache
 - match cache
 - descriptor compiler


Each pass can modify a state table, and output a token set.
Although, does it actually need a state table? Or could each token be injected with metadata concerning the relevant state information they are working under? For instance, determining predeclarations.


I find it worth noting that not all passes need to actually perform matching. For instance, once the syntax tree is created, only one piece of it needs to be processed at a time, alongside some sort of state tracking. But evne then, the state tracking can be backed into each statement, etc.

So basically this system is only good for matching, and maybe even usable for context based matching. 
But ultimately the idea of a multi-pass system is starting to sound less ideal. With the exception of maybe a lexer pass and a parser pass, as matching is useful for both of those, while also creating benefits.

Maybehaps I should have a prematching system. Like, for matching patterns that are guarenteed to be the right pattern if the match succeeds, and then whenever someone tries to check that match, it will quickly return a true. Though I guess that a match cache would work well enough for that. For instance, if we match that something is a token early on, then anything else that checks if it is that will just check the cache to see that it is true.

This does assume some behavior with my system though, which is that it will try to match with the largest pattern, which might be ideal.






It is worth noting that a way more efficient way to perform matching is to group based on first token rather than by full matching statement. So it might be worth considering creating a descriptor compiler later on. But until then... Lets just go with the easiest and most powerfully descriptive way, yes? Something that screams simplicity.


Also, it currently sounds like my current system is still all I need for matches.
I can simply achieve token matching by creating token subclasses, which is simply done with my grouping and handlers.

I think though that the most efficient way to do this would be to separate the lexer and parser stage. And since I already have a lexer, I have no reason to rebuild it yet. But I should add it to the `TODO` list.

I feel like the parser should also handle code generation, because honestly a third stage would simply just do a recursive iteration through the whole thing. That is, unless it just makes the handler functions too big, which it very well might. Also it would probably hurt modularity. So I guess I don't have to do it. Although, my parser should handle context generation. That way the final output stage can work in parallel if I wanted.

I forgot though. Did I want a function handler for every type, or just one applied to the entire tree structure? I figure it would be better to just have one for the entire tree. One input function and one output function.








Out of curiosity, what would a truly prefix type language look like?

int x;

int[] x;
int[10] x;

So how about a function pointer?

*int(void) x;

Basically, how the type works is based on reading it from right to left, like a sentence.
So a pointer to a static array of functions would look like this, 



also, wouldn't an array look like this?

[10]int x;

What if it was read in reverse from the identifier?

int[10] x;

how about a string?

char* x;
char[] x;

How about a pointer to an array of strings?

char*[]* x;

What would that look like in C?

`char *(*x)[]`

How about an array of strings

// c
char *x[];

// meta
char*[] x;

Honestly, fair enough.


So how about a pointer to a static array of function pointers

int(void)* [10] *x;

vs

int(*)(void) (*x)[10];

Honestly, they both look pretty ugly. The biggest difference is that the first is much easier to read and interpret.

// static array of ten function pointers with a static array pointer parameter that return a function pointer

// C complient syntax
(int (*) void)(*)((float*)[]) x[10];

// true reverse prefix syntax
int fn(void)* fn(float[]*) *[10] x;

I think an explicit function keyword is neccessary for true prefix syntax.
Although I guess the pointer makes explicit that it is a scalar return type, and is necceissary, so it might actually work to just do it like this, 

int (void)* (float[]*) *[10] x;
Hmm... No this still doesn't work if I want type groupings.



Also, should that make all type operators be postfix?

For instance, to index the previous,

int y = x[3]*({0})();

Honestly, that does not look bad. It gets the point across about the operators.

The C alternative:

(*x[3])({0})()












Alright, we need to choose a parsing algorithm.
I do like top down. It feels the cleanest one to me. However, it is also kind of intensive without cache optimizations. Plus, there is the iteration issue. There is also the question of conflict resolution. Despite gpt's insistings, I am going to assume that picking the biggest is the best.
But now what about the recursive issue?
Mayhaps a simple self recursion optimization. Any other recursion should be avoided, if possible.
But with self recursion, there is clearly a way to determine when the recursion should end, which is basically when all other submatches do not match. If none of those match, then we have gone one step too far down the recursion hole.

I will be honest though, I would prefer some sort of check be done in the ruleset explicitly, rather than implicitly through the algorithm.
Also the idea of choosing the biggest requires that we iterate through every possible combination of the rules, which does get very bad very fast. So I wonder if I can create a version of the rules that is first match first serve. Caching will still probably be needed here though.

What error should be used should matching the translation unit fail?
Perhaps the one that got the closest match, which can perhaps be determined by the match that contained the most tokens that matched.


Alright, but what is the alternative here? 
Bottom up? I suppose it is also a rather elegant solution in it's simplest form. It could be a rather simple algorithm, that is reasonably fast, and we may even be able to go with first-match-first-serve. I mean, I don't see why not.

Actually, I guess I do see why not. Because first-match-first-serve will not work, even in a well constructed case, because there will likely be multiple different match paths for the same symbol. An identifier leading the tokens can be many different things, and so we would effectively need to check multiple matches before deciding which one. And as a result, we would sometimes need to backtrack.

https://en.wikipedia.org/wiki/LR_parser

Though a way to reduce this is by using a lookahead, which is looking ahead at the next symbol before deciding what kind of match is done with the previous symbols.

It seems like a bottom up would be better at errors too, as a note.
But this lookahead business still seems rather crude to me.


Lets look at the alternative here
https://en.wikipedia.org/wiki/LL_parser

So a top down can use a lookahead too. I am not entierly sure what this solves though. But it might solve the recursion issue.

Also, based off this wiki article, my algorithm doesn't really seem to conform to the algorithm of an LL parser. I think mine is a more general or different type of top-down parser. I am assuing this one though avoids recursion.





I feel like the final stage of my parser should be something that reads binary input, and all token matches are binary with a flag that states it is a binary match rather than a recursive match. Though an output stage is required in case I want to output linearly like for a tokenizer, so we might as well have an input stage as well. 
How should the output stage work though? I think we just go with our current output method, of returning a pointer to the output that gets inputted to the next input call. But I am thinking of doing this only at the very end of parsing to a tree, so that it isn't called redundantly.
But this does beg the question, should it be a single output call, or should it be done for each member of the tree, where the parser handles the recursive output calls?
I suppose it could just be a single call that returns the tree, for someone else to handle. And then to avoid needing to free it myself, the parser will free the tree once the output function returns.



Fixing recursion
```
    a:
        a b
        c

    cbbbbb
    (cbbbb)b
    ((cbbb)b)b
    (((cbb)b)b)b
    ((((cb)b)b)b)b
    (((((c)b)b)b)b)b

    We need the recursion to stop when there is no more tailing b

    If there is no heading c, then no match
    If there is a heading c, then check for c b. If b is a match, 
    then check for a b

    a:
        if c b then
            a b
        c

    a:
        c a b   1
        a b     2
        b       3

    cbbbbb
    1 (c)(bbbbb
    2 (c)((((((((bbbbb

    a:
        c b a


    a:
        if c: a b
        c

    
```

The problem with this is that it is trying to effectively match backwards using a forward parser.
You basically need to be able to look ahead to determine where it ends, as ultimately you need to figure out how deep you need to go. Which I guess is where bottom-up thrives.

I wonder if I can create a recursive bottom up algorithm that aligns with the stack

Lets say each token we consume we call ourselves with that token. But for each token we check we compare it to a truth table of all the match rules, as well as the match rule itself. The match rule truth table marks if all the previous tokens match with any of the rules. The token position and the position of each member of a rule are equivalent.

Ohh, yeah, doing this recursively wont work, as regressing backwards has no correlation with if the stack is increasing or regressing.

Alright then, in that case, 
We check the newest token in the stack with the last match member of all of the rules. If it matches, then we match the second to last match member with the second to last in the stack, and so on. If it is a full match, then we pop those tokens from the stack, and replace it with a single new match token. If there are no matches, then we simply keep adding tokens until there is.
But now, what about handling match conflicts? Is there a way to have the desciptor set in a way that it can be first match?
Lets just assume so for now until we actually run into problems. Because generally I think that the descriptor set is built with this in mind, or at least can be built with this in mind. And that conflicts should generally be rare based on what I have seen from my own C descriptor table.

In case I actually do need to add conflict resolution through, if there is no matches, and we have reached the EOF, then we simply start popping stuff off our stack until we reach our last match token, and we then dissolve our match token into it's components, and then we continue trying to match it from the list, but instead starting at the index that it was originally matched with.
We dont even need to go all the way to EOF, we can just stop once the number of tokens in our stack exceed the number of tokens in any of our rules. We can also use that to speed up matches by checking the number of tokens in the stack with the max number of tokens for any rule.
However, I suppose that is not ideal, as we aren't using the entire stack for a rule. And due to reverse operator associativity, it is very well possible for the entire file to be scanned before our first match is found.

In that case though, what would happen if we were to build from it backwards starting at the end of the document? I suppose it would not be that effective, as the tokens we condense to will generally be preceeding, and we will eventually just end up with the same algorithm except backwards.

So yeah, for now lets just ignore the match conflicts.
