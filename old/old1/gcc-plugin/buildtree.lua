-- buildtree module


local module = {}





debug = {}

debug.string_table = function(t, is_deep, sep, tabspace, _pref)
    if is_deep == nil then is_deep = false end
    --if sep == nil then sep = ' ' end
    --if tabspace == nil then tabspace = '' end
    if sep == nil then sep = '\n' end
    if tabspace == nil then tabspace = '   ' end
    if _pref == nil then _pref = '' end

    local _tstr = {}
    table.insert(_tstr, '{')
    for key, value in pairs(t) do
        local vstr

        if type(value) == "table" and is_deep then
            if next(value) == nil then
                vstr = '{}'
            else
                vstr = debug.string_table(value, is_deep, sep, tabspace, _pref..tabspace)
            end
        elseif type(value) == "string" then 
            vstr = '"'..tostring(value)..'"'
        else 
            vstr = tostring(value)
        end

        table.insert(_tstr, ('%s[%s] = %s,'):format(_pref..tabspace, tostring(key), vstr))

    end
    table.insert(_tstr, _pref..'}')

    return table.concat(_tstr, sep)
end


debug.print_table = function(t, is_deep, sep, tabspace)
    print(debug.string_table(t, is_deep, sep, tabspace))
end

module.debug = debug


-- debug.print_table(table, true, '\n', '\t')





local enum = {
    meta = {
        __index = function(self, key)
            return key
        end,
        __newindex = function(table, key, value)
            print("writing to ext table not allowed")
        end,
    },
}
setmetatable(enum, enum.meta)

module.enum = enum
module.ext  = enum
module.type = enum







local function cmd(command, is_print)
    if is_print then print(command) end
    local handle = io.popen(command)
    local out = handle:read("*a")
    local ok, stat = handle:close()
    -- remove trailing newline if it exists
    if out:sub(-1) == '\n' then out = out:sub(1, -2) end
    return out
end

module.cmd = cmd





-- separate string into table using separator
-- adds this behavior to the main string metatable
local stringmt = getmetatable("", strip)
stringmt.__index.split = function(self, sep)
    if sep == nil then sep = " " end
    if sep == "" then return self end

    --local out = table.new()
    local out = {}
    local index = 1
    local last = 1
    while true do
        start, stop = self:find(sep, index, true)
        if not start then break end
        table.insert(out, self:sub(last, start-1))
        index = stop + 1
        last = index
    end
    table.insert(out, self:sub(last))

    if strip == true then
        for key, value in pairs(out) do
            out[key] = value:strip()
        end
    end

    return out
end

-- strip beginning and end of string of whitespace
-- adds method to main string metatable
stringmt.__index.strip = function(self)
    return self:gsub("^%s+", ""):gsub("%s+$", "")
end

-- replaces all consecutive whitespace with single spaces
-- adds method to main string metatable
stringmt.__index.squeeze = function(self)
    local s = self:gsub("%s+", " ")
    return s:gsub("^%s+", ""):gsub("%s+$", "")
end









table.copy = function(t)
    rt = {}
    for key, value in pairs(t) do
        rt[key] = value
    end
    return rt
end

table.new = function(rtable)
    if rtable == nil then rtable = {} end
    return setmetatable(table.copy(rtable), table)
end

table.print = debug.print_table

-- implement later
-- table.slice = function(t, first, last)
--     if first < 0
-- end






local fs

