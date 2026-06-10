
-- /* ########################
--  * ##  !! IMPORTANT !!   ##
--  * ########################
--  *
--  * This document is based almost entierly on these specs and
--  * matching rules:
--  * 
--  * https://port70.net/~nsz/c/c23/n3220.html#A.1
--  *
--  * ######################## */


-- TODO: I should find a way to pre-run this and store it as a binary encoded lua state that can be
-- easily and quickly used.



local lpeg = require "lpeg"

local P, S, R, V, B   =  lpeg.P, lpeg.S, lpeg.R, lpeg.V, lpeg.B
local C, Cb, Cc, Cf   =  lpeg.C, lpeg.Cb, lpeg.Cc, lpeg.Cf
local Cg, Cp, Cs, Ct  =  lpeg.Cg, lpeg.Cp, lpeg.Cs, lpeg.Ct
local Carg, Cmt       =  lpeg.Carg, lpeg.Cmt

-- // For the C grammar rules:
-- // https://port70.net/~nsz/c/c23/n3220.html#A

-- For the matcher:
-- https://www.inf.puc-rio.br/~roberto/lpeg/



-- DON'T HANDLE THE PREPROCESSOR, EXCEPT FOR NECCESSARY ELEMENTS. 
-- LIKE LINE NUMBER, ETC. OTHERWISE JUST IGNORE AND MIRROR ALL PREPROCESSOR TOKENS



local clex = {}



-- Read up on phases of translation
-- https://en.cppreference.com/w/c/language/translation_phases.html
-- 
-- PHASE 1:
-- Map bytes of file to source character set
--
-- PHASE 2:
-- etc etc.




clex.digit           = R("09")                           -- range 0 to 9
clex.nonzero_digit   = R("19")
clex.octal_digit     = R("07")
clex.hex_digit       = R("09") + R("af") + R("AF")
clex.binary_digit    = S("01")                           -- one of these digits 0 or 1
clex.nondigit        = R("az") + R("AZ") + P("_")        -- range a to z and A to Z
clex.hex_prefix      = P("0x") + P("0X")
clex.binary_prefix   = P("0b") + P("0B")
clex.sign            = S("+-")
clex.encoding_prefix = S"uUL" + P"u8"
clex.punctuation     = S("_{}[]#()<>%:;.?*+-/^&|~!=,\\\"'")
clex.newline         = P("\n")
clex.whitespace      = S(" \t\v\f\n")
clex.alphanumeric    = clex.digit + clex.nondigit
clex.source_char     = clex.alphanumeric + clex.punctuation + clex.whitespace

-- clex.c_char = clex.source_char - P("'")  - P("\\") - P("\n") -- any char with these exceptions
-- clex.s_char = clex.source_char - P("\"") - P("\\") - P("\n") -- any char with these exceptions
-- clex.h_char = clex.source_char - P(">")  - P("\n")
-- clex.q_char = clex.source_char - P("\"") - P("\n")





-- keyword      - 55ish keywords
-- punctuator   - 49ish punctuators
-- identifier
-- constant
-- string_literal
-- unknown
clex.token_types = {
    ["keyword"]         = 1,
    ["punctuator"]      = 2,
    ["identifier"]      = 3,
    ["constant"]        = 4,
    ["string_literal"]  = 5,
    ["unknown"]         = 6,
}




