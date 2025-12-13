

Alright, so how do we plan to process things?
The idea is for it to use C syntax. I think there should be two modes.
The first mode is that it only compiles things with hyper specific attributes into more c and vulkan code. This is probably the best way to do hyper-c, as I don't think I could keep up with all the language specifics.

This way for the second mode I can just have it be meta-c, because I suspect that my meta-c will be much easier to parse whatever edge cases than ordinary C.

Anyhow, first things first, we create our own somewhat-c compiler to get things started and warmed up. I am not really sure what we are compiling it to. Just anything really. Probably another language rather than assembly. Or perhaps just a pseudo program.

Anyway, how do we plan to parse language stuff?

Lets look at an example

    __share int bitwin_new(bitwin_t *restrict bitwin, bitwin_prop_t *restrict prop) {
        int      xscreen;
        Display *xdisp;
        Window   xrootwin;
        Window   xwin;
        
        XVisualInfo *xvisual;
        XSetWindowAttributes xwin_attrib;

        // set opengl attributes for display
        int glx_attribs[] = { 
            GLX_RGBA,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 0, // No alpha buffer
            GLX_DEPTH_SIZE, 0, // No depth buffer
            GLX_DOUBLEBUFFER, True,
            None, // basically a null terminator for this list
        };

        return 0;
    }



So first of all, we need to figure out what separates statements. 
I think the best way to do this is to basically do a sort of processing where we process each token until it matches some sort of complete thing. 

In C, there seem to be two different things that create a complete statement. The first is a semicolon, and the next is curly brackets, which don't need a semicolon, and can represent a structure that can contain more code.

For instance, we process the function definition until it is a complete function declaration, and it is recognized to be one.
Lets look at this in the eyes of meta-c for a bit. Or maybe even just normal C. 
Essentially, functions are a type. A variable, if you will, that has certain operations that work with it, such as calling it. But it can also be used as a pointer. Past that is the brackets for defining the code for the function.

Also, the idea that curly brackets can contain sub-structures, as well as sub-code, feels similar enough that I should try to better merge the concepts.

Anyhow, C-style functions feel off to me. They just feel like a large outlier to the behavior of many other things in C. What if I compare them to a loop?

    for (int i = 0; i < 10; i++) {
        code here;
    }

Loops behave the exact same way, don't they? They are some sort of statement followed by a bracket that contains sub-code. And this statement defines what to do with the sub-code.
I suppose I can work with that then. It feels like a uniform enough way to define behavior for code blocks that I will probably try to extend it to meta-c.
On that note, I think I have a justification for why they don't need semicolons. And it is because those code blocks aren't a true code statement, but instead code organizers. It is a scope, essentially.

For instance, you have all of these statements

    statement1;
    statement2;
    statement3;
    statement4;


And then we can add a scope to separate the scopes of the statements. But realize that the number of statements don't actually change, and therefore we don't need any more semicolons.

    {
        statement1;
        statement2;
    }
    statement3;
    statement4;


And I suppose one good reason why this works is because brackets have a beginning and end, and therefore don't need a closer such as semicolons. But this contrasts when using semicolons for packaging data structures, as assigning structures is an actual code statement. And it is needed for declaring instances of a structure.


Still, the format of loops and functions are a bit odd. They aren't a cast, but simply precurse a code block, basically stating how it is to be treated. And this is detectable because anything otherwise will end with a semicolon or another closing bracket right behind it.


Lets accept this format for now. But lets look into a few things here.
For instance, function definitions decaying into pointers, along with anonymous functions

    void* myfunctptr = int (char *string) {
        // definition here
    }

But wait, does that imply that loops can decay into pointers as well?
Hmm... that seems a little stupid, to be honest. So lets just say that they can't decay into pointers?
But wait, in my meta-c document, I say that I should allow scopes to decay into pointers so that executable code literals can be placed in array definitions. So I guess then yeah, they can decay. If only for the sake of the philosophy and completion.




Anyhow, another thing about this language is that everything is compiled in order depending on what section each type goes to.

