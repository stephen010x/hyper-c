

Alright, so in this document, our focus is the hyper extensions we will add to gcc.

First off, I was thinking at worst case this could be an executable that is dependant on a path visable GCC to work, rather than being packaged into gcc itself. Which is honestly fine. Expecially on linux.


# Features
Now, lets talk about the features.

First is features to create libraries with. This is probably rather easy to do. Simply add ways to write actual opengl or vulkan code, and have it be directly imported.

Anyway, lets look at how we want to abstract this. This should be similar to multithreading, except the thread is specified to be on a different and more specilized processor.

So, awknowledging that, there are two fundamental things we need to keep track of. Executable code, and data. Generating executable code is pretty straightforward. We just place the `hyper` keyword at function declarations.

Next up, however, is data. How is data shared, and how is data separated? 
In normal multithreading, all threads have access to global data. I suppose I could do this as well. However, there are dangers associated with this. Though perhaps they are mitigatable using a few extra warnings I could add when compiling.

Now, storage specifiers can be included. Simply prepend a data type with `hyper`, and then it will be "effectively" stored on the gpu. Perhaps with vulkan it actually could reside only on the gpu without needing to ever store it on the cpu as a checkpoint between shaders and the like.




hcopyto
hcopyfro





# Running a hyper function

So running a hyper function requires a function call. This is perfectly fine, it is basically starting a new thread. Calling a function directly implies that it is running consecutively, and that it is running on the same thread. 

Anyhow, there are things one may want to specify when running a hyper function. But first, let us explore what hyper functions are generally used for, which is to transform large sets of data in parallel. That generally describes a shader, which these days can generally describe the function of a gpu.


Btw, `hyper` is not used like a storage specifier while as a function parameter. Rather, it specifies what data is being operated on in parallel. Although I suppose that doesn't really make all that much sense, as a hyper function when called from another hyper function won't treat it like that. And the only interface from the stuff that will treat it like that is a cpu-side thread-like function caller.

In which case, instead `hyper` will not apply to function parameters, nor local variables. And static variables within hyper functions are implicitly stored on the gpu. Instead, we will do this like how threads work, with specified parameters. The first parameter is a `void*` to a varying type, and the second parameter is a `void*` to a uniform type. Which generally will probably just be pointers to hyper storage structs.

Hmmm... I just realized a problem though. The hyper storage specifier doesn't specify which gpu it will be stored on.

Oh. But I do have an idea for encouraging data syncronization. 
Basically, whenever a hyper value is being written to on the CPU, it is awknowlegedly not updated for both sides. So you have to call a sync function on it. For instance, 
`void hsync(void*)`
Where you simply pass in the pointer to the data, which is effectively an ID. Infact, I suppose pointers for hyper types will behave differently than normal pointers. If you tried to take the pointer of a hyper type, it will probably give you something like a handler ID or something rather than an actual address.

Or better yet, I can give it memory that doesn't exist. It looks like I can set pages to protected, and add a segfault intercepter/handler.

There is the question of where hyper variables get initilized.
I suppose this needs to be analyzed with a philisophical perspective. C globals require a static location. A cpu and memory is assumed and effectively guarenteed in all supported uses of C. However, such isn't the case for a gpu. GPUs are technically dynamic, because you may have zero gpus, or you can have several. In other words, this can't be statically determined, and to stay in occordance with the c philosophy, we should not treat it statically.

Perhaps the problem is trying to fit this all into the same executable?
Because executables themselves are dynamically loadable, which would then make gpu globals way more feasable.

Another alternative is to pretend that GPUs can run linear control code that is secretly just cpu code, and to simply 'load' the entire executable to the gpu.
Although considering that the majority of the code will still probably be linear sequential code that is generally cpu ran, this idea doesn't sit well with me.



Alright, here is the dillemma. We can either treat this like we have multiple processors, where we require multiple files to be written. But this comes at the great cost of difficulty to syncronize. However, CPU processors are meant to run independantly, whereas gpus are rather dependant on the CPU for directions. 
The other way is to treat it like a core.