clex.keyword_list = {
    ["alignas"]     = -1,
    ["alignof"]     = -1,
    ["auto"]        = -1,
    ["bool"]        = -1,
    ["break"]       = -1,
    ["case"]        = -1,
    ["char"]        = -1,
    ["const"]       = -1,
    ["constexpr"]   = -1,
    ["continue"]    = -1,
    ["default"]     = -1,
    ["do"]          = -1,
    ["double"]      = -1,
    ["else"]        = -1,
    ["enum"]        = -1,
    ["extern"]      = -1,
    ["false"]       = -1,
    ["float"]       = -1,
    ["for"]         = -1,
    ["goto"]        = -1,
    ["if"]          = -1,
    ["inline"]      = -1,
    ["int"]         = -1,
    ["long"]        = -1,
    ["nullptr"]     = -1,
    ["register"]    = -1,
    ["restrict"]    = -1,
    ["return"]      = -1,
    ["short"]       = -1,
    ["signed"]      = -1,
    ["sizeof"]      = -1,
    ["static"]      = -1,
    ["static_assert"] = -1,
    ["struct"]      = -1,
    ["switch"]      = -1,
    ["thread_local"] = -1,
    ["true"]        = -1,
    ["typedef"]     = -1,
    ["typeof"]      = -1,
    ["typeof_unqual"] = -1,
    ["_unqual"]     = -1,
    ["union"]       = -1,
    ["unsigned"]    = -1,
    ["void"]        = -1,
    ["volatile"]    = -1,
    ["while"]       = -1,
    ["_Atomic"]     = -1,
    ["_BitInt"]     = -1,
    ["_Complex"]    = -1,
    ["_Decimal128"] = -1,
    ["_Decimal32"]  = -1,
    ["_Decimal64"]  = -1,
    ["_Generic"]    = -1,
    ["_Imaginary"]  = -1,
    ["_Noreturn"]   = -1,

    -- custom keywords (soon to be more. Way more)
    ["hyper"]       = -1,
}



clex.punctuator_list = {
    ["..."] = -1,
    ["<<="] = -1,
    [">>="] = -1,
    ["->"]  = -1,
    ["++"]  = -1,
    ["--"]  = -1,
    ["<<"]  = -1,
    [">>"]  = -1,
    ["<="]  = -1,
    [">="]  = -1,
    ["=="]  = -1,
    ["!="]  = -1,
    ["&&"]  = -1,
    ["||"]  = -1,
    ["::"]  = -1,
    ["*="]  = -1,
    ["/="]  = -1,
    ["%="]  = -1,
    ["+="]  = -1,
    ["-="]  = -1,
    ["&="]  = -1,
    ["^="]  = -1,
    ["|="]  = -1,
    ["##"]  = -1,
    ["."]   = -1,
    ["["]   = -1,
    ["]"]   = -1,
    ["("]   = -1,
    [")"]   = -1,
    ["{"]   = -1,
    ["}"]   = -1,
    ["&"]   = -1,
    ["*"]   = -1,
    ["+"]   = -1,
    ["-"]   = -1,
    ["~"]   = -1,
    ["!"]   = -1,
    ["/"]   = -1,
    ["%"]   = -1,
    ["<"]   = -1,
    [">"]   = -1,
    ["^"]   = -1,
    ["|"]   = -1,
    ["?"]   = -1,
    [":"]   = -1,
    [";"]   = -1,
    ["="]   = -1,
    [","]   = -1,
    ["#"]   = -1,    -- diagraphs not included for now
}




clex.keyword = lpeg.P(false)

local i = 1
for key, _ in pairs(clex.keyword_list) do
    clex.keyword_list[key] = i
    clex.keyword = clex.keyword + P(key)
    i = i + 1
end

-- ensure that no alphanumeric characters follow after a match
clex.keyword = clex.keyword * clex.alphanumeric^-1





clex.punctuator = lpeg.P(false)

-- longest ones added first to make sure they are matched with first
for len = 3, 1, -1 do
    local i = 1
    for key, _ in pairs(clex.punctuator_list) do
        clex.punctuator_list[key] = i
        if #key == len then
            clex.punctuator = clex.punctuator + P(key)
        end
        i = i + 1
    end
end

-- ensure that no alphanumeric characters follow after a match
clex.keyword = clex.keyword * clex.alphanumeric^-1










-- NOTE: I've decided to offload most captures to functions
--       as it improves readability, as well as consistancy


function pack_integer(string, suffix, type)
    return {
        type   = type,
        suffix = suffix,
        string = string,
    }
end


local pack_floating = pack_integer


function pack_character(prefix, string)
    return {
        prefix = suffix,
        string = string,
    }
end


function pack_predefined(string)
    return string
end


function pack_constant(table, type)
    return {
        type = type,
        [type] = table,
    }
end




-- clex.c_char = clex.source_char - P("'")  - P("\\") - P("\n") -- any char with these exceptions
-- clex.s_char = clex.source_char - P("\"") - P("\\") - P("\n") -- any char with these exceptions

