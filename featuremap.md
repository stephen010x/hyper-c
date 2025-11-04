



=====================================
##   Add these native types to C   ##
=====================================

[NOTE THAT ALL OF THESE ARE INCLUDED VIA LIBRARY]
This helps indicate that these aren't exactly native C types, but more like orgainic types you can include.



## Floats

I was considering fixed point floats, but that requires a whole bunch of combinations.
So this would probably be better suited to a library.

Look into fixed point types
https://gcc.gnu.org/onlinedocs/gcc-4.9.4/gcc/Fixed-Point.html#Fixed-Point


`float16_t` macro alias for `_Float16` (Implemented by this extension)
`float32_t` macro alias for `float`
`float64_t` macro alias for `double`




## Vectors

[n] can be '2', '3', and '4'

`_Vector[n]` with macros `vector[n]`, and `vec[n]_t`

`_Bvec[n]` with macros `bvec[n]_t`
`_Ivec[n]` with macros `ivec[n]_t`
`_Uvec[n]` with macros `uvec[n]_t`
`_Fvec[n]` with macros `fvec[n]_t`
`_Svec[n]` with macros `svec[n]_t`

For instance, `_Vvec2`, `fvec3_t`, etc

`_Vec[n]` and `_Vector` types can be combined with `signed`, `unsigned`, `char`, `short`, `int`, and `long` specifiers in order to create the following types:

[m] can be '8', '16', '32', '64'

`_B[m]vec[n]` with macros `b[m]vec[n]_t`
`_I[m]vec[n]` with macros `i[m]vec[n]_t`
`_U[m]vec[n]` with macros `u[m]vec[n]_t`
`_F[m]vec[n]` with macros `f[m]vec[n]_t`
`_S[m]vec[n]` with macros `s[m]vec[n]_t`

For instance, `_S32vec3`, `u16vec4_t`, and `s64vec2_t`

`_Vector[n]` defaults to `_S32vec[n]`

These support most of the common C arithmatic operators, which I suppose shall be defined later. 

*There shall be no swizzling* for now. It just doesn't really justify itself very well as c-like behavior. Plus, you can already specify them independantly. So that should be enough for now.
You can still access them individually using .x, .y, .r, .g, etc.

vectors can decay into structs, and can be both indexed and assigned through struct syntax.
For instance, 
`vec3 myvect = (vec3){.x = 1, .y = 2, .z = 3};`
`myvect.x = 10;`

Vectors can also decay into static arrays. For instance,
`myvect[1] = 10;`

Added will also be vector constructors for all of the listed types. For instance,
`vec2(...)`, `f32vec4(...)`, `uvec3(...)`, etc.
These will all just be macros to structlike assignment syntax.



## Matrices

[n] can be '2', '3', and '4'

`_Matrix[n]` with macros `matrix[n]`, and `mat[n]_t`

`_Bmat[n]` with macros `bmat[n]_t`
`_Imat[n]` with macros `imat[n]_t`
`_Umat[n]` with macros `umat[n]_t`
`_Fmat[n]` with macros `fmat[n]_t`
`_Smat[n]` with macros `smat[n]_t`

For instance, `_Vmat2`, `fmat3_t`, etc

`_Mat[n]` and `_Matrix` types can be combined with `signed`, `unsigned`, `char`, `short`, `int`, and `long` specifiers in order to create the following types:

[m] can be '8', '16', '32', '64'

`_B[m]mat[n]` with macros `b[m]mat[n]_t`
`_I[m]mat[n]` with macros `i[m]mat[n]_t`
`_U[m]mat[n]` with macros `u[m]mat[n]_t`
`_F[m]mat[n]` with macros `f[m]mat[n]_t`
`_S[m]mat[n]` with macros `s[m]mat[n]_t`

For instance, `_S32mat3`, `u16mat4_t`, and `s64mat2_t`
And `unsigned short matrix4`

`_Matrix[n]` defaults to `_S32mat[n]`

These also support common arithmatic operators relevant to matrices, plus various extra function operators in associated libraries.

Matrices can be initilized using both a multidimensional array, as well as an array of vectors. For instance,
`mat3 mymat = {{1,2,3}, {4,5,6}, {7,8,9}};`
`mat3 mymat = {vec3(1,2,3), vec3(4,5,6), vec3(7,8,9)};`

Matrices decay into an array of vectors. For instance,
`mymat[0] = vec3(1,1,1);`
You can also index them like a multidimensional array
`mymat[0][1] = 2;`






=====================================
##   The 'hyper' keyword           ##
=====================================

The `hyper` keyword is generally a storage classifier for both variables as well as functions.