What if I somehow specify a type as a rasterizer type?
Like, I could have a graphics library you can import to my language extension that adds a few extra type macros that specify, for instance, a `raster` buffer type that a hyper function can return data into. Although I would prefer a way to manually trigger rasterization.

Anyhow, I think I came up with a resolution to the global situation that I am satisfied with, which is basically that all hyper globals in the source code are basically initilized on the gpu once the gpu is first initilized for this process.
Which effectively makes sense. Whenever a function tries to access a variable, it should be able to access it.
And then we simply just optimize out unused globals per gpu invocation. I think glsl effectively does this anyway, so I could basically just include them all fine and just let the glsl compiler filter them all out for me.



Also, consider creating a compiler flag to create CPU versions of all hyper functions as a fallback if there is no gpu.








# Vectors

Also, screw it, lets add native vector and matrix types to C. They are both very useful, and also needed for the hyper aspect.







Alright, I think I should be more transparent with my functions. Dynamically interpreting what kind of shader function is used is kind of a violation of C trust, even if it is under the justification of optimization.
Plus, there is really no good way to seguay in a raster() function that is separate. However, I should still include a separate less-optimal raster() function in case in the future there is a way to optimally call it. (Because honestly that would be ideal to have each stage be a separate callable function)

Although... What if I could do this without language extensions, and solely through the library? Sure that is acceptable C behavior?
Mmm. No. It would be incredibly difficult to predict if a raster function is going to be called after a hyper function without static analysis from the compiler. So I guess the ideal case would be that one stage can be called, and before it finishes we decide to route the output to another stage. That way we wouldn't have to delay executing the asyncronous call of the first stage on the gpu in order to detect if a raster stage is called soon after.

Until then however, calling raster separately will be less optimal. And as a result, alternative functions will be provided that will allow you to explicitly run a full graphics pipeline. It will likely be a single function that gets passed a struct.


One question of mine is how to treat global buffers and local buffers.

Also there is the reocurring question of initilization. And I think I have the solution.
So basically, global variables will be referenced by the compiler when compiling to spir-v, as well as various helper functions that can be called by other library functions to assist with setup.
So basically what happens is that when a hyper function gets compiled, all hyper globals that are visable to the hyper function also get compiled into spir-v. And the unused ones will simply be optimized out by the spir-v compiler. So there is no harm in including all of them. And I don't need to worry about them.

But this also solves initilization, as it is effectively instanced to every entry hyper function. (not really sure how I will differentiate hyper entry functions though)
But basically you simply just need to initilize the hyper function on your target gpu, and all the referenced globals and helper functions will effectively be initilized as well. First the spir-v compile will optimize away all of the unneccissary junk, and then we can query the spir-v program for variables and functions that still remain in order to generate the CPU-side helper functions to intilize and change things. We can then package this into a location that is effectively pointed to by what is generated to be the pointer of the entry hyper function that we pass to a library function that will simply call the helper setups and whatnot, etc.

Global buffers are generally buffers that are expected to be shared across different hyper functions or shaders, wheras buffers passed via the entry function are more specific to that shader.


__Alright, so two more questions that need answering then. First, how do we determine the entry functions. Second, how do we handle shared cpu/gpu storage objects, as well as syncronize changes across them? And finally, how do we manage trying to read from buffers that are also currently being modified in a hyper function?__

Also, we should also have a way to initilize and modify gpu-only data that doesn't reside on the cpu. We should include an alloc function that can allocate arbitrary buffers on the gpu, as well as read and write functions for those buffers.
At this point, I wonder if it is even a good idea to allow for cpu/gpu shared objects that aren't explicitly managed by the gpu alloc function and sync functions.

Nah. It doesn't make sense in C to implicitly manage two storage locations as one. So they should be treated separately and synchronized explicitly by the programmer.
Basically, if a programmer wants to sync between two buffers, they should explicitly create two buffers with different storage classification, one for the cpu, and one for the gpu, and then call a sync function to syncronize those.