So, like, we have the text segment, and the data segment, in terms of compiler output.
That is basically like, the code and read-only data in the text segment, and then all of the global and static variables in the text segment. And basically what I plan to guarentee is that the order in which they appear in the code is the order they are placed in their respective segment.

This also includes variables in the stack sgement, which is dynamically allocated based on scope. The order in which they appear in the function are the order they are placed in the stack structure for that function.

And this is all basically because everything is a struct. But how do I work though struct syntax if functions are technically a struct?

Lets look at an example function


    int main(void) {
        int i = 0;
        char *hello = "hello world";
    
        printf(hello);

        return 0;
    }


I am going to be honest. Tying the bow of a "struct" on this will be difficult. There is no explicit indication that a struct is allocated on the stack. And there is no explicit struct keyword.

On that note, I probably should maintain that structs are explicit. Otherwise I would have to get rid of the struct keyword, and I really don't want to do that.

Alright then. So a struct should just be another type, like functions. Only, structs require semicolons at the end of their statement wheras the others don't, because instances can be initilized.
This implies that function pointer instances should be initilized at the end too.

    int(void) {
        return 0;
    } main;

Ahah! I was thinking that this wasn't really that consistant with structs because of the beginning. But then I realized that maybe it was.
A struct is a sort of type declaration. But so is int(void)... well, almost. It still violates how C treats types, which is that types are declared how they are used.

Although it is worth noting that function blocks aren't types. They are instances. So that further contradicts the syntax here. It would also make sense for the name declaration to be before the block, because typically the definition of a value comes after the value indentifier.





What if we start from scratch here, as if from assembly?
So lets say we have block scopes here. We also have statements that indicate what code is to be generated for constructing and deconstructing the environment for these block scopes. These are functions, loops, etc.
function, for, do/while, switch/case

Dang. Do/while and switch/case are rather strong outliers here. How do I handle them? Also, you know what would be cool? Is if programmers could define their own scope block generators. like for and switch/case. They are basically compiler directives for generating code.

How on earth would we unify this though?

    switch (number) {
        case 1:
            printf("Number is 1\n");
            break;
        case 2:
            printf("Number is 2\n");
            break;
        case 3:
            printf("Number is 3\n");
            break;
        default:
            printf("Number is not 1, 2, or 3\n");
            break;
    }



    do {
        printf("Count is %d\n", count);
        count++;
    } while (count <= 5);


Another option is to consider each of these independant code statements in how they handle their block.
It is worth noting that these all can handle actual statements without a scope block. (everyone except functions at least)



Anyhow, lets say that scope blocks and struct blocks are completely different. One defines scoping rules for code, and the other are namespaces for variables.

Ohhh, but they are still so similar! But I can't unless I get rid of the `struct` keyword!

Structs are templates, scopes are runtime
Structs are static and uninitilized, scopes are runtime
Struct instances don't decay, scopes decay into pointers.

Honestly, structs should stay the pure data packing that it always was meant to be, with maybe only the exception of the `meta` keyword for creating imaginary values within the struct namespace.

Hmm... I suppose another similarity can be drawn again, which is data blocks. 
A scope is a data block that represents both a variable block, and a code block, respectively.
A struct literal is a data block that represents only a variable block.


Hmm... Maybe scopes shouldn't decay to pointers. Because they can be used in place of statements for loops and stuff.


On that note, lets look at our generation types with just statements

for (;;) statement;
while(1) statement;
do statement while(1);

Alright, the only way I can justify the do statement is if it was it's own thing. Allowing for 
do statement for(;;)
The problem with this though is that the initial declaration in the for loop comes after the block. So it kind of implies that it is not visable in the scope.

I can justify do being a statement that requires a semicolon. Though it seems to be able to accept both a scope and a modifier, which means it can't accept a statement of it's own
So `do statement while(1);` is actually invalid.




Alright. For now lets make a normal C compiler, and then add features to it later.
We want a C compiler with a `hyper` gpu extension, and an extension to manually link along with better assembly support, as well as defined code placement behavior.