-- clex.simple_escape = P("\\") * S("'\"?\\abfnrtv") 
-- clex.octal_escape = P("\\") + clex.octal_digit * clex.octal_digit^-2
-- clex.hex_escape = P("\\x") + clex.hex_digit^1
-- clex.unichar = (P("\\u") + clex.hex_digit^4 - clex.hex_digit) + (P("\\U") + clex.hex_digit^8 - clex.hex_digit)
-- clex.escape = clex.simple_escape + clex.octal_escape + clex.hex_escape + clex.unichar


function unescaped(str)
    return P(str) - B("\\")
end

clex.c_char = clex.source_char - (P("'" ) - B("\\")) - P("\n") --+ clex.escape -- - P("\\") 
clex.s_char = clex.source_char - (P("\"") - B("\\")) - P("\n") --+ clex.escape -- - P("\\") 





clex.constant = lpeg.P({
    "CONSTANT",


    CONSTANT = 
        (V"FLOATING_CONSTANT"   * Cc"floating")   / pack_constant +
        (V"INTEGER_CONSTANT"    * Cc"integer")    / pack_constant +
        --V"ENUMERATION_CONSTANT" +
        (V"CHARACTER_CONSTANT"  * Cc"character")  / pack_constant +
        (V"PREDEFINED_CONSTANT" * Cc"predefined") / pack_constant,


    -- optional integer suffix
    INTEGER_CONSTANT =
        (C(V"DECIMAL_CONSTANT")     * C(V"INTEGER_SUFFIX"^-1) * Cc"decimal")     / pack_integer +
        (C(V"OCTAL_CONSTANT")       * C(V"INTEGER_SUFFIX"^-1) * Cc"octal")       / pack_integer +
        (C(V"HEXADECIMAL_CONSTANT") * C(V"INTEGER_SUFFIX"^-1) * Cc"hexadecimal") / pack_integer +
        (C(V"BINARY_CONSTANT")      * C(V"INTEGER_SUFFIX"^-1) * Cc"binary")      / pack_integer,


    -- decimal is leading nonzero followed by zero or more digits
    -- octal is leading zero followed by one or more digits, etc.
    DECIMAL_CONSTANT     = clex.nonzero_digit * clex.digit^0,
    OCTAL_CONSTANT       = P("0")             * clex.octal_digit^0,
    HEXADECIMAL_CONSTANT = clex.hex_prefix    * clex.hex_digit^1,
    BINARY_CONSTANT      = clex.binary_prefix * clex.binary_digit^1,


    FLOATING_CONSTANT = (V"DECIMAL_FLOATING_CONSTANT"     * Cc"decimal")     / pack_floating +
                        (V"HEXADECIMAL_FLOATING_CONSTANT" * Cc"hexadecimal") / pack_floating,


    DECIMAL_FLOATING_CONSTANT = C(V"FRACTIONAL_CONSTANT" * V"EXPONENT_PART"^-1) * C(V"FLOATING_SUFFIX"^-1) +
                                C(clex.digit^1           * V"EXPONENT_PART")    * C(V"FLOATING_SUFFIX"^-1),


    HEXADECIMAL_FLOATING_CONSTANT = C(clex.hex_prefix *
                                        (V"HEXADECIMAL_FRACTIONAL_CONSTANT" + clex.hex_digit^1) *
                                            V"BINARY_EXPONENT_PART") * C(V"FLOATING_SUFFIX"^-1),


    -- set this later, maybe
    --ENUMERATION_CONSTANT = V("IDENTIFIER"),


    CHARACTER_CONSTANT = (C(clex.encoding_prefix^-1) * (P("'") - B("\\")) * C(clex.c_char^1) *
                         (P("'") - B("\\"))) / pack_character,


    PREDEFINED_CONSTANT = C(P("false") + P("true") + P("nullptr")) / pack_predefined,


    FRACTIONAL_CONSTANT = clex.digit^0 * P(".") * clex.digit^1 +
                          clex.digit^1 * P("."),


    HEXADECIMAL_FRACTIONAL_CONSTANT = clex.hex_digit^0 * P(".") * clex.hex_digit^1 +
                                      clex.hex_digit^1 * P("."),


    EXPONENT_PART = S("eE") * clex.sign^-1 * clex.digit^1,


    BINARY_EXPONENT_PART = S("pP") * clex.sign^-1 * clex.digit^1,


    INTEGER_SUFFIX = S"uU" * (P"ll" + P"LL" + P"wb" + P"WB" + S"lL")^-1 + 
                     (P"ll" + P"LL" + P"wb" + P"WB" + S"lL") * S"uU"^-1,


    FLOATING_SUFFIX = P"df" + P"dd" + P"dl" + P"DF" + P"DD" + P"DL" + S"flFL",
    
})