I think for determining entry functions we should use one of the existing C keywords.

volatile
static
restrict
inline
extern
default
const

__attribute__((spirv_entry))

ugh. Alright, I guess I will add an `entry` keyword. It seems like such a waste though for how seldom it is to be used, and always has to be used with `hyper`.
Maybe it could be called `entry`

`entry hyper vector3 myfunct(void*, void*);`


Also, if there is a way to actually treat memory and buffers as indexable with pointers, that would be awesome. For instance, if I simply import all of my uniforms using a uniform buffer, it could effectively be taken pointers of and indexed into.

Actually, wait, what if I just use hyper for entry functions again. So basically we don't require hyper... mmm... I don't like that. I still would like to make it explicit if a function is getting compiled to spirv rather than x86.

One thing to consider is simply if it fits the pointer format for an entry function. But that is a pretty weak way to do this.

Another alternative is specific names. For instance, anything that starts with a main_ would be treated like an entry, which so far is the best idea.

`hyper vector3 main_myfunct(void*, void*);`

Yeah. I actually do not mind this.
And then I will also include the attribute along with a library imported macro keyword for those who want to do it other ways.




# Alright, big thing, 
If GPT is not lying to me, modern glsl supports incoherent image read and writes. 
This dramatically reduces many limitations imposed here. For instance, I can now have global variables be written to by the hyper function. Basically, globals can be split into two sections. The uniform buffer, and an image buffer. The uniform buffer for variables that are never modified by the shader, and the image buffer for variables that are modified by the shader.
And of course I could have a library that import various atomic read and write functions, and of the like.

What's more is that I can also now have fully functional loops and conditionals. Like, this is basically unrestricted behavior at this point.

**Although, I should add behavior to fix dFdx and dFdy issues when within conditionals**




# Pointers
Alright, so I have a pretty neat idea for hyper pointers.
Basically, hyper pointers will be 64 bits, generally. Although I suppose a 32 bit version could be devised if neccessary. But generally the higher 32 bits of a pointer will represent a buffer id, whereas the lower 32 bits would represent an address in that buffer. And this would basically represent our virtual address space on the gpu.
And for future possible extensions, the address could instead be the higher 16 bits be the buffer, and the lower 48 bits be the address in the buffer. 
Or honestly I suppose the most optimal layout is the first 24 bits be the buffer, and the remaining 40 be the buffer byte index.
But anyway, for now it will be a 32/32 split.

Now I suppose there will still be limitations. If the compiler cannot predict that you will try to access an out-of-range pointer or buffer specified by a pointer, then it will of course produce a spirv runtime error that my compiler may not have much control over.
Then again, maybe I do, which kind of reveals an inefficiency with this, which is that every memory access would require me to also process which buffer it is referring to, which is unideal. Generally buffer indexes should never try to access a different buffer. So perhaps instead the glsl side of the pointers will be limited to 32 bits, whereas the CPU version of said pointers will be 64 bits to indicate which buffer.
Next question though, should I allow byte addressing?
How about this? Buffers are generally stored at the data size they are initilized at, and any attempts to treat it like a different type via a cast will simply just add a bit of extra processing overhead to bitwise read and write said data, along with calculating the correct indexing address. Not exactly a huge overhead, and something that is avoidable if the programmer knows what they are doing. Generally, doing it this way still leaves it up to the programmer how they want to optimize this.





Add a printf/debugging library for hyper functions, with maybe either some debugging attributes to add to functions, or simply a compiler flag for debugging.

Also I should add a bunch of tertiary libraries to help with more optimal shader code. For instance, functions for shadows, and functions for parallax, and etc.


It looks like opengl has functions that can persistantly map buffers to user memory. So I suppose that I could introduce some of my own that would work hand-in-hand with the `hyper_hybrid` attribute for variables.
I could import a `hybrid` macro alias for that attribute as well.




