-- local lex   = require "grammar/c"
local dtools = require "dtools"
local lex    = require "lexer"
local par    = require "parser"



-- read file
local file, err = io.open("test/main.c")
if not file then error(err) end
local fstring = file:read("*a")
file:close()


local tokens = lex.tokenlist:match(fstring)


print(dtools.print_table(tokens, true, '\n', '    '))

local undefc = 0
for i, token in ipairs(tokens) do
    if token.type == "undefined" then
        undefc = undefc + 1
        print(("WARNING: token [%d] undefined"):format(i))
    end
end
print(("NOTE: %d tokens were undefined"):format(undefc))


local entok = lex.encode_token_list(tokens)

-- print(dtools.print_table(, true, '\n', '\t'))

local tree = par.grammar:match(entok)


-- print(dtools.print_table(tree, true, '\n', '  '))


function print_tree(tree, tokens, depth)
    depth = depth or -1
    space = "| "
    
    for i = 0, depth do io.write(space) end
    if (tree.mode or 1) < 1 then
        io.write("INVALID ")
    end
    io.write(("%s (%d)\n"):format(tree.type, tree.mode or 0))
    
    for _, node in ipairs(tree) do
        for i = 0, depth do io.write(space) end
        if type(node) == "table" then
            print_tree(node, tokens, depth+1)
        elseif type(node) == "number" then
            token = tokens[node]
            io.write(("%s%s: \"%s\"\n"):format(space, token.type, token[token.type]))
        else
            io.write(("%s<unknown>\n"):format(space))
        end
    end
end


print_tree(tree, tokens)