When specified on a variable, this indicates to the compiler that the variable should only exist on the gpu (generally in reference to spir-v shader code)

`hyper` functions are generally compiled to spir-v, and then stored in a read-only non executable text segment to be passed to opengl/vulkan to be further compiled and placed to be ran on the GPU.

`hyper` functions *cannot* call non-hyper functions unless specifically enabled by a compiler flag or function attribute. Originally I was going to allow this such that common library functions could be called from a hyper function. But this can't be done past current source files, so it is now marked as a nonexistant feature.

Ordinary functiosn cannot call a `hyper` function. Doing so will result in a compiler error unless explicitly enabled via a flag or function attribute.

`hyper` functions can call other `hyper` functions.




=====================================
##   Added Attributes              ##
=====================================

__attribute__((hyper_entry))

Indicates that an hyper function is also an entry function without needing the function name to start with `main_`


__attribute__((hyper_hybrid))

Indicates that a function is to be both compiled for x86 and spirv (if x86 and spirv are the cpu and gpu architectures. I would have specified spirv in the name, but I wanted this to be more general).
This can also be used on variables to indicate that a version is to exist both on the cpu and for the gpu. However, these two versions won't be automatically synced, and are technically independant of each-other.




=====================================
##   Added Compiler Flags          ##
=====================================

-hfhybrid
This indicates that all `hyper` functions implicitly use the `hyper_hybrid` attribute.

-hvhybrid
This indicates to the compiler that all `hyper` variables implicitly use the `hyper_hybrid` attribute.

-hdiscard
This tells the compiler to discard all unused output for `hyper` functions that are never directly referenced in the source code. This includes CPU code generated as a result of the `hfhybrid` flag.

-Whextra
Enable warnings for certain behaviors in hyper functions that are generally very slow or unoptimal practices for shaders to do.


# shader output flags
-hspirv     [default]
-hglsl
-hhlsl      [not yet supported]
-hmetal     [not yet supported]
These specify which shader language output for hyper function compilation. (spirv is reccomended)


# library output flags
-hopengl
-hvulkan    [default]
-hdirectx   [not yet supported]
-hmetal     [not yet supported]
These specify whether to output setup code in opengl or vulkan (generally a question between size, speed, and compatability)
Note that the `hmetal` flag is shared between the shader output and the library output flags, and all other shader and library output flags are incompatable with this one.
Note that the `hdirectx` flag is incompatable with the `hglsl` and `hmetal` flags.
Note that the `hopengl` and `hvulkan` flags are incompatable with the `hmetal` flag, and until further notice the `hhlsl` flag.

__NOTE! I plan to add versioning to these flags later, such as 'hopengl1.0', etc.__






==================================================
##   Rules and Limitations of Hyper Functions   ##
==================================================

Generally, all native C types are valid in hyper functions. All general C syntax should be valid within a `hyper` function, with exception to pointers.


# function parameters

Function parameters for hyper functions generally behave the same as ordinary functions (though functions are likely to be optimized out by the spir-v compiler). You don't need any `hyper` specifier within function parameters as they are implicitly `hyper` already. Doing so would likely cause a warning.


# entry functions

Entry functions are specified one of two ways. Either by using the hyper entry naming convention of prepending `hyper_` to the function identifier (ie `hyper_myshader`), or by using the `__attribute__((hyper_entry))` attribute.

Entry functions behave just like normal hyper functions, except a certain set of function prototypes may be enforced, and some backend setup code is generated in order for helper CPU-side functions to load and run the shader onto the GPU.

If a  hyper  function is either not also an entry function, or not referenced by an entry function, then it most likely will be optimized out of the final compiled binary.


# local variables

Local variables declared within a hyper function, as well as function parameters, as well as static variables, all within a hyper function, do not need `hyper` storage specifiers, as they inherit that behavior from the hyper function they reside in. Otherwise they behave the same. However you cannot reference them unless they are members of an array or struct. (Or maybe I should just have referencing return zero)


# global variables

Hyper global variables behave a bit differently than ordinary global variables.


# pointers and buffers





=====================================
##   Hyper Structs                 ##
=====================================





==========================================
##   Hyper Function Input and Outputs   ##
==========================================




=====================================
##   Hyperc Library Functions      ##
=====================================





=====================================
##   Notes                         ##
=====================================

While objects require the `hyper` identifier to be visable to a `hyper` function, types do not.
In other words, struct *types*, even if not struct *objects*, are visable as usable types within a `hyper` function. This also includes enums, unions, and anything similar that doesn't require physical resources in the compiler output file or in runtime memory to use.

