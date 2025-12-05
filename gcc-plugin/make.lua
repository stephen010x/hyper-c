local build = require "buildtree"
local fs = build.fs
local filter = build.filter
local enum, ext, type = build.enum, build.ext, build.type
local gcc = build.gcc


local dir = {
    src = "src",
    tmp = "tmp",
    inc = "inc",
    bin = "bin",
}

local target = "main.exe"
local srcdir = "src"
local bindir = "bin"

local flags = "-d -Os"
local cflags = "-DDEBUG"
local lflags = " -fPIC"


cflags = cflags.." -I"..dir.inc


-- get location of gcc plugin headers
local gcc_plugin_dir = cmd("gcc -print-file-name=plugin")
cflags = cflags.." -I"..gcc_plugin_dir.."/include"



-- get recursive list of C files in the source directory
local c_srcs = fs.find(dir.src, true, filter.ext(ext.c))



local buildtree = build.newtree()


-- add target to build tree
buildtree.targets[dir.bin..'/'..target] = {
    type = type.elf,
    builder = gcc.bulder,
    depends = c_objs,
    options = flags.." "..lflags,
    force_rebuild = false,
}



-- generate dependancy tree from sources
for _, cfile in pairs(c_srcs) do
    local deps = gcc.gen_dep(cfile, options)

    local objfile = fs.set_ext(cfile, ext.o)

    buildtree.targets[objfile] = {
        type = ext.o,
        builder = gcc.builder,
        depends = gcc.gen_dep(cfile, cflags),
        options = cflags.." -c",
        force_rebuild = false,
    }
end



-- build target
buildtree:build(target)
