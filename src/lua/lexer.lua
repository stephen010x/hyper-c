
-- /* ########################
--  * ##  !! IMPORTANT !!   ##
--  * ########################
--  *
--  * This document is based almost entierly on these specs and
--  * matching rules:
--  * 
--  * https://port70.net/~nsz/c/c23/n3220.html#A
--  *
--  * ######################## */



local lpeg = require "lpeg"

local P, S, R, V, B   =  lpeg.P, lpeg.S, lpeg.R, lpeg.V, lpeg.B
local C, Cb, Cc, Cf   =  lpeg.C, lpeg.Cb, lpeg.Cc, lpeg.Cf
local Cg, Cp, Cs, Ct  =  lpeg.Cg, lpeg.Cp, lpeg.Cs, lpeg.Ct
local Carg, Cmt       =  lpeg.Cart, lpeg.Cmt

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




clex.digit           = R("19")                           -- range 0 to 9
clex.nonzero_digit   = R("09")
clex.octal_digit     = R("07")
clex.hex_digit       = R("09") + R("af") + R("AF")
clex.binary_digit    = S("01")                           -- one of these digits 0 or 1
clex.nondigit        = R("az") + R("AZ") + P("_")        -- range a to z and A to Z
clex.hex_prefix      = P("0x0") + P("0X")
clex.binary_prefix   = P("0b") + P("0B")
clex.sign            = S("+-")
clex.encoding_prefix = S"uUL" + P"u8"
clex.punctuation     = S("_{}[]#()<>%:;.?*+-/^&|~!=,\\\"'")
clex.newline         = P("\n")
clex.whitespace      = S(" \t\v\f\n")
clex.alphanumeric    = clex.digit + clex.nondigit
clex.source_char     = clex.alphanumeric + clex.punctuation + clex.whitespace

clex.c_char = clex.source_char - P("'")  - P("\\") - P("\n") -- any char with these exceptions
clex.s_char = clex.source_char - P("\"") - P("\\") - P("\n") -- any char with these exceptions
clex.h_char = clex.source_char - P(">")  - P("\n")
clex.q_char = clex.source_char - P("\"") - P("\n")






clex.keyword = lpeg.P({
    "KEYWORD",
    KEYWORD = 
        P("alignas")       +
        P("alignof")       +
        P("auto")          +
        P("bool")          +
        P("break")         +
        P("case")          +
        P("char")          +
        P("const")         +
        P("constexpr")     +
        P("continue")      +
        P("default")       +
        P("do")            +
        P("double")        +
        P("else")          +
        P("enum")          +
        P("extern")        +
        P("false")         +
        P("float")         +
        P("for")           +
        P("goto")          +
        P("if")            +
        P("inline")        +
        P("int")           +
        P("long")          +
        P("nullptr")       +
        P("register")      +
        P("restrict")      +
        P("return")        +
        P("short")         +
        P("signed")        +
        P("sizeof")        +
        P("static")        +
        P("static_assert") +
        P("struct")        +
        P("switch")        +
        P("thread_local")  +
        P("true")          +
        P("typedef")       +
        P("typeof")        +
        P("typeof")        +
        P("_unqual")       +
        P("union")         +
        P("unsigned")      +
        P("void")          +
        P("volatile")      +
        P("while")         +
        P("_Atomic")       +
        P("_BitInt")       +
        P("_Complex")      +
        P("_Decimal128")   +
        P("_Decimal32")    +
        P("_Decimal64")    +
        P("_Generic")      +
        P("_Imaginary")    +
        P("_Noreturn")     +

        -- custom keywords (soon to be more. Way more)
        P("hyper"), 
}) * clex.alphanumeric^-1     -- ensure that no alphanumeric characters follow after a match








-- ordered by length, so that it tries the longest ones first
clex.punctuator = lpeg.P({
    "PUNCTUATOR",
    PUNCTUATOR =
        P("...") +
        P("<<=") +
        P(">>=") +
        P("->")  +
        P("++")  +
        P("--")  +
        P("<<")  +
        P(">>")  +
        P("<=")  +
        P(">=")  +
        P("==")  +
        P("!=")  +
        P("&&")  +
        P("||")  +
        P("::")  +
        P("*=")  +
        P("/=")  +
        P("%=")  +
        P("+=")  +
        P("-=")  +
        P("&=")  +
        P("^=")  +
        P("|=")  +
        P("##")  +
        P(".")   +
        P("[")   +
        P("]")   +
        P("(")   +
        P(")")   +
        P("{")   +
        P("}")   +
        P("&")   +
        P("*")   +
        P("+")   +
        P("-")   +
        P("~")   +
        P("!")   +
        P("/")   +
        P("%")   +
        P("<")   +
        P(">")   +
        P("^")   +
        P("|")   +
        P("?")   +
        P(":")   +
        P(";")   +
        P("=")   +
        P(",")   +
        P("#"),
        -- diagraphs not included for now
})








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