Back to pointers though. What do we do if we take a pointer of something, and then dereference it? We would need a way to preserve which buffer it points to, so we could at compiler time retain the top 32 bits of the pointer, and use that value to determine which buffer we index into when it is dereferenced. But it is compile time stuff, not runtime stuff, so there is zero overhead. And also when someone takes a prointer and converts it to an integer, the higher 32 bits of the buffer address is included.
But what happens when someone tries to modify it? 
Well, I guess we might as well allow it by adding runtime code. Might as well allow people to do this, but add some compiler warnings, as it would add runtime code.
Although I need a way to differentiate when the user is directly trying to edit it, and when they are not. And static analysis probably isn't going to cut it.

Perhaps, if I justify a different behavior for hyper pointers.
Ahah! That is exactly it! 
I can still keep this within C standards by saying that hyper pointers have a certain arithmatic behavior, which is that it is basically a 2D vector, and all operations generally mimic that. All additions, multiplications, and etc, will be treated like a component-wise operation. And adding values like integers to a hyper pointer will treat said integers as two different high and low components.
Of course, ints and shorts will only ever add to the lower component. Only 64 bit integers will, unless you cast the pointer to access the higher bits.

How will CPU code interact with hyper pointers? I suppose they will interact with it in the exact same way. Only trying to dereference it might cause compiler errors, or just point to normal memory. More likely compiler errors.

Hmm. But what about input parameters to functions. There is no way to really predetermine what kind of higher 32 bits that is going to be.


Alright, I guess we are scrapping this idea. However, there will still be specific hyper pointer types.

So what kind of buffers do we have here? 
We have uniform buffers, image buffers, and then local variables, which may be too inefficient to store in a buffer. And these would generally associate to a `hyper* const`, a `hyper*` and a local `hyper`
And I was thinking we simply just don't allow you to take the pointer of a local hyper variable. I just cant see any other way around it.
Maybe I can just make it so that if a pointer is ever taken that it will create a fake stack array and just reference all local variables from there. Although that wouldn't really work for the input parameter.



Alright, I relent. Maybe no pointers, yeah?
I mean, maybe if I could get pointers from parameters. But that isn't really in the cards.
Alright, lets just list it as a tenatively and hopefully future feature. But for now there will simply be no pointers, at least in the first version of this.

Though there is just one problem. If there are no pointers, then what exactly are my function parameters?
I suppose there is no point in having those be pointers, and just have them instead be actual objects being fed in. However, that comes with it's own problems. For instance, I can't justify it as being arbitrary. So I guess that is the purpose of treating it like a pointer, as it is now effectively a pointer to an arbitrary buffer size which is simply just a buffer that is populated with the data.

Alright then, this may be a second revival to my initial hyper pointer idea.

So effectively I should treat everything as virtually memory mapped independant of function parameters. The function parameters will simply be pointers to these virtually mapped spaces.
In order to improve optimization, I will introduce specific hyper pointer types who's higher 32 bits are predefined constants, but are generally not needed to be used by the user.
For instance, `_Hyperptr_input`, `_Hyperptr_uniform`, and `_Hyperptr_output`, etc.
Though I suppose I might as well make the entire thing constant. So they are enforced const types that can only ever be a single value that is known at compile time. But while they cannot be changed, their rvalues just decay to an ordinary `hyper*`.
So ideally you will want to dereference them in all uses rather than copy the pointer to dereference that.
Mmm. One bad thing though is that the programmer will have to specify these custom types for the function prototype, which is not ideal.

Oh! But what if I use the legacy feature to prototype a function with unspecified parameters?
`hyper vector3 myfunct();`
with no parameters, in C this means that there are an unspecified number of parameters. I can then use this to justify functions with more absolute parameters. At least, the input parameter could be direct, and still have the uniform parameter simply point to the larger uniform buffer. And then the function that starts the hyper function from the cpu side will simply use the size parameters it was passed to "determine" how the data is passed to the hyper function.

