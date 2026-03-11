
local lex   = require "grammar/c"
local dtools = require "./debug" -- rename to dtools



-- read file
local file, err = io.open("../../test/main.c")
if not file then error(err) end
local fstring = file:read("*a")
file:close()


local tokens = lex.tokenlist:match(fstring)


print(dtools.print_table(tokens, true, '\n', '\t'))
