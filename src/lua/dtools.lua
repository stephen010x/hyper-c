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




return debug