Though there is still a problem if I wanted the uniform buffer to be a pointer. Hmm...
As well as the issue of passing pointers to other functions.
I could try to provide some simple form of static analysis by allowing for pointers to be passed via parameters, but checking if either the top or the bottom are ever modified or potentially modified. And this will effectively be a form of compiler optimization. And this could be used to basically treat pointers as statically pointing to a single buffer or instead being a compile-time known static pointer and of the like.
What's more, is that we can basically guarentee to the compiler what the values of the input parameters are going to be. So we can just optimize those as well. (I could probably just do a pass that does simple static analysis to determine which pointers are actually just pre-baked values, and could just replace them with such).

Anyhow, this would allow for complete memory mappings. Lets define them here.
First, lets just have `Buffer 0` be a null buffer. 
`Buffer 1` are our inputs, of which there will basically only ever be one input.
`Buffer 2` is our constant globals (the uniform buffer)
`Buffer 3` is our dynamic globals (image buffer)
And then the rest are dynamically generated via scopes.
The entry function is effectively guarenteed to be `Buffer 4`, so we can use that for optimizations.
Latter functions though will be tricky, because I want to treat it like stack scoping. And evidently spir-v can have functions with their own stack frame.

I think only variables that get referenced will be placed on my own pseudo stack frame, and I will effectively treat the rest as being optimized out.

But back to the tricky nature of subsequent function calls.
So how about this: The compiler will treat all references and dereferences as compile-time mechanics only. And it is only when either the top or bottom half of the pointer is modified and dereferenced or used in certain contexts do we need to inject runtime code.
But in many cases, the actual pointer aspect gets optimized out, and the variables get referenced directly by the function.

And this isn't just with function parameters. This is simply dereferencing any local function variable.
And I will just have it so that function parameters are treated as the first items on the local function buffer unless optimized out.





Note look into the #line directive for the glsl to spirv compiler to see if it is supported for debugging purposes.








Alright, pointers really are just infeasable right now. So lets just not allow pointers at all, yeah? The exception being when the pointers are only used for casts or calulating a single dimensional array index for multidimensional arrays, and stuff like that.
So basically, any pointer that involves actually taking the value of a normal pointer.

This generally means that 1.0 entry functions will not be pointers, but direct data. Which is fair, that is basically how glsl does it or whatever. Or something similar, idk. 

Anyhow, this means that function templates will have to be more lax, and they wont catch you trying to pass the wrong type. But this still works for C, as it is still very possible to populate the parameters of an arbitrary function if the method that populates it knows what they are, where they begin in memory, and the calling convention. So this will effectively behave the same way, where the provided library functions know the secret formula, but won't stop you if you want to try to do it all by hand yourself if you happen to know the calling convention.

Should I just use `hyperlib` as a prefix for my library functions?

`hyperlib_start_threads(hyper_ctx_t *, void *h_call, void *h_in, int h_in_size, void *h_uni, int h_uni_size)`;














###############################
## Old text
##############################

`hyper` functions can call non-hyper functions. The compiler will automatically compile an alternate spirv version of every function that is referenced from a `hyper` function. This is mainly to preserve library compatability. However, this will result in a compiler warning unless disabled. Generally the only functions a `hyper` function should call is other `hyper` functions.



These support most of the common C arithmatic operators, which I suppose shall be defined later. 
I suppose we could also support swizzling, as swizzling can be recreated using standard C struct syntax.
Well, almost. I don't think it could support rearanged versions, like yxz, which is making me reconsider swizzling. It is certainly bordering on non-c-like behavior.

Also plenty of undetermined functions for extra vector operations will be included in associated libraries.

The swizzled types can be:
xyzw
rgba
stpq

Yeah, I suppose *I shall add no swizzling* for now. It just doesn't really justify itself very well as c-like behavior. Plus, you can already specify them independantly. So that should be enough for now.
You can still access them individually using .x, .y, .r, .g, etc.
