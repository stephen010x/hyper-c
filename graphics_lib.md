

Alright, new idea for graphics library, because opengl annoys me.
Ideally it is built off of vulkan. But initially it probably needs to use opengl.


But basically, the idea is to get rid of the whole binding scheme. And to instead basically either have a bunch of structs that represents the bindings, or to instead have opengl-like bindings, but have the binding be explicit.

For instance, uploading data would require an integer that represents what buffer it is being uploaded to. 

And certain binds in a binding heiarchy would be assigned a certain number basically, kind of like the winapi. But ultimately it is all explicit.
