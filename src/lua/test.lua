-- local lex   = require "grammar/c"
local lex    = require "lexer"
local dtools = require "dtools"



-- read file
local file, err = io.open("test/main.c")
if not file then error(err) end
local fstring = file:read("*a")
file:close()


local tokens = lex.tokenlist:match(fstring)


print(dtools.print_table(tokens, true, '\n', '\t'))

local undefc = 0
for i, token in ipairs(tokens) do
    if token.type == "undefined" then
        undefc = undefc + 1
        print(("WARNING: token [%d] undefined"):format(i))
    end
end
print(("NOTE: %d tokens were undefined"):format(undefc))