clex.constant = lpeg.P({
    "CONSTANT",


    CONSTANT = (V"INTEGER_CONSTANT"    * Cc"integer")    / pack_constant +
               (V"FLOATING_CONSTANT"   * Cc"floating")   / pack_constant +
               --V"ENUMERATION_CONSTANT" +
               (V"CHARACTER_CONSTANT"  * Cc"character")  / pack_constant +
               (V"PREDEFINED_CONSTANT" * Cc"predefined") / pack_constant,


    -- optional integer suffix
    INTEGER_CONSTANT = (C(V"DECIMAL_CONSTANT")     * C(V"INTEGER_SUFFIX"^-1) * Cc"decimal")     / pack_integer +
                       (C(V"OCTAL_CONSTANT")       * C(V"INTEGER_SUFFIX"^-1) * Cc"octal")       / pack_integer +
                       (C(V"HEXADECIMAL_CONSTANT") * C(V"INTEGER_SUFFIX"^-1) * Cc"hexadecimal") / pack_integer +
                       (C(V"BINARY_CONSTANT")      * C(V"INTEGER_SUFFIX"^-1) * Cc"binary")      / pack_integer,


    -- decimal is leading nonzero followed by zero or more digits
    -- octal is leading zero followed by one or more digits, etc.
    DECIMAL_CONSTANT     = clex.nonzero_digit * clex.digit^0,
    OCTAL_CONSTANT       = P("0")             * clex.octal_digit^1,
    HEXADECIMAL_CONSTANT = clex.hex_prefix    * clex.hex_digit^1,
    BINARY_CONSTANT      = clex.binary_prefix * clex.binary_digit^1,


    FLOATING_CONSTANT = (V"DECIMAL_FLOATING_CONSTANT"     * Cc"decimal")     / pack_floating +
                        (V"HEXADECIMAL_FLOATING_CONSTANT" * Cc"hexadecimal") / pack_floating,


    DECIMAL_FLOATING_CONSTANT = C(V"FRACTIONAL_CONSTANT" * V"EXPONENT_PART"^-1) * C(V"FLOATING_SUFFIX"^-1) +
                                C(clex.digit^1           * V"EXPONENT_PART")    * C(V"FLOATING_SUFFIX"^-1),


    HEXADECIMAL_FLOATING_CONSTANT = C(clex.hex_prefix *
                                        (V"HEXADECIMAL_FRACTIONAL_CONSTANT" + clex.hex_digit^1) *
                                            V"BINARY_EXPONENT_PART") * C(V"FLOATING_SUFFIX"^-1),


    -- set this later
    --ENUMERATION_CONSTANT = V("IDENTIFIER"),


    CHARACTER_CONSTANT = (C(clex.encoding_prefix^-1) * P"'" * C(clex.c_char^1) * P"'") / pack_character,


    PREDEFINED_CONSTANT = C(P("false") + P("true") + P("nullptr")) / pack_predefined,


    FRACTIONAL_CONSTANT = clex.digit^0 * P(".") * clex.digit^1 +
                          clex.digit^1 * P("."),


    HEXADECIMAL_FRACTIONAL_CONSTANT = clex.hex_digit^0 * P(".") * clex.hex_digit^1 +
                                      clex.hex_digit^1 * P("."),


    EXPONENT_PART = S("eE") * clex.sign^-1 * clex.digit^1,


    BINARY_EXPONENT_PART = S("pP") * clex.sign^-1 * clex.digit^1,


    INTEGER_SUFFIX = S"uU" * (S"lL" + P"ll" + P"LL" + P"wb" + P"WB")^-1 + 
                     (S"lL" + P"ll" + P"LL" + P"wb" + P"WB") * S"uU"^-1,


    FLOATING_SUFFIX = S"flFL" + P"df" + P"dd" + P"dl" + P"DF" + P"DD" + P"DL",
    
})








local pack_identifier = pack_predefined
local pack_string_literal = pack_character
local pack_punctuator = pack_predefined
local pack_preprocessing_directive = pack_predefined


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
            ( Cp() * C(clex.keyword)    * Cp() * Cc"keywords"       ) / pack_token +
            ( Cp() * V"IDENTIFIER"      * Cp() * Cc"identifier"     ) / pack_token +
            ( Cp() * clex.constant      * Cp() * Cc"constant"       ) / pack_token +
            ( Cp() * V"STRING_LITERAL"  * Cp() * Cc"string_literal" ) / pack_token +
            ( Cp() * C(clex.punctuator) * Cp() * Cc"punctuator"     ) / pack_token +
            -- ideally I dont want the lexer to fail. Let the parser handle it
            ( Cp() * P(1)               * Cp() * Cc"unknown"        ) / pack_token,
            
            
            


    -- leading nondigit, followed by 0 or more alphanumeric characters
    IDENTIFIER = C(clex.nondigit * clex.alphanumeric^0) / pack_identifier,


    -- TODO: I have half a mind to place the STRING_LITERAL in the constants section
    --       next to the character constant.
    STRING_LITERAL = (C(clex.encoding_prefix^-1) * P"\"" * C(clex.s_char^0) * P"\"") / pack_string_literal,


    -- doesn't consume trailing newline
    COMMENT = P"//" * (P(1) - P"\n")^0 * #P"\n" +
              -- P"/*" * (P(1) - P"*/")^0 * (-B"\\" * P"*/"),
              P"/*" * (P(1) - P"*/")^0 * P"*/",


    WHITESPACE = clex.whitespace^1,


    -- doesn't consume trailing newline
    -- will consume any newline preceeded by a backslash '\'
    -- must be preceeded by a newline with optional whitespace
    PREPROCESSING_DIRECTIVE = P"\n" * (clex.whitespace - P"\n")^0 * 
                                  C(P"#" * ((1 - P"\n") + P"\\\n")) / pack_preprocessing_directive * 
                                      #P"\n",


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







return clex;