local pack_identifier = pack_predefined
local pack_string_literal = pack_character
local pack_punctuator = pack_predefined
local pack_preprocessing_directive = pack_predefined


-- TODO: try to add a way to keep track of line number and column in this
function pack_token(start, table, stop, type)
    return {
        index  = tonumber(start),
        len    = tonumber(stop) - tonumber(start),
        type   = type,
        [type] = table,
    }
end








-- this will only match one token at a time, so run this repeatedly until there are no more matches
clex.token = lpeg.P({
    "TOKEN",    -- initial rule name


    TOKEN = V"TOKEN",


            -- directives need to be checked before whitespace
    TOKEN = (Cp() * V"PREPROCESSING_DIRECTIVE" * Cp() * Cc"preprocessing_directive") / pack_token +
            V("WHITESPACE") +
            V("COMMENT") +
            ( Cp() * C(clex.keyword)    * Cp() * Cc"keyword"       ) / pack_token +
            ( Cp() * V"IDENTIFIER"      * Cp() * Cc"identifier"     ) / pack_token +
            ( Cp() * clex.constant      * Cp() * Cc"constant"       ) / pack_token +
            ( Cp() * V"STRING_LITERAL"  * Cp() * Cc"string_literal" ) / pack_token +
            ( Cp() * C(clex.punctuator) * Cp() * Cc"punctuator"     ) / pack_token +
            -- ideally I dont want the lexer to fail. Let the parser handle it
            ( Cp() * C(clex.alphanumeric^1 + P(1)) * Cp() * Cc"unknown" ) / pack_token,
            
            
            


    -- leading nondigit, followed by 0 or more alphanumeric characters
    IDENTIFIER = C(clex.nondigit * clex.alphanumeric^0) / pack_identifier,


    -- TODO: I have half a mind to place the STRING_LITERAL in the constants section
    --       next to the character constant.
    STRING_LITERAL = (C(clex.encoding_prefix^-1) * (P("\"") - B("\\")) * C(clex.s_char^0) *
                     (P("\"") - B("\\"))) / pack_string_literal,


    -- doesn't consume trailing newline
    COMMENT = P"//" * (P(1) - P"\n")^0 * #P"\n" +
              -- P"/*" * (P(1) - P"*/")^0 * (-B"\\" * P"*/"),
              P"/*" * (P(1) - P"*/")^0 * P"*/",


    -- -- separate newlines and other whitespace into distinct tokens
    -- -- so that it doesn't impede on the preprocessing directive
    -- -- for instance: "   \n\n\n  #  define "
    -- WHITESPACE = (clex.whitespace - P"\n")^1 + 
    --              P"\n"^1,

    -- nevermind. It isn't like the whitespace is captured by the
    -- preprocessing directive anyway
    WHITESPACE = clex.whitespace^1,


    -- doesn't consume trailing newline
    -- will consume any newline preceeded by a backslash '\'
    -- must be preceeded by a newline with optional whitespace
    -- TODO: This needs to be tested!
    PREPROCESSING_DIRECTIVE = P"\n" * (clex.whitespace - P"\n")^0 * 
                                  C(P"#" * (clex.whitespace - P"\n")^0 * ((1 - P"\n") + P"\\\n")) / 
                                      pack_preprocessing_directive * #P"\n",


}) --* P(-1) -- matches with the end of the document (read lpeg docs)
           -- it is applied only to the start rule pattern







-- I am unsure if when constant is referenced a copy is created, or if it is an actual reference
-- if just a reference, then this should work.
-- removed, because it will conflict with identifiers (might as well be considered one anyway)
--clex.constant.ENUMERATION_CONSTANT = clex.grammar.IDENTIFIER








