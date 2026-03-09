#!/usr/bin/env lua
-- make.lua [goal]
-- arg[0] = script name, arg[1]..arg[n] = args, #arg = count
local build = require "buildtree"
local fs = build.fs
local cmd = build.cmd
local filter = build.filter
local enum, ext, type = build.enum, build.ext, build.type
local debug = build.debug


local is_verbose = false
local is_show_tree = false


local system = enum.linux
local cc = build.gcc

local dir = {
    src = "src",
    tmp = "tmp",
    inc = "inc",
    bin = "bin",
    lib = "lib",
}


-- get location of gcc plugin headers
local gcc_plugin_dir = cmd("gcc -print-file-name=plugin")


flags = "-Wall -Wextra"
cflags = "-std=gnu17 -fvisibility=internal -I" .. dir.inc
cflags = cflags .. " -isystem" .. gcc_plugin_dir .. "/include"
lflags = ""


-- these are the flags actually used, so make sure to append the previous shared flags
local goals = {
    debug = {
        flags  = "-g -Og " .. flags,
        cflags = "-D__DEBUG__ -D__debug__ " .. cflags,
        lflags = " " .. lflags,
    },
    release = {
        flags  = "-Os -g0 -flto " .. flags,
        cflags = "-fdata-sections -ffunction-sections " .. cflags,
        lflags = "-s -Wl,--gc-sections " .. lflags,
    },
}

goals.default = goals.debug


local outputs = {
    cmd = {
        target = "bin/hyper",
        srcdir = "src/cmd",
        cflags = "-fno-pie",
        lflags = "-no-pie",
        
    },
    lib = {
        target = "bin/hyper.so",
        srcdir = "src/lib",
        cflags = "-fPIC",
        lflags = "-Wl,-E -shared",
    },
}








local function concat_flags(...)
    flist = {...}
    out = ''
    for _, flags in pairs(flist) do
        out = out..(' '..flags or '')
    end
    return out:squeeze()
end


local goal = arg[1] or 'default'







local buildtree = build.newtree()




local function gen_objs(srcs, tmpdir, refdir, opts)
    if refdir == nil then refdir = fs.pwd() end
    
    local objs = {}

    -- generate dependancy tree from sources
    for _, cfile in pairs(srcs) do

        local nearpath = fs.get_nearpath(cfile, refdir)
        local objfile = fs.set_ext(tmpdir..'/'..nearpath, ext.o)
        
        table.insert(objs, objfile)
    
        buildtree.targets[objfile] = {
            --type = ext.o,
            builder = cc.builder,
            depends = cc.gen_dep(cfile, opts),
            options = opts.." -c",
            force_rebuild = false,
        }
    end

    return objs
end


-- add targets to build tree
-- TODO: add a sources member to targets that is distinct from depends
--       as well as add this makefile to the dependancies
for _, out in pairs(outputs) do
    -- get recursive list of C files in the source directory
    local c_srcs = fs.find(out.srcdir, true, filter.ext('.c'))

    -- local oflags = concat_flags(flags, cflags, out.flags, out.cflags)
    -- local xflags = concat_flags(flags, lflags, out.flags, out.lfla
    local oflags = concat_flags(goals[goal].flags, goals[goal].cflags, out.flags, out.cflags)
    local xflags = concat_flags(goals[goal].flags, goals[goal].lflags, out.flags, out.lflags)
    
    -- generate objects into buildtree
    local tmpdir = dir.tmp .. '/' .. fs.get_name(out.target) .. '_' .. goal
    local c_objs = gen_objs(c_srcs, tmpdir, fs.get_dir(out.srcdir), oflags)

    buildtree.targets[out.target] = {
        --type = type.elf,
        builder = cc.builder,
        depends = c_objs,
        options = xflags,
        force_rebuild = false,
    }
end



if is_show_tree then
    debug.print_table(buildtree, true)
end



-- build outputs
for _, out in pairs(outputs) do
    local ok = buildtree:build(out.target, is_verbose)
    if not ok then 
        print("build failed")
        break
    end
end
