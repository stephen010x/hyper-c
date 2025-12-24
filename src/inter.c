
/*
 *
 * This is the intermediate portion of the code translation. 
 * Ideally I want to simplify the output as much as possible, so as to allow
 * for final outputs to have a much easier time generating code.
 *
 * this will take the rather bloated and redundant syntax tree
 * and convert it into a behavior tree
 *
 */




/*

So lets plan here what kind of output we have here.

Basically, we probably separate everything into statements. I think I will keep everything
in a tree linked format, even for sibling statements, as it will probably ultimately simplify
output generation.

But lets see here, we have a translation unit, which is filled with external declarations.
Each external declaration can be function definition, or a declaration.

Lets just build our tree format here:

translation-unit:
    declaration:
        type:
            scalar:
            struct:
            union:
        identifier
        initilizer
            type:
            data:
                constant:
                expression:
                    ...
    function-definition:
        compound-statement:
            statement:


I mean, lets think about this fundamentally.
We have a scope
We have certain statements such as functions, conditionals, iterators, etc,
We have an expression, which is math and assignments (reading, editing, and writing data)
We have references, which are variables, with type enforcement. They represent scalars in some form.
And then finally we have constants, such as strings and static arrays.

And I think that is generally it.

Since this isn't yet context based generation, I feel like this could be achieved with another table.

For instance, 

primary-block:
    compound-statement
    selection-statement
    iteration-statement


iteration-statement:
    while ( expression ) secondary-block
    do secondary-block while ( expression ) ;
    for ( expressionopt ; expressionopt ; expressionopt ) secondary-block
    for ( declaration expressionopt ; expressionopt ) secondary-block


Lets say we have our for statement here. We want to combine the two for statements, and we don't want the iterator group, but instead have the interators be grouped as a primary block.


Note: that if you wanted to have these three types of iterators separate, then you should 
create separate rule matches for them. So don't bother adding extra functionality for that.

iteration-statement: 
    input {
        A: while ( expression ) secondary-block
        B: do secondary-block while ( expression ) ;
        C: for ( expressionopt ; expressionopt ; expressionopt ) secondary-block
        D: for ( declaration expressionopt ; expressionopt ) secondary-block
    }
    output {
        // [0]  [1]                    [2]        [3]        [4]
        // type declaration/expression expression expression secondary-block 

        A: [0] ( [1] ) [4]
        B: [0] [4] while ( [1] ) ;
        C: [0] ( [1] ; [2] ; [3] ) [4]
        D: [0] ( [1] [2] ; [3] ) [4]
    }



We also want to be able to collapse groups, mostly redundant ones,

I wonder if it is possible to seperate a declaration into two objects.
The first is a declaration without initilizer, 
and the second is the identifier with the initilizer assignment;

I could probably allocate a section of the flags for the output arguments
OUT_8
OUT_9


Before we do any more though, I should write the match function, and have a debugging output



Actually, new idea. Noncontextual direct output, with some help from an output function.


For instance, 


iteration-statement: 
    input {
        // c
        A: while ( expression ) secondary-block
        B: do secondary-block while ( expression ) ;
        C: for ( expressionopt ; expressionopt ; expressionopt ) secondary-block
        D: for ( declaration expressionopt ; expressionopt ) secondary-block
    }
    output {
        // lua
        A: while ( expression ) secondary-block do
        B: do secondary-block while ( expression ) ;
        C: for ( expressionopt ; expressionopt ; expressionopt ) secondary-block
        D: for ( declaration expressionopt ; expressionopt ) secondary-block
    }

Yeah, that isn't working as well as I hoped it would. The problem is that it can just be so
structurally different that a simple mirror such as this doesn't really help. Basically we need
an intermediary here again.

I think simply condensing the number of classes might be the play here. I imagine half of these are basically intermediates. But even that is rather finiky.

Mayhaps a lot of these can be grouped. For instance, anything that ends with a list can probably all
fall under a grouped case statement. 

    case MATCH_MEMBER_DECLARATION_LIST:
    case MATCH_SPECIFIER_QUALIFIER_LIST:
    case MATCH_INIT_DECLARATOR_LIST:

    And then maybe have two for space separated and then comma separated lists. Although plurality might
    be the difference between the two, but anyway, we would then just output a collapsed expression.

    I am going to be honest though, I feel like the output needs to be recursive for this to work.
    I suppose if our match will pass in nested args to our output. But that would require the output
    to return it's current output.

    This wouldn't be a problem if the output was binary, but we do need to return pointers here for linked
    struct output.

    Lets just go with returning a pointer for now. Philisophically it makes sense.
    We match with a statement, which will then pass either match with a token, or 
    an output from a previous deeper output call. And it is the output's job to combine
    those into a group and return that.

    The alternative is for the output to handle the entire output itself. But can it reasonably 
    do this recursively? I don't think there is any way it can keep track of it's depth and whatnot.
    So I suppose we will have to rely on the first idea.

    Alright, how on earth would I pass either a flag or an object?
    Perhaps I need to rely on the input handler to return a token packaged in it's own type of object.
    Yeah, that might work.

    Alright, the good thing about this system is that it is still capable of generating a string.
    It could literally just pass around a string.



*/