-- TODO: maybe not implement this
-- clex.preprocessor = lpeg.P({
--     "PREPROCESSING_TOKEN",
-- 
-- 
--     PREPROCESSING_TOKEN = V("HEADER_NAME") + 
--                          clex.grammar.IDENTIFIER + 
--                          V("PP_NUMBER") +
--                          clex.constant.CHARACTER_CONSTANT +
--                          clex.grammar.STRING_LITERAL +
--                          clex.punctuator, --+
--                          --clex.non_whitespace, --each non-white-space character not one of the above
-- 
-- 
--     HEADER_NAME = P("<")  * clex.h_char^1 * P(">") +
--                   P("\"") * clex.q_char^1 * P("\""),
-- 
-- 
--     PP_NUMBER = P(".")^-1 * clex.digit * (P("'")^-1 * clex.alphanumeric +
--                                           S("eEpP") * clex.sign         +
--                                           P(".")                         )^0,
--     
-- })





clex.tokenlist = lpeg.P({
    "TOKENLIST",

    -- P(-1) matches with the end of the document (read lpeg docs)
    TOKENLIST = Ct(clex.token^0) * P(-1),
    
})




-- there are 6 basic token types. 
-- keyword      - 55ish keywords
-- punctuator   - 49ish punctuators
-- identifier
-- constant
-- string_literal
-- unknown

-- So that is more than a byte to encode. So, we just go with two bytes.

-- Say. What if I didn't need the heavy overhead anyway? What if I didn't even need C for this?
-- If I encode the tokenlist into bytes, I can just match with those bytes too like normal.




clex.encode_token = function(token)
    if clex.token_types[token.type] == nil then
        error("invalid token")
    end

    if token.type == "keyword" then
        return clex.token_type[token.type], clex.keyword_list[token.keyword]
    elseif token.type == "punctuator" then
        return clex.token_type[token.type], clex.punctuator_list[token.punctuator]
    else
        return clex.token_type[token.type], 0
    end
end



clex.encode_token_list = function(tokens)
    local buff = {}
    for token in ipairs(tokens) do
        local b1, b2 = clex.encode_token(token)
        table.insert(buff, b1)
        table.insert(buff, b2)
    end
    return string.char(table.unpack(buff))
end


clex.match = {}


-- clex.match.keyword = Cmt( P(2), lhook_iskeyword )
-- getmetatable(clex.match.keyword).__call = function(name) return lhook_keyword(name) end
-- 
-- 
-- clex.match.identifier = P(
--     
-- )
-- setmetatable(clex.match.identifier, {
--     __call = function(name)
-- 
--     end
-- })




-- clex.match.keyword = P(string.char(clex.token_types.keyword)) * P(1)
 
-- local meta = getmetatable(clex.match.keyword)
-- local dtools = require "dtools"
-- print(dtools.print_table(meta, true, '\n', '\t'))


-- getmetatable(clex.match.keyword).__call = function(_, name)
clex.match.keyword = function(name)
    if not clex.is_keyword(name) then
        error(("\"%s\" not a recognized keyword"):format(name), 2)
    end
    return P(string.char(clex.token_types.keyword, clex.keyword_list[name]))
end


-- clex.match.punctuator = P(string.char(clex.token_types.punctuator)) * P(1)

-- getmetatable(clex.match.punctuator).__call = function(_, name)
clex.match.punctuator = function(name)
    if not clex.is_punctuator(name) then
        error(("\"%s\" not a recognized punctuator"):format(name), 2)
    end
    return P(string.char(clex.token_types.punctuator, clex.punctuator_list[name]))
end


clex.match.constant       = P(string.char(clex.token_types.punctuator))     * P(1)
clex.match.string_literal = P(string.char(clex.token_types.string_literal)) * P(1)
clex.match.identifier     = P(string.char(clex.token_types.identifier))     * P(1)


clex.is_keyword = function(name)
    return (clex.keyword_list[name] or 0) > 0
end

clex.is_punctuator = function(name)
    return (clex.punctuator_list[name] or 0) > 0
end






return clex






