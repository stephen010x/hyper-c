-- buildtree module


local module = {}





local enum = {
    meta = {
        __index = function(self, key)
            return key
        end
        __newindex = function(table, key, value)
            print("writing to ext table not allowed")
        end
    },
}
setmetatable(ext, ext.meta)

module.enum = enum
module.ext  = enum
module.type = enum







local function cmd(command)
    local handle = io.popen(command)
    local out = handle:read("*a")
    local ok, stat = handle:close()
    return out
end

module.cmd = cmd







-- separate string into table using separator
-- adds this behavior to the main string metatable
local stringmt = getmetatable("", strip)
stringmt.__index.split = function(self, sep)
    if sep == nil then sep = " " end
    if sep == "" then return self end

    local out = table.new()
    local index = 1
    local last = 1
    while true do
        start, stop = self:find(sep, index, true)
        if not start then break end
        out:insert(self:sub(last, start-1))
        index = stop + 1
        last = index
    end
    out:insert(self:sub(last))

    if strip == true then
        for key, value in pairs(out) do
            out[key] = value:strip()
        end
    end

    return out
end

-- strip string of whitespace
-- adds method to main string metatable
stringmt.__index.strip = function(self)
    return self:gsub("^%s+", ""):gsub("%s+$", "")
end









table.copy = function(t)
    rt = {}
    for key, value in pairs do
        rt[key] = value
    end
    return rt
end

table.new = function(rtable)
    if rtable == nil then rtable = {} end
    return setmetatable(table.copy(rtable), table)
end

-- implement later
-- table.slice = function(t, first, last)
--     if first < 0
-- end








-- linux file system helper
local fs = {
    ls = function(path)
        if path == nil then path = "."
        local out = cmd("ls \""..path.."\""):split(' ', true)
    end,
    
    cd = function(path)
        if path == nil then path = "~"
        return cmd("cd \""..path.."\"")
    end,

    pwd = function()
        return cmd("pwd -P")
    end,

    get_type = function(path)
        return cmd("file -b \""..path.."\"")
    end,

    get_fullpath = function(path)
        return cmd("realpath \""..path.."\"")
    end,

    -- get file extension from name
    get_ext = function(path)
        local index = path:reverse():find('.', 1, true)
        index = #path - index + 2
        return i and path:sub(index) or ""
    end,

    get_name = function(path)
        return cmd("basename \""..path.."\"")
    end,

    set_ext = function(names, ext)
        if type(names) ~= table then names = {names} end

        local rt = {}

        for key, name in pairs(names) do
            name = name:split('.')[1]
            rt[key] = name..'.'..ext
        end
    end,

    -- returns path relative to base
    get_nearpath = function(base, path)
        return cmd("realpath --relative-to=\""..base.."\" \""..path.."\"")
    end,

    get_time = function(path)
        return math.tointeger(tonumber(cmd("stat -c %Y "..path)))
    end

    is_dir = function(path)
        return cmd("[ -d "..path.." ] && echo 1") == "1"
    end,

    is_exist = function(path)
        return cmd("[ -e "..path.." ] && echo 1") == "1"
    end,

    --is_ext = function(path, ext) end
}



fs.find = function(path, is_recursive, filter)
    if is_recursive == nil then is_recursive = false

    if not fs.is_dir(path) then
        print("fs.find(...): \""..path.."\" is not a directory")
        
    local dirs = table.new({path})
    local files = table.new()

    while #dirs ~= 0 do
        local _, dpath = next(dirs)
        if dpath == nil then break end
        
        for _, fpath in pairs(fs.ls(dpath)) do
            if fs.is_dir(fpath) then
                dirs:insert(fpath)
            elseif filter(fs.get_ext(fpath)) then
                files:insert(fpath)
            end
        end
    end
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
                return rawget(self, fs.fullpath(key))
            end

            __newindex = function(self, key, value)
                rawset(self, fs.fullpath(key), value)
            end
            
        }),


        local should_compile = false


        -- this is a recursive function
        -- it will call itself until all
        -- dependancies built or older
        -- btw, targets can be directories
        build = function(self, target)

            local ttarg = self.targets[target]
        
            -- make sure target exists in buildtree. If not, then return successfully
            if ttarg == nil then return true end
        
            -- get time of target
            local targ_time = fs.get_time(target)

            local should_rebuild = false
            
            if ttarg.force_rebuild == true then should_rebuild = true end
            if #ttarg.depends == 0         then should_rebuild = true end
            
            -- check every dependancy
            for _, path in pairs(ttarg.depends) do
                -- check if it exists in the buildtree
                -- if so then call self:build on it
                if self.buildtree.targets[path] ~= nil then
                    self:build(path)
                end

                -- check if dependancy exists in the filesystem
                -- if not then fail build
                if not fs.is_exist(path) then return false end

                -- check if dep is newer than targ
                -- if newer, then set should_rebuild flag
                if fs.get_time(path) >= targ_time then
                    should_rebuild = true
                end
            end

            if should_rebuild then
                ttarg.builder(ttarg.depends, ttarg.options)
            end
            
        end,
    }
end





local gcc = {}

-- return header dependancies
gcc.gen_dep = function(file, opts)
    local str = cmd("gcc "..file.." -MM "..opts)
    str = str:split(":")[2]
    str = str:gsub("\n", " ", true)
    local deps = str:split()
    -- get rid of first source file
    return table.move(deps, 2, 1, #deps-1)
end


gcc.builder = function(outpath, infiles, opts)
    local infiles_str = table.concat(infiles, ' ')
    cmd("gcc -o %s %s %s":format(outpath, infiles_str, opts))
end


module.gcc = gcc








return module





-- get recursive list of C files in a directory
--local c_srcs = fs.find(dir.src, true, filter.ext(ext.c))
-- replace all c extensions with obj extensions
--local c_objs = change_ext(c_src, ext.o)
