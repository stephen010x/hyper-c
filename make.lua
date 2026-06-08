#!/usr/bin/env luajit

-- make.lua [goal]
-- arg[0] = script name, arg[1]..arg[n] = args, #arg = count
local build = require "buildtree"
local fs = build.fs
local cmd = build.cmd
local filter = build.filter
local enum, ext, type = build.enum, build.ext, build.type
local debug = build.debug
local this = arg[0]

-- forward declare helper functions
local get_tmp_path
local ccatflags
local get_tmpdir
local gen_c_objs
local gen_lua_objs


local is_verbose = false
local is_show_tree = false


local system = enum.linux
local cc     = build.gcc
local make   = build.make
local luajit = build.luajit
local copy   = build.copy
local gen    = build.gen
table = build.table


default_goal = 'release'
-- set goal variable from input args
local goal = arg[1] or default_goal


local outname = "hyperc"


local dir = {
    src = "src/",
    tmp = "tmp/",
    -- inc = "inc/",
    inc = "src/",
    bin = "bin/",
    lib = "lib/",
}




local libs = {
    ["libluajit.a"] = {
        make = fs.path(dir.lib, "LuaJIT/"),
        bin  = fs.path(dir.lib, "LuaJIT/src/"),
        inc  = fs.path(dir.lib, "LuaJIT/src/"),
        opts = 'BUILDMODE=static XCFLAGS+="-DLUAJIT_DISABLE_FFI" CFLAGS+="-fdata-sections -ffunction-sections -Wno-switch"',
        prebuild = true, -- builds before rest of build tree is constructed due to generation dependancies
    },
    ["liblpeg.a"] = {
        make = fs.path(dir.lib, "lpeg-1.1.0/"),
        bin  = fs.path(dir.lib, "lpeg-1.1.0/"),
        inc  = fs.path(dir.lib, "lpeg-1.1.0/"),
        opts = "",
    },
    ["utils"] = {
        bin  = fs.path(dir.lib, "toolkit/"),
        inc  = fs.path(dir.lib, "toolkit/inc/"),
        opts = "",
        header_only = true, -- makes it not a dependancy, but still adds compiler flags to find headers
    },
}




flags = "-Wall -Wextra"
cflags = "-std=gnu17 -fvisibility=internal -Wno-multichar -fno-pie -I" .. dir.inc
--cflags = cflags .. " -isystem" .. gcc_plugin_dir .. "/include"
lflags = "-no-pie -static -lluajit -llpeg -lm"
jflags = ""




-- these are the flags actually used, so make sure to append the previous shared flags
local goals = {
    debug = {
        flags  = flags  .. " -g -Og ",
        cflags = cflags .. " -D__DEBUG__ -D__debug__ ",
        lflags = lflags .. "",
        jflags = jflags .. " -g",   -- luajit flags
    },
    release = {
        flags  = flags  .. " -Os -g0 -flto ",
        cflags = cflags .. " -fdata-sections -ffunction-sections ",
        lflags = lflags .. " -s -Wl,--gc-sections ",
        jflags = jflags .. " -s",   -- luajit flags
    },
}



-- get recursive list of C files in the source directory
local c_srcs = fs.find(dir.src, true, filter.ext('.c'))
-- get recursive list of lua files in the source directory
local lua_srcs = fs.find(dir.src, true, filter.ext('.lua'))


local buildtree = build.newtree()














--#########################################################################
--#########################################################################
--          RUNTIME BUILD STUFF



-- add flags to make all libraries dirs visable to compiler
for name, lib in pairs(libs) do
    goals[goal].cflags = goals[goal].cflags .. " -I" .. lib.inc
    if not lib.header_only then
        goals[goal].lflags = goals[goal].lflags .. " -L" .. lib.bin
    end
end



-- object compiler flags, linker flags, and lua flags
local oflags = goals[goal].flags .. " " .. goals[goal].cflags
local xflags = goals[goal].flags .. " " .. goals[goal].lflags
local jflags = goals[goal].jflags




-- setup library buildtrees and flags
local libpaths = {}
for name, lib in pairs(libs) do
    if not lib.header_only then
        buildtree.targets[fs.path(lib.bin, name)] = {
            builder = lib.make and make.builder or nil,
            depends = {fs.path(lib.bin, name)}, -- depend on self so it won't rerun if it exists
            sources = {lib.make},
            options = lib.opts,
        }
        table.insert(libpaths, fs.path(lib.bin, name))

        if lib.prebuild then
            -- debug.print_table(buildtree, true)
            local ok = buildtree:build(fs.path(lib.bin, name), is_verbose)
            if not ok then error("build failed") end
        end
    end
end