--[[
    Lets plan how I want the output to look

    TOKEN = {
        -- use index and len to access the token string
        .index
        .len
        .type = [identifier, constant, string_literal, punctuator, preprocessing_directive]

        --[\[IF IDENTIFIER]\]-- 
        .identifier = {
            .string
        }

        --[\[IF CONSTANT]\]-- 
        .constant = {
            .type = [integer, floating, character, predefined]

            --[\[IF INTEGER]\]-- 
            .integer = {
                .suffix
                .type = [decimal, octal, hexadecimal, binary]
                .string      -- string value without suffix
            }

            --[\[IF FLOATING]\]-- 
            .floating = {
                .suffix
                .type = [decimal, hexadecimal]
                .string      -- string value without suffix
            }

            --[\[IF CHARACTER]\]-- 
            .character = {
                .prefix
                .string
            }
            
            --[\[IF PREDEFINED]\]-- 
            .predefined.string
        }

        --[\[IF STRING_LITERAL]\]-- 
        .string_literal = {
            .prefix
            .string
        }
        
        --[\[IF PUNCTUATOR]\]-- 
        .punctuator.string
        
        --[\[IF PREPROCESSING_DIRECTIVE]\]-- 
        .preprocessing_directive.string
    }



    For evaluating integers and floats, there is the wonder sscanf
    https://en.cppreference.com/w/c/io/fscanf

    This way I dont have to store integer prefixes, nor float information like exponents.
    And it can generally evaluate everything except binary prefixes. So the type will still
    need to be stored.
]]--






-- clex.keyword = lpeg.P({
--     "KEYWORD",
--     KEYWORD = 
--         P("alignas")       +
--         P("alignof")       +
--         P("auto")          +
--         P("bool")          +
--         P("break")         +
--         P("case")          +
--         P("char")          +
--         P("const")         +
--         P("constexpr")     +
--         P("continue")      +
--         P("default")       +
--         P("do")            +
--         P("double")        +
--         P("else")          +
--         P("enum")          +
--         P("extern")        +
--         P("false")         +
--         P("float")         +
--         P("for")           +
--         P("goto")          +
--         P("if")            +
--         P("inline")        +
--         P("int")           +
--         P("long")          +
--         P("nullptr")       +
--         P("register")      +
--         P("restrict")      +
--         P("return")        +
--         P("short")         +
--         P("signed")        +
--         P("sizeof")        +
--         P("static")        +
--         P("static_assert") +
--         P("struct")        +
--         P("switch")        +
--         P("thread_local")  +
--         P("true")          +
--         P("typedef")       +
--         P("typeof")        +
--         P("typeof")        +
--         P("_unqual")       +
--         P("union")         +
--         P("unsigned")      +
--         P("void")          +
--         P("volatile")      +
--         P("while")         +
--         P("_Atomic")       +
--         P("_BitInt")       +
--         P("_Complex")      +
--         P("_Decimal128")   +
--         P("_Decimal32")    +
--         P("_Decimal64")    +
--         P("_Generic")      +
--         P("_Imaginary")    +
--         P("_Noreturn")     +
-- 
--         -- custom keywords (soon to be more. Way more)
--         P("hyper"), 
-- }) * clex.alphanumeric^-1     -- ensure that no alphanumeric characters follow after a match






-- clex.punctuator = lpeg.P({
--     "PUNCTUATOR",
--     PUNCTUATOR =
--         P("...") +
--         P("<<=") +
--         P(">>=") +
--         P("->")  +
--         P("++")  +
--         P("--")  +
--         P("<<")  +
--         P(">>")  +
--         P("<=")  +
--         P(">=")  +
--         P("==")  +
--         P("!=")  +
--         P("&&")  +
--         P("||")  +
--         P("::")  +
--         P("*=")  +
--         P("/=")  +
--         P("%=")  +
--         P("+=")  +
--         P("-=")  +
--         P("&=")  +
--         P("^=")  +
--         P("|=")  +
--         P("##")  +
--         P(".")   +
--         P("[")   +
--         P("]")   +
--         P("(")   +
--         P(")")   +
--         P("{")   +
--         P("}")   +
--         P("&")   +
--         P("*")   +
--         P("+")   +
--         P("-")   +
--         P("~")   +
--         P("!")   +
--         P("/")   +
--         P("%")   +
--         P("<")   +
--         P(">")   +
--         P("^")   +
--         P("|")   +
--         P("?")   +
--         P(":")   +
--         P(";")   +
--         P("=")   +
--         P(",")   +
--         P("#"),
--         -- diagraphs not included for now
-- })
