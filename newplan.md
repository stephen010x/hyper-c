
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
