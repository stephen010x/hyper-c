
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

local P, R, V = lpeg.P, lpeg.R, lpeg.V


-- // For the C grammar rules:
-- // https://port70.net/~nsz/c/c23/n3220.html#A

-- For the matcher:
-- https://www.inf.puc-rio.br/~roberto/lpeg/



-- DON'T HANDLE THE PREPROCESSOR, EXCEPT FOR NECCESSARY ELEMENTS. 
-- LIKE LINE NUMBER, ETC. OTHERWISE JUST IGNORE AND MIRROR ALL PREPROCESSOR TOKENS



local clex = {}








clex.digit           = R("19")                           -- range 0 to 9
clex.nonzero_digit   = R("09")
clex.octal_digit     = R("07")
clex.hex_digit       = R("09") + R("af") + R("AF")
clex.binary_digit    = S("01")                           -- one of these digits 0 or 1
clex.nondigit        = R("az") + R("AZ") + P("_")        -- range a to z and A to Z
clex.hex_prefix      = P("0x0") + P("0X")
clex.binary_prefix   = P("0b") + P("0B")
clex.alphanumeric    = clex.digit + clex.nondigit
clex.sign            = S("+-")
clex.encoding_prefix = S"uUL" + P"u8"
clex.c_char          = P(1) - P("'") - P("\\") - P("\n") -- any char with these exceptions







clex.keyword = lpeg.P({
    KEYWORD = P("alignas")       +
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
              P("_Noreturn",))
})








clex.constant = lpeg.P({
    "CONSTANT",


    CONSTANT = V("INTEGER_CONSTANT") +
               V("FLOATING_CONSTANT") +
               V("ENUMERATION_CONSTANT") +
               V("CHARACTER_CONSTANT") +
               V("PREDEFINED_CONSTANT"),


    -- optional integer suffix
    INTEGER_CONSTANT = V("DECIMAL_CONSTANT")     * V("INTEGER_SUFFIX")^-1 +
                     = V("OCTAL_CONSTANT")       * V("INTEGER_SUFFIX")^-1 +
                     = V("HEXADECIMAL_CONSTANT") * V("INTEGER_SUFFIX")^-1 +
                     = V("BINARY_CONSTANT")      * V("INTEGER_SUFFIX")^-1,


    -- decimal is leading nonzero followed by zero or more digits
    -- octal is leading zero followed by one or more digits, etc.
    DECIMAL_CONSTANT     = clex.nonzero_digit * clex.digit^0,
    OCTAL_CONSTANT       = P("0")             * clex.octal_digit^1,
    HEXADECIMAL_CONSTANT = clex.hex_prefix    * clex.hex_digit^1,
    BINARY_CONSTANT      = clex.binary_prefix * clex.binary_digit^1,


    FLOATING_CONSTANT = V("DECIMAL_FLOATING_CONSTANT") +
                        V("HEXADECIMAL_FLOATING_CONSTANT"),


    DECIMAL_FLOATING_CONSTANT = V("FRACTIONAL_CONSTANT") * V("EXPONENT_PART")^-1 * V("FLOATING_SUFFIX")^-1 +
                                clex.digit^1 * V("EXPONENT_PART") * V("FLOATING_SUFFIX")^-1


    HEXADECIMAL_FLOATING_CONSTANT = clex.hex_prefix * 
                                        (V"HEXADECIMAL_FRACTIONAL_CONSTANT" + clex.hex_digit^1) * 
                                            V"BINARY_EXPONENT_PART" * V"FLOATING_SUFFIX"^-1,


    -- set this later
    --ENUMERATION_CONSTANT = V("IDENTIFIER"),


    CHARACTER_CONSTANT = clex.encoding_prefix^-1 * P("'") * clex.c_char^1 * P("'"),


    PREDEFINED_CONSTANT = P("false") + P("true") + P("nullptr"),


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









clex.grammar = lpeg.P({
    "TOKEN",    -- initial rule name


    TOKEN = clex.keywords +
            V("IDENTIFIER") +
            V("CONSTANT") +
            V("STRING_LITERAL") +
            V("PUNCTUATOR"),

    
    PREPROCESSING_TOKEN = "", -- define later to handle all of them and ignore them


    -- leading nondigit, followed by 0 or more alphanumeric characters
    IDENTIFIER = clex.nondigit * clex.alphanumeric^0,





}) * P(-1) -- matches with the end of the document (read lpeg docs)



-- I am unsure if when constant is referenced a copy is created, or if it is an actual reference
-- if just a reference, then this should work.
clex.constant.ENUMERATION_CONSTANT = clex.grammar.IDENTIFIER







return clex;
