


Alright, so no pointers. Entry functions have unstrict parameters.

What kind of output do I need to do here, and can it be done independantly from a library?

First, there is the spirv output, mayhaps with injected debug info. What else do we need to know? The varying inputs are determined by the programmer, and the uniforms are buffers.

Whether uniform buffer objects are static in size or non-static, we can still safely determine their size using struct and object definitions. Since struct definitions can be shared between hyper and non hyper contexts, it makes type safety for this rather trivial.

Of course, we will want to add optional runtime checks and walls for debugging.

But generally, I guess all we really need to do is compile spirv. No extra struct data is likely needed. For now anyway. Everything else is up to the library. 

Now, I should add some helper functions (via the library) for those who want to do this using a normal library like opengl or vulkan, so as to allow for them to pass data arbitrarily without writing their own functions to do that. 



Oh. I should add a way to include inline spirv or inline glsl, and stuff like that. This way I can create helper macros or hyper functions that can run some of the more ingrained stuff, like the fragment derivatives, and stuff like that.




# The graphics library

So I guess we are now onto the library aspect of this. First off, lets look at all the things we need to bind.

Input buffer
Output buffer
    framebuffer
    

Oh. How are we going to treat the creation of programs?

I suppose before running a shader we must first load a shader. I think initially I was going to have that be baked into the code, but recent developments, and the call to C-like behavior, is to have this be apart of the library now.

So the question is, should this be done by the user, or done by the library? 
Ideally I wanted to just have a simple thread-call-like function to run whatever spirv code I had. Which I can honestly still do through the library by utilizing caching. However, this might be rather unideal for the programmer, so I suppose I have little choice but to allow them this. 

But, I should add an optional helper call that skips that process, and just loads them dynamically with caching, for those who don't want to have to do that extra setup.

In any case, the extra setup should still be minimal.
I just need to add a function to upload and deload.

Oh. But I forgot about pipelines. See, we aren't just uploading shaders, we are uploading groups of shaders. Or in this case the whole pipeline. Well, we are creating the pipeline, not so much uploading it. But this means that each combination of shaders will require it's own pipeline. 

Honestly, I don't want this to be apart of my library. Or at least, I can add functions so that it is optional.

Actually, I guess it is fine to be apart of my library, considering that there is basically already a distinct function for launching a graphics function, I might as well create a way to compile multiple functions into their own pipeline.

Actually, I guess it could be the exact same function for uploading them to the gpu in the first place. For compute functions, you simply just upload them using the standard upload function. And for graphics functions, you upload them as a group into their own pipeline. Makes sense, honestly.


I should separate the hyper graphics function from the normal hyper functions into their own headers. 



Note from Trevor, 
The pipeline also includes a renderpass, which is just information about the image size, and image type, etc. If a renderpass is included in the pipeline, then it is baked into the pipeline and cannot be changed. However, you can not include a renderpass in the pipeline, and determine the renderpass dynamically via specifying it in the command queue or whatever.
Baking it can be faster, but usually not by much.



Anyway, for now lets keep the renderpasses dynamic, as I want to keep the input/output buffers independant.


On that note, how should I attach buffers to the pipeline?
Should it be explicit on the user, or more indirect and managed by the library?

Alright, the uploading to the gpu makes sense, I could dismiss that. But I genuinely need this to feel like you are inputting a buffer, and getting a buffer output. The user should not feel like they are attaching anything. And if I can get away with it being as fast as anything else, I want the user to explicit the input/output buffer each call.


You know what would be nice, is if there were macros that could process macro functions in a way such that we could basically bake all of the static hyper function starts into a single command queue.


Anyhow, I should just have a dynamic queue like in opengl. And then maybe also add a way to submit a queue of hyper run commands as well as a way to optimize.
Also have a way to specify a queue of input/outputs for each function.






Also, sick idea for debugging shaders, is that when you set it to debug, it can generate a C version of the shader so that you can just debug it using gdb.






Hey, I forgot something! Which is varying data that isn't explicitly tied to a buffer (but probably implicitly) that is passed from one shader stage to the next.

I want to avoid descriptors at all cost. That eliminates multiple input parameters. Although I suppose I could just generate descriptors for that.
But also there can only ever be one output. 

I don't want to use globals, because only the inputs and outputs are suppose to be strictly instanced to each object on the buffer.

Also, according to gpt, you can bind multiple input and output buffers. Which can all technically be different sizes so long as you never overflow any of them when accessing them. (So they will all effectively be treated as the same size)

On that note, add a way to start from a certain index in the buffer(s) as well.

But this creates a new dillema. One that if solved would solve the raster problem, which is reading and writing to multiple instanced buffers.

I did have an idea to solve this though, specifically for rasterization, but can probably be extended to this. 
That is, if I use function calls.
I know, it isn't exactly ideal. But lets consider it. We want to output data to the rasterizer like we are filling a buffer exclusively for it. So what if we just call a function that will do that? It is something simple, like, `hyper_toraster(vertex);`

or `hyper_out(HBUFF_RASTER, vertex);`
`hyper_out(HBUFF_3, value);`

Oh, but crap. Functions are typed. I would need to create a function type for each one. Or I would just have to use a size.

`hyper_out(HBUFF_RASTER, vertex, 12);`

Now, I don't want to forsake the return value for a function call. So I think I shall have buffer0 be the return value, and then have extra functions to write to more output buffers.

Now, the question is if I want to do the rest of the inputs to be parameters, or also be function calls?



How should I handle `discard`?

