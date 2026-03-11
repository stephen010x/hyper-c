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
local make = build.make


goals.default = 'debug'
-- set goal variable from input args
local goal = arg[1] or goals.default


local outname = "hyper"


local dir = {
    src = "src/",
    tmp = "tmp/",
    inc = "inc/",
    bin = "bin/",
    lib = "lib/",
}




flags = "-Wall -Wextra"
cflags = "-std=gnu17 -fvisibility=internal -Wno-multichar -fno-pie -I" .. dir.inc
--cflags = cflags .. " -isystem" .. gcc_plugin_dir .. "/include"
lflags = "-no-pie"
jflags = ""


-- these are the flags actually used, so make sure to append the previous shared flags
local goals = {
    debug = {
        flags  = flags  .. "-g -Og ",
        cflags = cflags .. "-D__DEBUG__ -D__debug__ ",
        lflags = lflags .. " ",
        jflags = jflags .. "-g",   -- luajit flags
    },
    release = {
        flags  = flags  .. "-Os -g0 -flto ",
        cflags = cflags .. "-fdata-sections -ffunction-sections ",
        lflags = lflags .. "-s -Wl,--gc-sections ",
        jflags = jflags .. "-s",   -- luajit flags
    },
}



-- get recursive list of C files in the source directory
local c_srcs = fs.find(out.srcdir, true, filter.ext('.c'))
-- get recursive list of lua files in the source directory
local lua_srcs = fs.find(out.srcdir, true, filter.ext('.lua'))


local buildtree = build.newtree()


-- add LuaJIT library makefile to buildtree
buildtree.targets[dir.lib .. "/LuaJIT/src/libluajit.a"] = {
    builder = make.builder,
    depends = ["/LuaJIT/Makefile"],
    options = "CFLAGS=\"-std=gnu17 -fdata-sections -ffunction-sections\"",
    force_rebuild = false,
}


local libs = {
    dir.lib .. "/LuaJIT/src/libluajit.a",
}





--#########################################################################
--#########################################################################
--          RUNTIME BUILD STUFF




-- add flags to make all libraries dirs visable to compiler
for _, libdir in pairs(fs.ls(dir.lib)) do
    if fs.is_dir(libdir) then
        goals[goal].cflags = goals[goal].cflags..' -I'..libdir..'/inc'..' -I'..libdir..'/src'
        goals[goal].lflags = goals[goal].lflags..' -L'..libdir..'/bin'
    end
end



-- object compiler flags, linker flags, and lua flags
local oflags = goals[goal].flags .. " " .. goals[goal].cflags
local xflags = goals[goal].flags .. " " .. goals[goal].lflags
local jflags = goals[goal].jflags



function onbuild()


    local tmpdir = get_tmpdir(dir.bin .. "/" .. outname)
    
    -- generate objects into buildtree
    local   c_objs = gen_c_objs(   c_srcs,   tmpdir, fs.get_dir(dir.src), oflags )
    local lua_objs = gen_lua_objs( lua_srcs, tmpdir, fs.get_dir(dir.src), jflags )


    local depends = concat_tables(c_objs, lua_objs, libs)


    buildtree.targets[dir.bin .. "/" .. outname] = {
        --type = type.elf,
        builder = cc.builder,
        depends = depends,
        sources = depends,
        options = xflags,
        force_rebuild = false,
    }


    -- run buildtree
    local ok = buildtree:build(out.target, is_verbose)
    if not ok then print("build failed") end


    -- optionally print buildtree
    if ok and is_show_tree then debug.print_table(buildtree, true) end

    
end









--#########################################################################
--#########################################################################
--          HELPER FUNCTIONS PAST THIS POINT








local function concat_tables(...)
    out = {}
    for _, table in pairs({...}) do
        for key, value in pairs(table) do
            out[key] = value
        end
    end
    return out
end


-- function to properly concat flags
local function concat_flags(...)
    flist = {...}
    out = ''
    for _, flags in pairs(flist) do
        out = out..(' '..flags or '')
    end
    return out:squeeze()
end







local function get_tmpdir(target)
    return dir.tmp .. '/' .. fs.get_name(target) .. '_' .. goal
end






-- function to add c objects to build tree
local function gen_c_objs(srcs, tmpdir, refdir, opts)
    if refdir == nil then refdir = fs.pwd() end
    
    local objs = {}

    -- generate dependancy tree from sources
    for _, cfile in pairs(srcs) do

        local nearpath = fs.get_nearpath(cfile, refdir)
        local objfile = fs.set_ext(tmpdir..'/'..nearpath, ext.o)
        local depends = cc.gen_dep(cfile, opts)
        
        table.insert(objs, objfile)
    
        buildtree.targets[objfile] = {
            --type = ext.o,
            builder = cc.builder,
            depends = depends,
            sources = {cfile},
            options = opts.." -c",
            force_rebuild = false,
        }
    end

    return objs
end





-- function to add lua objects to build tree
local function gen_lua_objs(srcs, tmpdir, refdir, opts)
    if refdir == nil then refdir = fs.pwd() end
    
    local objs = {}

    -- generate dependancy tree from sources
    for _, file in pairs(srcs) do

        local nearpath = fs.get_nearpath(file, refdir)
        local objfile = fs.set_ext(tmpdir..'/'..nearpath, "lua.o")
        
        table.insert(objs, objfile)
    
        buildtree.targets[objfile] = {
            --type = ext.o,
            builder = luajit.builder,
            depends = {file},
            sources = {file},
            options = opts.." -t obj",
            force_rebuild = false,
        }
    end

    return objs
end





onbuild()






--#########################################################################
--#########################################################################
--          JUNK CODE








-- add targets to build tree
-- TODO: add a sources member to targets that is distinct from depends
--       as well as add this makefile to the dependancies
-- for _, out in pairs(outputs) do
--     -- get recursive list of C files in the source directory
--     local c_srcs = fs.find(out.srcdir, true, filter.ext('.c'))
-- 
--     -- local oflags = concat_flags(flags, cflags, out.flags, out.cflags)
--     -- local xflags = concat_flags(flags, lflags, out.flags, out.lfla
--     local oflags = concat_flags(goals[goal].flags, goals[goal].cflags, out.flags, out.cflags)
--     local xflags = concat_flags(goals[goal].flags, goals[goal].lflags, out.flags, out.lflags)
--     
--     -- generate objects into buildtree
--     local tmpdir = dir.tmp .. '/' .. fs.get_name(out.target) .. '_' .. goal
--     local c_objs = gen_c_objs(c_srcs, tmpdir, fs.get_dir(out.srcdir), oflags)
-- 
--     buildtree.targets[out.target] = {
--         --type = type.elf,
--         builder = cc.builder,
--         depends = concat_tables(c_objs, out.depends),
--         sources = c_objs,
--         options = xflags,
--         force_rebuild = false,
--     }
-- end



-- if is_show_tree then
--     debug.print_table(buildtree, true)
-- end



-- -- build outputs
-- for _, out in pairs(outputs) do
--     local ok = buildtree:build(out.target, is_verbose)
--     if not ok then 
--         print("build failed")
--         break
--     end
-- end







-- manual output tree
-- more is generated and added to this
-- local outputs = {
--     main = {
--         target = dir.bin .. "/" .. outname,
--         srcdir = dir.src,
--         cflags = "-fno-pie",
--         lflags = "-no-pie",
--         depends = {},
--     },
-- }
