local lpeg = require "lpeg"
local clex = require "lexer"

local P, S, R, V, B   =  lpeg.P, lpeg.S, lpeg.R, lpeg.V, lpeg.B
local C, Cb, Cc, Cf   =  lpeg.C, lpeg.Cb, lpeg.Cc, lpeg.Cf
local Cg, Cp, Cs, Ct  =  lpeg.Cg, lpeg.Cp, lpeg.Cs, lpeg.Ct
local Carg, Cmt       =  lpeg.Cart, lpeg.Cmt


local cpar = {}


local keyword        = clex.match.keyword
local punctuator     = clex.match.punctuator
local constant       = clex.match.constant
local string_literal = clex.match.string_literal
local identifier     = clex.match.identifier






-- TODO: for error detection, I think I basically need to make sure that there is a match for everything
--       by adding invalid matches when all other matches fail, etc. basically add redundancy.
--       kind of like your unknown token matcher in the parser

--[[    It sounds like lua strings and lpeg are binary safe. So theoretically I can store 
        raw indices into them. And potentailly also hook into a capture with a c function
        to process them.

        Alternatively, what about a full pattern writtin in C for token matching?
        That does feel redundant, as in the end we still rely on the constructed pattern.

        I need a global c-lua function that returns a c-lua function.

        So like clex.punctuator("<<=") or clex.keyword.inline return grammar that have a C
        matching function in there, via lpeg.P(function)
]]

cpar.grammar = lpeg.P({
    "TRANSLATION_UNIT",

    -- ############################################
    -- ####                                    ####
    -- ####             EXPRESSION             ####
    -- ####                                    ####
    -- ############################################

    -- ############################################
    -- ####                                    ####
    -- ####             DECLARATION            ####
    -- ####                                    ####
    -- ############################################

    -- ############################################
    -- ####                                    ####
    -- ####             STATEMENTS             ####
    -- ####                                    ####
    -- ############################################

    STATEMENT = V"LABELED_STATEMENT" +
                V"UNLABELED_STATEMENT",

    UNLABELED_STATEMENT = V"EXPRESSION_STATEMENT" +
                          V"ATTRIBUTE_SPECIFIER"^0 + V"PRIMARY_BLOCK" +
                          V"ATTRIBUTE_SPECIFIER"^0 + V"JUMP_STATEMENT",

    PRIMARY_BLOCK = V"COMPOUND_STATEMENT" +
                    V"SELECTION_STATEMENT" +
                    V"ITERATION_STATEMENT",

    SECONDARY_BLOCK = V"STATEMENT",

    LABEL



    

    -- ############################################
    -- ####                                    ####
    -- ####        EXTERNAL DEFINITIONS        ####
    -- ####                                    ####
    -- ############################################
    
    -- P(-1) matches with the end of the document (read lpeg docs)
    TRANSLATION_UNIT = Ct(V"EXTERNAL_DECLARATION"^0) * P(-1),

    EXTERNAL_DECLARAION = V"FUNCTION_DEFINITION" +
                          V"DECLARATION",

    FUNCTION_DEFINITION = V"ATTRIBUTE_SPECIFIER"^0 + V"DECLARATION_SPECIFIERS" + V"DECLARATOR" + V"FUNCTION_BODY",

    FUNCTION_BODY = V"COMPOUND_STATEMENT",
    
})


return cpar
