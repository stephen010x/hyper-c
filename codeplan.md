

Alright, so the gcc plugin thing didn't work out. And I am having a really hard time getting into any of the known parsers, so we are back to making our own.

Emphasise modularity, strategicness, and don't worry about being super optimial, yeah?



So I want this system to be an incremental parser. As a result, certain stages of this can run basically side by side, such as the lexor and the parser.

So first, we need a lexer to parse the tokens into a much more processable format.




Lets create a heiarchy here:
- Block heiarchy
- Statement heiarchy
- Token heiarchy


Now the question is, which is delegated to the lexer, and which is delegated to the parser?
Also, how do we handle contextual syntax?

Lets have an overview of our general code formats again.

*function*
type name([params]) {statement}

*branching*
for (statement; statement; statement) statement;
while (statement) statement;
do statement; while statement;
switch (statement) statement;

*struct*
struct name structdef name;
union name structdef name;
enum name enumdef name;


Note that any modifiers such as `extern`, `register`, or any attributes, and similar things, can basically appear anywhere in a statement to have an effect. For everything else, generally an order matters.



Note that all of these can have embedded complexity. So ultimately when processing we would need the ability to group code together that we know will resolve to a certain type of token. So effectively sort of compound tokens.



In what cases do C scopes need semicolons, and when do they not?
I suppose the reason why structs need semicolons is because they have a statement that comes after, which can be blank.
So effectively any syntax that truly ends with a statement doesnt need a semicolon if that statement is a scope block.

I wonder if I can unify struct definitions, declarations, and scope blocks here


structdef {
    int a;
    int b;
    char c : 10;
}

structinst {
    .a = 10,
    .b = 11,
    .c = 'k',
}


codeblock {
    int a = 10;
    a = 20;
    myfunct(a);
    return 0;
}


The problem here is that one architects memory format, while the other architects assembly code. At least the last two are comparable, in that they both would generate strings. But the first is purely a compile time construct used for other things. So I am thinking of the wrong kind of generalization here.

Instead, I should have whatever preceeds the block (which can be nothing), define the parse behavior for the internal block.


Lets try not to overthink our unifying syntax here right now, yeah?



Alright, I suspect that all statements either revolve around a formatting keyword such as for, do, while, etc, 
Or it is going to revolve around a custom word like a variable or function name.
Basically, in C pivots will be pivot keywords and unknown words.



I should probably have this system primarily be callback based.



# Steps
So lets see here, lets say after a simple tokenization, we then incrementally create a tree using opening and closing parenthesis, as well as tokens that also create one layer groups such as comments or strings.

Then we select a pivot. To my understanding, in C all operators, even ones related to types, have an order of operation, so we can use that to determine how we expand out from the pivot (if it is an unknown token type), just about every time.

Using the pivot, or maybe even multiple pivots, we then can properly categorize every grouping here.

Once properly grouped, we can then check the group to a format type from a list, and then further group those



In terms of organizing operators, I can either change to postfix operator notation, or I can group and subdivide more. Considering the nature of opertors, I think subgrouping them sounds like the better idea. I am pretty sure it would be simpler that way anyway compared to needing to convert to postfix.

I think a very easy way to handle operator precendence is to give each operator token a value that represents their precedence, rather than have to check a table. And we just go over the statement and find the operator with the highest precendence.

So the tokenization is just going to have to be simple and have no context.





I think it is worth noting that the parser will need to use a state machine with known variables to determine if a variable is known or unknown, as I think that context will determine how the type is handled.