One downside of a return as opposed to simply writing to a value is that we are forced to return a value for every one.
Now, I do suppose I could just implement `discard` in the library instead of the extension, which is probably good enough. Although ideally I would want something that feels more elegant and inline.

I suppose having a discard like that is fine. Discard only appears in fragment shaders, not anywhere else like compute shaders. Because in other shaders you have a guarenteed input and a guarenteed output. But for fragment shaders you are overriding data on an existing buffer per draw call in order to add drawings to the framebuffer for each mesh you call it on. And as a result you need a way to preserve the previous value. And this is probably most efficiently done by not writing anything at all rather than simply a copy-write.

And I suppose ideally I would want all of my functions to behave that way. That is, on the assumption that they are adding new things to an existing buffer, rather than emitting a new buffer.

Ugh. I really wish I could have pointers.

Maybe provide options here. A return value can be that. But the other option is to be a buffer-only scheme, where designing using the direct ID will optimize it to an input/output buffer. Although this will heavily encourage unoptimal use of the buffers, as it would make it seem like arbitrary buffer access is equally as optimal as id indexing a buffer.



But anyhow, I first need to find a justification for this multi-scheme. I suppose I can have a separate function for each type.
Besides, each input buffer guarentees an input, so why not have each output buffer guarentee an output as well?
And then whenever you don't want that, just have a void output, and just use normal indexing buffers. And unlike the inputs and outputs, you can both read and write these buffers. (Of course, remember to include atomic stuff. I imagine something like a mutex buffer might be useful for more advanced stuff.)


Anyway, I guess we don't even need a different function for each one. We simply have inputs and outputs, and both the inputs and outputs can be void or null. And we can always access arbitrary buffers. They can be passed via the uniform object parameter, or simply as a global.



Anyhow, I have discovered yet another problem. Which is that the spir-v compiler requires that you know what shader type you are compiling for (it determines what entry point it uses).
There are several possible solutions to this. But the foremost thing to keep in mind is that by default it must appear runtime based for the user.
1. The first solution, which is probably the least ideal, is to bake it as glsl rather than spirv. I would honestly prefer not.
2. The second solution is to default everything to compute shaders, and just add function attributes that determine if otherwise, which is generally neccessary for the graphic pipeline functions. 
3. The third soluion is to compile an entry point for each of the common shader types. (That being compute, vertex, and fragment shader), and to add hint attributes for the compiler to only compile one type, or even a rarer type like the geometry shader or one of the other shaders.


I am honestly split between the last two options. Though I suppose the third option has way more potential for problems and optimization issues. (Although I can potentially try both of them. Maybe add a compiler flag and test the efficiency and binary size of each one. Of course, there will already be a flag for the first option)

Alright, I guess explicit behavior wins again. The C standard would be proud. 
So I guess by default the library will be compute shaders, and the graphics parts will generally be excluded. (Although I imagine it will be just as easy to use the compute shaders for graphics). The only major downside I see is the raster stage.
**Of course, remember to test a version of the pipeline that uses a compute shader for fragment processing, followed by an independant raster stage, followed by a compute shader for the fragment stage**

`#define fragment __attribute__((fragment))`




Alright, and back to the other problem, which is treating inputs and outputs. After some thought, the idea of treating outputs as non-interruptable makes sense. Philisophically, you shouldn't be able to not write to an output when you are streaming an output. It makes more sense for an arbitrary buffer. (Although, I should add a macro function that will allow you do to so anyway. Just because.)

A possible word for this command, 

exit
bail
abort
discard
_skip_


Alternatively I can just allow `return void` in non-void functions. However, this kind of is against the C standard, and is against my philosophical justification earlier, so I should just keep it to a function.

Besides, it makes sense to be able to create a function that returns without returning anything in CPU-C, right? Like, it is generally possible to create an assembly routine that a `skip()` macro would run that would essentially return a void that would generally keep whatever register the same that it initially was. Right?

So yeah, I guess I can philisophically justify keeping such a macro.

Also, I like `skip`, so I think I will go with that.

I guess `hskip()` to avoid conflicts, if possible. And it will effectively just inject a glsl `return` value into your function.

I should also just include `hout(buff, size)` or `hout_32(buff)` and `hin(buff, size)` or `hin_32(buff)` macros for extra buffer streaming inputs and outputs that are explicit.

Addressing arbitrary buffers as streaming only will still optimize them to be streaming buffers. But for when you want to be explicit.



# KEYWORDS
[Streaming I/O Buffer] - Buffer that streams input or output
[Arbitrary R/W Buffer] - Buffer with arbitrary indexable reads and/or writes

What should I call uniform data?
- _Uniform data_
- Shared data
- Common Data
- State data


Alright, I say stick with uniform data. The name still makes sense, and it is familiar, and it is better than the names for the "varying" data.



__What is the most efficient indexing sceheme for arbitrary indexable buffers?__


Note, SSBO is generally what I should be using for buffers, by the looks of it.







Note, there should probably be a difference between local uniforms, and global uniforms. I suppose the answer to that is quite obvious though. Just like how I despise the idea of global buffer streaming to be treated like global variables, so too do I despise the idea of an instanced global uniform. Therefore, either all changes to a uniform are shared between shader instances.

I should call them _hyperthreads_.
Although that name overlaps with hyperthreading in cpus. 

But lets use this term for now. All hthreads share global data. And all changes must either be shared between all of them, or they must be read-only. However, this shared data does not mean it is atomic, and can lead to a lot of side effects when accessed directly. Therefore, also have features for atomic access.

And then we have local uniforms, which are really less uniforms at this point, and more like state data. But generally these are passed via parameters, and are truly local to each hthread.
