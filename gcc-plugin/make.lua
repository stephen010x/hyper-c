#!/usr/bin/env lua
-- make.lua [goal]
-- arg[0] = script name, arg[1]..arg[n] = args, #arg = count
local build = require "buildtree"
local fs = build.fs
local cmd = build.cmd
local filter = build.filter
local enum, ext, type = build.enum, build.ext, build.type
local cc = build.gcc
local debug = build.debug


local system = enum.linux

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
        lflags = "" .. lflags,
    },
    release = {
        flags  = "-Os -g0" .. flags,
        cflags = "" .. cflags,
        lflags = "-s" .. lflags,
    },
}

goals.default = goals.debug


local outputs = {
    cmd = {
        target = "bin/hyper",
        srcdir = "src/cmd",
    },
    lib = {
        target = "bin/hyper.so",
        srcdir = "src/lib",
        cflags = "-pie -Wl,-E",
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

-- if goal == "clean" then
--     fs.rm(dir.tmp, "-rf")
-- elseif goal == "test" then
-- elseif goal == "debug" then
-- elseif goal == "dbg" then
-- end

-- for gname, g in pairs(goals) do
--     if gname == goal then
--         flags  = concat_flags(flags,  g.flags)
--         cflags = concat_flags(cflags, g.cflags)
--         lflags = concat_flags(lflags, g.lflags)
--     end
-- end







local buildtree = build.newtree()




local function gen_objs(srcs, tmpdir, refdir, opts)
    if refdir == nil then refdir = fs.pwd() end
    
    local objs = {}

    -- generate dependancy tree from sources
    for _, cfile in pairs(srcs) do

        local nearpath = fs.get_nearpath(cfile, refdir)
        local objfile = fs.set_ext(tmpdir..'/'..nearpath, ext.o)

        print(objfile)
        
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
for _, out in pairs(outputs) do
    -- get recursive list of C files in the source directory
    local c_srcs = fs.find(out.srcdir, true, filter.ext('.c'))

    -- local oflags = concat_flags(flags, cflags, out.flags, out.cflags)
    -- local xflags = concat_flags(flags, lflags, out.flags, out.lfla
    local oflags = concat_flags(goals[goal].flags, goals[goal].cflags, out.flags, out.cflags)
    local xflags = concat_flags(goals[goal].flags, goals[goal].lflags, out.flags, out.lflags)
    
    -- generate objects into buildtree
    local tmpdir = dir.tmp .. '/' .. out.target .. '_' .. goal
    local c_objs = gen_objs(c_srcs, tmpdir, fs.pwd(), oflags)

    buildtree.targets[out.target] = {
        --type = type.elf,
        builder = cc.builder,
        depends = c_objs,
        options = xflags,
        force_rebuild = false,
    }
end



debug.print_table(buildtree, true, '\n', '    ')



-- build outputs
-- TODO: add a way to interrupt this if there is an error
for _, out in pairs(outputs) do
    local ok = buildtree:build(out.target)
    if not ok then 
        print("build failed")
        break
    end
end