-- tidy flags
local oflags = oflags:squeeze()
local xflags = xflags:squeeze()
local jflags = jflags:squeeze()



function onbuild()


    -- local tmpdir = get_tmpdir(dir.bin .. "/" .. outname)
    local tmpdir = get_tmpdir(outname)
    
    -- generate objects into buildtree
    local   c_objs = gen_c_objs(   c_srcs,   tmpdir, fs.get_dir(dir.src), oflags )
    local lua_objs = gen_lua_objs( lua_srcs, tmpdir, fs.get_dir(dir.src), jflags, oflags )


    -- local depends = concat_tables(c_objs, lua_objs, libs, this)
    -- local sources = concat_tables(c_objs, lua_objs, libs)
    local depends = table.join(libpaths, c_objs, lua_objs, {this})
    local sources = table.join(c_objs, lua_objs)


    -- build to tmp dir
    buildtree.targets[fs.path(tmpdir, outname)] = {
        builder = cc.builder,
        depends = depends,
        sources = sources,
        options = xflags,
        force_rebuild = false,
    }


    -- copy from executable from tmp to bin
    buildtree.targets[fs.path(dir.bin, outname)] = {
        builder = copy.builder,
        depends = {fs.path(tmpdir, outname), this},
        sources = {fs.path(tmpdir, outname)},
        options = "",
        force_rebuild = true,
    }


    -- if touchup then
    --     local ok = touchup(buildtree)
    --     if not ok then print("build failed"); return end
    -- end

    -- optionally print buildtree
    if is_show_tree then debug.print_table(buildtree, true) end

    -- run buildtree
    local ok = buildtree:build(fs.path(dir.bin, outname), is_verbose)
    if not ok then error("build failed") end
    
end









--#########################################################################
--#########################################################################
--          HELPER FUNCTIONS PAST THIS POINT







get_tmp_path = function(path, tmpdir, refdir, ext, do_append)
    local nearpath = fs.get_nearpath(path, refdir)
    if ext == nil then
        return tmpdir..'/'..nearpath
    elseif do_append or ext == nil then
        return tmpdir..'/'..nearpath..'.'..ext
    else
        return fs.set_ext(tmpdir..'/'..nearpath, ext)
    end
end





-- function to properly concat flags
ccatflags = function(...)
    flist = {...}
    out = ''
    for _, flags in pairs(flist) do
        out = out..(' '..flags or '')
    end
    return out:squeeze()
end






get_tmpdir = function(target)
    -- return fs.path(dir.tmp, fs.get_name(target)..'_'..goal)
    return fs.path(dir.tmp, fs.get_name(target), goal)
end





-- function to add c objects to build tree
gen_c_objs = function(srcs, tmpdir, refdir, opts)
    if refdir == nil then refdir = fs.pwd() end
    if opts == nil then opts = "" end
    
    local objs = {}

    -- generate dependancy tree from sources
    for _, cfile in pairs(srcs) do

        -- local nearpath = fs.get_nearpath(cfile, refdir)
        -- local objfile = fs.set_ext(tmpdir..'/'..nearpath, ext.o)
        -- local depends = concat_tables(cc.gen_dep(cfile, opts), this)
        local objfile = get_tmp_path(cfile, tmpdir, refdir, ext.o)
        local depends = table.join(cc.gen_dep(cfile, opts), {this})
        
        table.insert(objs, objfile)
    
        buildtree.targets[objfile] = {
            --type = ext.o,
            builder = cc.builder,
            depends = depends,
            sources = {cfile},
            options = "-c "..opts,
        }
    end

    return objs
end





-- function to add lua objects to build tree
gen_lua_objs = function(srcs, tmpdir, refdir, opts, copts)
    if refdir == nil then refdir = fs.pwd() end
     if opts == nil then opts = "" end
    
    local objs = {}

    -- generate dependancy tree from sources
    for _, file in pairs(srcs) do

        -- local nearpath = fs.get_nearpath(file, refdir)
        -- local objfile = fs.set_ext(tmpdir..'/'..nearpath, ext.o)
        local objfile = get_tmp_path(file, tmpdir, refdir, ext.o, true)
        
        table.insert(objs, objfile)
    
        buildtree.targets[objfile] = {
            builder = luajit.builder,
            depends = {file, this},
            sources = {file},
            options = opts.." -t obj",
        }
    end

    -- generate an object file with the bytecode sizes
    buildtree.targets[fs.path(tmpdir, "_auto_luabc.o")] = {
        builder = gen.builder.osizes,
        depends = objs,
        sources = objs,
        options = copts,
    }

    objs = table.join(objs, {fs.path(tmpdir, "_auto_luabc.o")})
    return objs
end





onbuild()