-- linux file system helper
fs = {
    ls = function(path)
        if path == nil then path = "." end
        local files = cmd("ls -1 --color=never \""..path.."\""):split('\n', true)
        for key, value in pairs(files) do
            files[key] = './'..fs.get_nearpath(path..'/'..value)
        end
        return files
    end,
    
    cd = function(path)
        if path == nil then path = "~" end
        return cmd("cd \""..path.."\"")
    end,

    -- rm = function(path, opts)
    --     cmd("\\rm "..opts.." \""..path.."\"")
    -- end,

    pwd = function()
        return cmd("pwd -P")
    end,

    mkdir = function(path, opts)
        if opts == nil then opts = "" end
        cmd ("mkdir "..opts.." "..path)
    end,

    get_type = function(path)
        return cmd("file -b \""..path.."\"")
    end,

    get_fullpath = function(path)
        return cmd("realpath --canonicalize-missing \""..path.."\"")
    end,

    get_dir = function(path)
        return cmd("dirname -- "..path)
    end,

    -- get file extension from name
    get_ext = function(path)
        --if path == nil then return nil end
        local index = path:reverse():find('.', 1, true)
        if index == nil then return nil end
        index = #path - index + 1
        return index and path:sub(index) or ""
    end,

    get_name = function(path)
        return cmd("basename \""..path.."\"")
    end,

    set_ext = function(names, ext)
        local is_table = type(names) == "table"
        if not is_table then names = {names} end

        local rt = {}

        for key, name in pairs(names) do
            local nlist = name:split('.')
            nlist[#nlist] = nil
            rt[key] = table.concat(nlist, '.')..'.'..ext
        end

        return is_table and rt or select(2, next(rt))
    end,

    -- returns path relative to base
    get_nearpath = function(path, base)
        if base == nil then base = '.' end
        return cmd("realpath --relative-to=\""..base.."\" \""..path.."\"")
    end,

    get_time = function(path)
        return tonumber(cmd("stat -c %Y "..path.." 2>/dev/null"))
    end,

    is_dir = function(path)
        return cmd("[ -d \""..path.."\" ] && echo 1") == "1"
    end,

    is_exist = function(path)
        return cmd("[ -e \""..path.."\" ] && echo 1") == "1"
    end,

    --is_ext = function(path, ext) end
}




-- fs.find = function(path, is_recursive, filter)
--     if is_recursive == nil then is_recursive = false end
-- 
--     if not fs.is_dir(path) then
--         print("fs.find(...): \""..path.."\" is not a directory")
--     end
--         
--     --local dirs = table.new({path})
--     local dirs = {path}
--     local newdirs = {}
--     local files = {}
--     
--     while #dirs ~= 0 do
--         local _, dpath = next(dirs)
--         if dpath == nil then 
--             if #newdirs == 0 then break end
--             
--         end
-- 
--         table.insert(newdirs, dpath)
-- 
--         print(#dirs)
--         
--         for _, fpath in pairs(fs.ls(dpath)) do
--             print(fpath)
--             print(fs.is_dir(fpath))
--             if fs.is_dir(fpath) then
--                 table.insert(dirs, fpath)
--             elseif filter(fs.get_ext(fpath)) then
--                 table.insert(files, fpath)
--             end
--         end
--         
--     end
-- 
--     return files
-- end



fs.find = function(path, is_recursive, filter, _files)
    if is_recursive == nil then is_recursive = false end
    if _files == nil then _files = {} end

    if not fs.is_dir(path) then
        print("fs.find(...): \""..path.."\" is not a directory")
        return false
    end

    local files = fs.ls(path)

    for _, file in pairs(files) do
    
        if fs.is_dir(file) then
            fs.find(file, true, filter, _files)
        elseif filter(file) then
            table.insert(_files, file)
        end
        
    end

    return _files
end



module.fs = fs







local filter = {
    ext = function(ext)
        return function(path)
            return fs.get_ext(path) == ext
        end
    end,

    name = function(name)
        return function(path)
            return fs.get_name(path) == name
        end
    end,
}


module.filter = filter







module.newtree = function()
    return {
    
        targets = setmetatable({}, {
        
            __index = function(self, key)
                return rawget(self, fs.get_fullpath(key))
            end,

            __newindex = function(self, key, value)
                rawset(self, fs.get_fullpath(key), value)
            end,
            
        }),



        -- this is a recursive function
        -- it will call itself until all
        -- dependancies built or older
        -- btw, targets can be directories
        build = function(self, target, is_verbose)
            if is_verbose == nil then is_verbose = false end

            local ttarg = self.targets[target]
        
            -- make sure target exists in buildtree. If not, then return successfully
            if ttarg == nil then return true end
        
            -- get time of target
            local targ_time = fs.get_time(target) or 0

            local should_rebuild = false
            
            if ttarg.force_rebuild == true then should_rebuild = true end
            if #ttarg.depends == 0         then should_rebuild = true end
            
            -- check every dependancy
            for _, path in pairs(ttarg.depends) do
                -- check if it exists in the buildtree
                -- if so then call self:build on it
                if self.targets[path] ~= nil then
                    local ok = self:build(path, is_verbose)
                    if not ok then return ok end
                end

                -- check if dependancy exists in the filesystem
                -- if not then fail build
                if not fs.is_exist(path) then return false end

                -- check if dep is newer than targ
                -- if newer, then set should_rebuild flag
                if (fs.get_time(path) or 0) >= targ_time then
                    should_rebuild = true
                end
            end

            if should_rebuild then
                if is_verbose then print('rebuilding: '..target) end
                ttarg.builder(target, ttarg.depends, ttarg.options)
            elseif is_verbose then print('skipping:   '..target) end

            return true
        end,
    }
end





local gcc = {}

-- return header dependancies
gcc.gen_dep = function(file, opts)
    local str = cmd("gcc "..file.." -MM "..opts)
    str = str:split(":")[2]
    str = str:gsub("\n", " ")
    str = str:gsub("\\", " ")
    local deps = str:squeeze():split()
    -- get rid of first source file
    --return table.concat(table.move(deps, 2, 1, #deps-1), ' ')
    return deps
end


gcc.filter_headers = function(files)
    local rt = {}
    for _, file in pairs(files) do
        if fs.get_ext(file) ~= '.h' then
            table.insert(rt, file)
        end
    end
    return rt
end


gcc.builder = function(outpath, infiles, opts)
    -- make sure output directory exists
    fs.mkdir(fs.get_dir(outpath), "-p")
    -- compile
    local infiles_str = table.concat(gcc.filter_headers(infiles), ' ')
    cmd(("gcc -o %s %s %s"):format(outpath, infiles_str, opts), true)
end


module.gcc = gcc








return module





-- get recursive list of C files in a directory
--local c_srcs = fs.find(dir.src, true, filter.ext(ext.c))
-- replace all c extensions with obj extensions
--local c_objs = change_ext(c_src, ext.o)
