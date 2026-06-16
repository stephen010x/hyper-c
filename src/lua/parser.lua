-- /* ########################
--  * ##  !! IMPORTANT !!   ##
--  * ########################
--  *
--  * This document is based almost entierly on these specs and
--  * matching rules:
--  * 
--  * https://port70.net/~nsz/c/c23/n3220.html#A.1
--  * https://www.inf.puc-rio.br/~roberto/lpeg/
--  *
--  * ######################## */


local lpeg = require "lpeg"
local clex = require "lexer"

local P, S, R, V, B   =  lpeg.P, lpeg.S, lpeg.R, lpeg.V, lpeg.B
local C, Cb, Cc, Cf   =  lpeg.C, lpeg.Cb, lpeg.Cc, lpeg.Cf
local Cg, Cp, Cs, Ct  =  lpeg.Cg, lpeg.Cp, lpeg.Cs, lpeg.Ct
local Carg, Cmt       =  lpeg.Carg, lpeg.Cmt


local cpar = {}


local keyword        = clex.match.keyword
local punctuator     = clex.match.punctuator
local constant       = clex.match.constant
local string_literal = clex.match.string_literal
local identifier     = clex.match.identifier




function token(name)
    if clex.is_keyword(name) then
        return keyword(name)
    elseif clex.is_punctuator(name) then
        return punctuator(name)
    end
    error(("unknown token \"%s\""):format(name))
end

local t = token

-- function tokenCapture(name)
--     return Cp() * C(token(name))
-- end
-- 
-- local tC = tokenCapture


function tokenCapture(name)
    return Cp() * token(name) / function(pos) return (pos+1)/2 end
end

local tC = tokenCapture



function captNode(type, ...)
    return {
        ["type"] = type,
        ...
    }
end

function captNodeMode(type, mode, ...)
    return {
        ["type"] = type,
        ["mode"] = mode,
        ...
    }
end





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


--[[    I think for captures, it should generally be like this:
        group -> {
            [type] = string
            [mode] = number
            [1] = group or token index
            [2] = group or token index
            [3] = group or token index
            ...
        }

        So basically we have our group type and mode, which determines the format of the latter
        indexed items. The items are (probably)  stored in the order they occur. Though I can probably use
        numbered captures to reduce modes to simplify this.
        Something to note is that I should try to ALWAYS stick to a format, and not have optional formats.
        I can potentaily do this with group captures to capture optionals or lists as a single group.
        Also, I should try to keep captures to the beginning of each rule only, rather than interleaved.
        It should be managable. But ultimately, [1], [2], [3], etc should be predicatable. And if it is 
        a list or optional, then capture it in it's own subgroup.

        If mode is a negative number, then it is invalid.
        If mode is nil, then modes are arbitrary, and just check the subgroup type

        WARNING: A rule should never return more than one capture! It must always return either a token
        index, or a group. Also, I should probably make an effort to capture every token.
]]

cpar.grammar = lpeg.P({
    "TRANSLATION_UNIT",

    -- ############################################
    -- ####                                    ####
    -- ####             EXPRESSION             ####
    -- ####                                    ####
    -- ############################################

    PRIMARY_EXPRESSION = Cc"PRIMARY_EXPRESSION" * (
        Cc(1) * identifier +
        Cc(2) * constant +
        Cc(3) * string_literal +
        Cc(4) * tC"(" * V"EXPRESSION" * tC")"
        -- V"GENERIC_SELECTION",    -- don't implement for now
        ) / captNodeMode,

--     GENERIC_SELECTION =
--         tC"_Generic" * tC"(" * V"ASSIGNMENT_EXPRESSION" * tC"," * V"GENERIC_ASSOCIATION"^1 * tC")",
-- 
--     GENERIC_ASSOCIATION =

    -- POSTFIX_EXPRESSION = (
    --         V"PRIMARY_EXPRESSION" + 
    --         V"COMPOUND_LITERAL"
    --     ) * (
    --         tC"[" * V"EXPRESSION" * tC"]" +
    --         tC"(" * V"ARGUMENT_EXPRESSION_LIST"^-1 * tC")" +
    --         tC"." * identifier +
    --         tC"->" * identifier +
    --         tC"++" +
    --         tC"--" +
    --     )^1

    POSTFIX_EXPRESSION = Cc"POSTFIX_EXPRESSION" * (
        V"COMPOUND_LITERAL" +
        V"PRIMARY_EXPRESSION"
        ) * (
        tC"[" * V"EXPRESSION" * tC"]" +
        tC"(" * V"ARGUMENT_EXPRESSION_LIST"^-1 * tC")" +
        tC"." * identifier +
        tC"->" * identifier +
        tC"++" +
        tC"--"
        )^1 / captNode +
        V"COMPOUND_LITERAL" +
        V"PRIMARY_EXPRESSION",

    ARGUMENT_EXPRESSION_LIST = Cc"ARGUMENT_EXPRESSION_LIST" * ( 
        ( V"ASSIGNMENT_EXPRESSION" * tC"," )^0 * V"ASSIGNMENT_EXPRESSION"
        ) / captNode,

    COMPOUND_LITERAL = Cc"COMPOUND_LITERAL" * ( 
        tC"(" * V"STORAGE_CLASS_SPECIFIER"^0 * V"TYPE_NAME" * tC")" * V"BRACED_INITIALIZER"
        ) / captNode,

    UNARY_EXPRESSION = Cc"UNARY_EXPRESSION" * (
        Cc(1) * tC"++" * V"UNARY_EXPRESSION" +
        Cc(2) * tC"--" * V"UNARY_EXPRESSION" +
        Cc(3) * tC"sizeof" * V"UNARY_EXPRESSION" +
        Cc(4) * tC"sizeof" * tC"(" * V"TYPE_NAME" * tC")" +
        Cc(5) * tC"alignof" * tC"(" * V"TYPE_NAME" * tC")" +
        Cc(6) * V"UNARY_OPERATOR" * V"CAST_EXPRESSION"
        ) / captNodeMode +
        V"POSTFIX_EXPRESSION",

    UNARY_OPERATOR =
        tC"&" +
        tC"*" +
        tC"+" +
        tC"-" +
        tC"~" +
        tC"!",

    CAST_EXPRESSION = Cc"CAST_EXPRESSION" * (
        tC"(" * V"TYPE_NAME" * tC")" * V"CAST_EXPRESSION"
        ) / captNode +
        V"UNARY_EXPRESSION",

    MULTIPLICATIVE_EXPRESSION = Cc"MULTIPLICATIVE_EXPRESSION" * (
        V"CAST_EXPRESSION" * (
        tC"*" * V"CAST_EXPRESSION" +
        tC"/" * V"CAST_EXPRESSION" +
        tC"%" * V"CAST_EXPRESSION"
        )^1 ) / captNode +
        V"CAST_EXPRESSION",

    ADDITIVE_EXPRESSION = Cc"ADDITIVE_EXPRESSION" * (
        V"MULTIPLICATIVE_EXPRESSION" * (
        tC"+" * V"MULTIPLICATIVE_EXPRESSION" +
        tC"-" * V"MULTIPLICATIVE_EXPRESSION"
        )^1 ) / captNode +
        V"MULTIPLICATIVE_EXPRESSION",

    SHIFT_EXPRESSION = Cc"SHIFT_EXPRESSION" * (
        V"ADDITIVE_EXPRESSION" * (
        tC"<<" * V"ADDITIVE_EXPRESSION" +
        tC">>" * V"ADDITIVE_EXPRESSION"
        )^1 ) / captNode +
        V"ADDITIVE_EXPRESSION",

    RELATIONAL_EXPRESSION = Cc"RELATIONAL_EXPRESSION" * (
        V"SHIFT_EXPRESSION" * (
        tC"<" * V"SHIFT_EXPRESSION" +
        tC">" * V"SHIFT_EXPRESSION" +
        tC"<=" * V"SHIFT_EXPRESSION" +
        tC">=" * V"SHIFT_EXPRESSION"
        )^1 ) / captNode +
        V"SHIFT_EXPRESSION",

    EQUALITY_EXPRESSION = Cc"EQUALITY_EXPRESSION" * (
        V"RELATIONAL_EXPRESSION" * (
        tC"==" * V"RELATIONAL_EXPRESSION" +
        tC"!=" * V"RELATIONAL_EXPRESSION"
        )^1 ) / captNode +
        V"RELATIONAL_EXPRESSION",

    AND_EXPRESSION = Cc"AND_EXPRESSION" * (
        V"EQUALITY_EXPRESSION" * ( tC"&" * V"EQUALITY_EXPRESSION" )^1
        ) / captNode +
        V"EQUALITY_EXPRESSION",

    EXCLUSIVE_OR_EXPRESSION = Cc"EXCLUSIVE_OR_EXPRESSION" * (
        V"AND_EXPRESSION" * ( tC"^" * V"AND_EXPRESSION" )^1
        ) / captNode +
        V"AND_EXPRESSION",

    INCLUSIVE_OR_EXPRESSION = Cc"INCLUSIVE_OR_EXPRESSION" * (
        V"EXCLUSIVE_OR_EXPRESSION" * ( tC"|" * V"EXCLUSIVE_OR_EXPRESSION" )^1
        ) / captNode +
        V"EXCLUSIVE_OR_EXPRESSION",

    LOGICAL_AND_EXPRESSION = Cc"LOGICAL_AND_EXPRESSION" * (
        V"INCLUSIVE_OR_EXPRESSION" * ( tC"&&" * V"INCLUSIVE_OR_EXPRESSION" )^1
        ) / captNode +
        V"INCLUSIVE_OR_EXPRESSION",

    LOGICAL_OR_EXPRESSION = Cc"LOGICAL_OR_EXPRESSION" * (
        V"LOGICAL_AND_EXPRESSION" * ( tC"||" * V"LOGICAL_AND_EXPRESSION" )^1
        ) / captNode +
        V"LOGICAL_AND_EXPRESSION",

    CONDITIONAL_EXPRESSION = Cc"CONDITIONAL_EXPRESSION" * (
        V"LOGICAL_OR_EXPRESSION" * tC"?" * V"EXPRESSION" * tC":" * V"CONDITIONAL_EXPRESSION"
        ) / captNode +
        V"LOGICAL_OR_EXPRESSION",

    ASSIGNMENT_EXPRESSION = Cc"ASSIGNMENT_EXPRESSION" * (
        V"UNARY_EXPRESSION" * V"ASSIGNMENT_OPERATOR" * V"ASSIGNMENT_EXPRESSION"
        ) / captNode +
        V"CONDITIONAL_EXPRESSION",

    ASSIGNMENT_OPERATOR = Cp() * (
            tC"=" +
            tC"*=" +
            tC"/=" +
            tC"%=" +
            tC"+=" +
            tC"-=" +
            tC"&=" +
            tC"^=" +
            tC"|=" +
            tC"<<="+
            tC">>="
        ),

    EXPRESSION = Cc"EXPRESSION" * (
        ( V"ASSIGNMENT_EXPRESSION" * tC"," )^0 * V"ASSIGNMENT_EXPRESSION"
        ) / captNode,

    CONSTANT_EXPRESSION =
        V"CONDITIONAL_EXPRESSION",
    



    

    -- ############################################
    -- ####                                    ####
    -- ####             DECLARATION            ####
    -- ####                                    ####
    -- ############################################

    DECLARATION = Cc"DECLARATION" * (
        Cc(1) * V"DECLARATION_SPECIFIERS" * V"INIT_DECLARATOR_LIST"^-1 * tC";" +
        Cc(2) * V"ATTRIBUTE_SPECIFIER"^1 * V"DECLARATION_SPECIFIERS" * V"INIT_DECLARATOR_LIST" * tC";" +
        Cc(3) * V"STATIC_ASSERT_DECLARATION" +
        Cc(4) * V"ATTRIBUTE_DECLARATION"
        ) / captNodeMode,

    -- DECLARATION_SPECIFIERS = 
    --     V"DECLARATION_SPECIFIER" * V"DECLARATION_SPECIFIERS" +
    --     V"DECLARATION_SPECIFIER" * V"ATTRIBUTE_SPECIFIER"^0,

    DECLARATION_SPECIFIERS = Cc"DECLARATION_SPECIFIERS" * (
        V"DECLARATION_SPECIFIER"^1 * V"ATTRIBUTE_SPECIFIER"^0
        ) / captNode,
        

    DECLARATION_SPECIFIER =
        V"STORAGE_CLASS_SPECIFIER" +
        V"TYPE_SPECIFIER_QUALIFIER" +
        V"FUNCTION_SPECIFIER",

    INIT_DECLARATOR_LIST = Cc"INIT_DECLARATOR_LIST" * (
        ( V"INIT_DECLARATOR" * tC"," )^0 + V"INIT_DECLARATOR"
        ) / captNode,

    -- INIT_DECLARATOR = 
    --     V"DECLARATOR" +
    --     V"DECLARATOR" * tC"=" * V"INITIALIZER", 

    INIT_DECLARATOR = Cc"INIT_DECLARATOR" * (
        V"DECLARATOR" * ( tC"=" * V"INITIALIZER" )^-1
        ) / captNode,

    -- TODO: implement later
    -- ATTRIBUTE_DECLARATION = Cc"ATTRIBUTE_DECLARATION" * (
    --     V"ATTRIBUTE_SPECIFIER"^1 * tC";",
    --     ) / captNode,

    ATTRIBUTE_DECLARATION = P(false),

    STORAGE_CLASS_SPECIFIER =
        tC"auto" +
        tC"constexpr" +
        tC"extern" +
        tC"register" +
        tC"static" +
        tC"thread_local" +
        tC"typedef",

    TYPE_SPECIFIER = Cc"TYPE_SPECIFIER" * (
        tC"void" +
        tC"char" +
        tC"short" +
        tC"int" +
        tC"long" +
        tC"float" +
        tC"double" +
        tC"signed" +
        tC"unsigned" +
        tC"_BitInt" * tC"(" * V"CONSTANT_EXPRESSION" * tC")" +
        tC"bool" +
        tC"_Complex" +
        tC"_Decimal32" +
        tC"_Decimal64" +
        tC"_Decimal128" +
        V"ATOMIC_TYPE_SPECIFIER" +
        V"STRUCT_OR_UNION_SPECIFIER" +
        V"ENUM_SPECIFIER" +
        V"TYPEDEF_NAME" +
        V"TYPEOF_SPECIFIER"
        ) / captNode,

    STRUCT_OR_UNION_SPECIFIER = Cc"STRUCT_OR_UNION_SPECIFIER" * (
        Cc(1) * V"STRUCT_OR_UNION" * V"ATTRIBUTE_SPECIFIER"^0 * identifier^-1 *
                tC"{" * V"MEMBER_DECLARATION"^1 * tC"}" +
        Cc(2) * V"STRUCT_OR_UNION" * V"ATTRIBUTE_SPECIFIER"^0 * identifier
        ) / captNodeMode,

    STRUCT_OR_UNION =
        tC"struct" +
        tC"union",

    MEMBER_DECLARATION = Cc"MEMBER_DECLARATION" * (
        Cc(1) * V"ATTRIBUTE_SPECIFIER"^0 * V"SPECIFIER_QUALIFIER_LIST" * V"MEMBER_DECLARATOR_LIST"^-1 * tC";" +
        Cc(2) * V"STATIC_ASSERT_DECLARATION"
        ) / captNodeMode,

    -- SPECIFIER_QUALIFIER_LIST =
    --     V"TYPE_SPECIFIER_QUALIFIER" * V"SPECIFIER_QUALIFIER_LIST" +
    --     V"TYPE_SPECIFIER_QUALIFIER" * V"ATTRIBUTE_SPECIFIER"^0

    SPECIFIER_QUALIFIER_LIST = Cc"SPECIFIER_QUALIFIER_LIST" * (
        V"TYPE_SPECIFIER_QUALIFIER"^1 * V"ATTRIBUTE_SPECIFIER"^0
        ) / captNode,

    TYPE_SPECIFIER_QUALIFIER =
        V"TYPE_SPECIFIER" + 
        V"TYPE_QUALIFIER" + 
        V"ALIGNMENT_SPECIFIER", 

    MEMBER_DECLARATOR_LIST = Cc"MEMBER_DECLARATOR_LIST" * (
        ( V"MEMBER_DECLARATOR" * tC"," )^0 * V"MEMBER_DECLARATOR"
        ) / captNode,

    MEMBER_DECLARATOR = Cc"MEMBER_DECLARATOR" * (
        Cc(1) * V"DECLARATOR"^-1 * tC":" * V"CONSTANT_EXPRESSION" +
        Cc(2) * V"DECLARATOR"
        ) / captNodeMode,

    ENUM_SPECIFIER = Cc"ENUM_SPECIFIER" * (
        Cc(1) * tC"enum" * V"ATTRIBUTE_SPECIFIER"^0 * identifier^-1 * V"ENUM_TYPE_SPECIFIER"^-1 *
                tC"{" * V"ENUMERATOR_LIST" * tC","^-1 * tC"}" +
        Cc(2) * tC"enum" * identifier * V"ENUM_TYPE_SPECIFIER"^-1
        ) / captNodeMode,

    ENUMERATOR_LIST = Cc"ENUMERATOR_LIST" * (
        ( V"ENUMERATOR" * tC"," )^0 * V"ENUMERATOR"
    ) / captNode,

    ENUMERATOR = Cc"ENUMERATOR" * (
        identifier * V"ATTRIBUTE_SPECIFIER"^0 * ( tC"=" * V"CONSTANT_EXPRESSION" )^-1
        ) / captNode,

    ENUM_TYPE_SPECIFIER = Cc"ENUM_TYPE_SPECIFIER" * (
        tC":" * V"SPECIFIER_QUALIFIER_LIST"
        ) / captNode,

    ATOMIC_TYPE_SPECIFIER = Cc"ATOMIC_TYPE_SPECIFIER" * (
        tC"_Atomic" * tC"(" * V"TYPE_NAME" * tC")"
        ) / captNode,

    TYPEOF_SPECIFIER = Cc"TYPEOF_SPECIFIER" * (
        tC"typeof" * tC"(" * V"TYPEOF_SPECIFIER_ARGUMENT" * tC")" + 
        tC"typeof_unqual" * tC"(" * V"TYPEOF_SPECIFIER_ARGUMENT" * tC")"
        ) / captNode,

    TYPEOF_SPECIFIER_ARGUMENT =
        V"EXPRESSION" +
        V"TYPE_NAME",

    TYPE_QUALIFIER =
        tC"const" +
        tC"restrict" +
        tC"volatile" +
        tC"_Atomic",

    FUNCTION_SPECIFIER =
        tC"inline" +
        tC"_Noreturn",

    -- ALIGNMENT_SPECIFIER =
    --     tC"alignas" * tC"(" * V"TYPE_NAME" * tC")" +
    --     tC"alignas" * tC"(" * V"CONSTANT_EXPRESSION" * tC")",

    ALIGNMENT_SPECIFIER = Cc"ALIGNMENT_SPECIFIER" * (
        tC"alignas" * tC"(" * (
                    V"TYPE_NAME" +
                    V"CONSTANT_EXPRESSION"
                ) * tC")"
        ) / captNode,

    DECLARATOR = Cc"DECLARATOR" * (
        V"POINTER"^-1 * V"DIRECT_DECLARATOR"
        ) / captNode,

    -- DIRECT_DECLARATOR = Cc"DIRECT_DECLARATOR" * (
    --     Cc(1) * identifier * V"ATTRIBUTE_SPECIFIER"^0 +
    --     Cc(2) * tC"(" * V"DECLARATOR" * tC")" +
    --     Cc(3) * V"FUNCTION_DECLARATOR" * V"ATTRIBUTE_SPECIFIER"^0
    --     ) / captNodeMode,

    DIRECT_DECLARATOR = Cc"DIRECT_DECLARATOR" * (
        (
        Cc(1) * identifier * V"ATTRIBUTE_SPECIFIER"^0 +
        Cc(2) * tC"(" * V"DECLARATOR" * tC")"
        ) * (
        V"ARRAY_DECLARATOR" * V"ATTRIBUTE_SPECIFIER"^0 +
        V"FUNCTION_DECLARATOR" * V"ATTRIBUTE_SPECIFIER"^0
        )^1
        ) / captNodeMode,

    -- ARRAY_DECLARATOR = Cc"ARRAY_DECLARATOR" * (
    --     V"DIRECT_DECLARATOR" * tC"[" * V"ARRAY_DECLARATOR_INNER" * tC"]"
    --     ) / captNode,

    ARRAY_DECLARATOR = Cc"ARRAY_DECLARATOR" * (
        tC"[" * V"ARRAY_DECLARATOR_INNER" * tC"]"
        ) / captNode,

    ARRAY_DECLARATOR_INNER = Cc"ARRAY_DECLARATOR_INNER" * (
        Cc(1) * V"TYPE_QUALIFIER"^0 * V"ASSIGNMENT_EXPRESSION"^-1 +
        Cc(2) * tC"static" * V"TYPE_QUALIFIER"^0 * V"ASSIGNMENT_EXPRESSION" +
        Cc(3) * V"TYPE_QUALIFIER"^1 * tC"static" * V"ASSIGNMENT_EXPRESSION" +
        Cc(4) * V"TYPE_QUALIFIER"^0 * tC"*"
        ) / captNodeMode,

    -- FUNCTION_DECLARATOR = Cc"FUNCTION_DECLARATOR" * (
    --     V"DIRECT_DECLARATOR" * tC"(" * V"PARAMETER_TYPE_LIST"^-1 * tC")"
    --     ) / captNode,

    FUNCTION_DECLARATOR = Cc"FUNCTION_DECLARATOR" * (
        tC"(" * V"PARAMETER_TYPE_LIST"^-1 * tC")"
        ) / captNode,

    POINTER = Cc"POINTER" * (
        tC"*" * V"ATTRIBUTE_SPECIFIER"^0 * V"TYPE_QUALIFIER"^0 * V"POINTER"^-1
        ) / captNode,

    PARAMETER_TYPE_LIST = Cc"PARAMETER_TYPE_LIST" * (
        Cc(1) * V"PARAMETER_LIST" * ( tC"," * tC"..." )^-1 +
        Cc(2) * tC"..."
        ) / captNodeMode,

    PARAMETER_LIST = Cc"PARAMETER_LIST" * (
        ( V"PARAMETER_DECLARATION" * tC"," )^0 * V"PARAMETER_DECLARATION"
        ) / captNode,

    PARAMETER_DECLARATION = Cc"PARAMETER_DECLARATION" * (
        V"ATTRIBUTE_SPECIFIER"^0 * V"DECLARATION_SPECIFIERS" * (
                    V"DECLARATOR" +
                    V"ABSTRACT_DECLARATOR"
                )^-1
        ) / captNode,

    TYPE_NAME = Cc"TYPE_NAME" * (
        V"SPECIFIER_QUALIFIER_LIST" * V"ABSTRACT_DECLARATOR"^-1
        ) / captNode,

    ABSTRACT_DECLARATOR = Cc"ABSTRACT_DECLARATOR" * (
        Cc(1) * V"POINTER"^-1 * V"DIRECT_ABSTRACT_DECLARATOR" +
        Cc(2) * V"POINTER"
        ) / captNodeMode,

    -- DIRECT_ABSTRACT_DECLARATOR = Cc"DIRECT_ABSTRACT_DECLARATOR" * (
    --     Cc(1) * tC"(" * V"ABSTRACT_DECLARATOR" * tC")" +
    --     Cc(2) * V"ARRAY_ABSTRACT_DECLARATOR" * V"ATTRIBUTE_SPECIFIER"^0 +
    --     Cc(3) * V"FUNCTION_ABSTRACT_DECLARATOR" * V"ATTRIBUTE_SPECIFIER"^0
    --     ) / captNodeMode,

    DIRECT_ABSTRACT_DECLARATOR = Cc"DIRECT_ABSTRACT_DECLARATOR" * (
        ( tC"(" * V"ABSTRACT_DECLARATOR" * tC")" )^-1 * (
            V"ARRAY_ABSTRACT_DECLARATOR" * V"ATTRIBUTE_SPECIFIER"^0 +
            V"FUNCTION_ABSTRACT_DECLARATOR" * V"ATTRIBUTE_SPECIFIER"^0
            )^1 ) / captNode,

    -- ARRAY_ABSTRACT_DECLARATOR = Cc"ARRAY_ABSTRACT_DECLARATOR" * (
    --     V"DIRECT_ABSTRACT_DECLARATOR"^-1 * tC"[" * V"ARRAY_ABSTRACT_DECLARATOR_INNER" * tC"]"
    --     ) / captNode,

    ARRAY_ABSTRACT_DECLARATOR = Cc"ARRAY_ABSTRACT_DECLARATOR" * (
        tC"[" * V"ARRAY_ABSTRACT_DECLARATOR_INNER" * tC"]"
        ) / captNode,

    ARRAY_ABSTRACT_DECLARATOR_INNER = Cc"ARRAY_ABSTRACT_DECLARATOR_INNER" * (
        Cc(1) * V"TYPE_QUALIFIER"^0 * V"ASSIGNMENT_EXPRESSION"^-1 +
        Cc(2) * tC"static" * V"TYPE_QUALIFIER"^0 * V"ASSIGNMENT_EXPRESSION" +
        Cc(3) * V"TYPE_QUALIFIER"^1 * tC"static" * V"ASSIGNMENT_EXPRESSION" +
        Cc(4) * tC"*"
        ) / captNodeMode,

    -- FUNCTION_ABSTRACT_DECLARATOR = Cc"FUNCTION_ABSTRACT_DECLARATOR" * (
    --     V"DIRECT_ABSTRACT_DECLARATOR"^-1 * tC"(" * V"PARAMETER_TYPE_LIST"^-1 * tC")"
    --     ) / captNode,

    FUNCTION_ABSTRACT_DECLARATOR = Cc"FUNCTION_ABSTRACT_DECLARATOR" * (
        tC"(" * V"PARAMETER_TYPE_LIST"^-1 * tC")"
        ) / captNode,

    TYPEDEF_NAME =
        identifier,

    BRACED_INITIALIZER = Cc"BRACED_INITIALIZER" * (
        tC"{" * ( V"INITIALIZER_LIST" * tC","^-1 )^-1 * tC"}"
        ) / captNode,

    INITIALIZER =
        V"ASSIGNMENT_EXPRESSION" +
        V"BRACED_INITIALIZER",

    INITIALIZER_LIST = Cc"INITIALIZER_LIST" * (
        ( V"DESIGNATION"^-1 * V"INITIALIZER" * tC"," )^0 * V"DESIGNATION"^-1 * V"INITIALIZER"
        ) / captNode,

    DESIGNATION = Cc"DESIGNATION" * (
        V"DESIGNATOR"^1
        ) / captNode,

    DESIGNATOR = Cc"DESIGNATOR" * (
        Cc(1) * tC"[" * V"CONSTANT_EXPRESSION" * tC"]" +
        Cc(2) * tC"." * identifier
        ) / captNodeMode,

    STATIC_ASSERT_DECLARATION = Cc"STATIC_ASSERT_DECLARATION" * (
        tC"static_assert" * tC"(" * V"CONSTANT_EXPRESSION" * ( tC"," * string_literal )^-1 * tC")" * tC";"
        ) / captNode,

    -- TODO: implement the old attribute syntax, not the new one.

    -- TODO: implement attributes later

    ATTRIBUTE_SPECIFIER = P(false),

--     ATTRIBUTE_SPECIFIER =
--         tC"[" * tC"[" * tC"]" * tC"]",
-- 
--     ATTRIBUTE_LIST =
-- 
--     ATTRIBUTE =
-- 
--     ATTRIBUTE_TOKEN =
-- 
--     STANDARD_ATTRIBUTE =
-- 
--     ATTRIBUTE_PREFIXED_TOKEN =
-- 
--     ATTRIBUTE_PREFIX =
-- 
--     ATTRIBUTE_ARGUMENT_CLAUSE =
-- 
--     BALANCED_TOKEN_SEQUENCE =
-- 
--     BALANCED_TOKEN =



    

    -- ############################################
    -- ####                                    ####
    -- ####             STATEMENTS             ####
    -- ####                                    ####
    -- ############################################

    STATEMENT =
        V"LABELED_STATEMENT" +
        V"UNLABELED_STATEMENT",

    UNLABELED_STATEMENT = Cc"UNLABELED_STATEMENT" * (
        V"EXPRESSION_STATEMENT" +
        V"ATTRIBUTE_SPECIFIER"^0 * V"PRIMARY_BLOCK" +
        V"ATTRIBUTE_SPECIFIER"^0 * V"JUMP_STATEMENT"
        ) / captNode,

    PRIMARY_BLOCK = Cc"PRIMARY_BLOCK" * (
        V"COMPOUND_STATEMENT" +
        V"SELECTION_STATEMENT" +
        V"ITERATION_STATEMENT"
        ) / captNode,

    SECONDARY_BLOCK =
        V"STATEMENT",

    LABEL = Cc"LABEL" * (
        Cc(1) * V"ATTRIBUTE_SPECIFIER"^0 * identifier * tC":" +
        Cc(2) * V"ATTRIBUTE_SPECIFIER"^0 * tC"case" * V"CONSTANT_EXPRESSION" * tC":" +
        Cc(3) * V"ATTRIBUTE_SPECIFIER"^0 * tC"default" * tC":"
        ) / captNodeMode,

    LABELED_STATEMENT = Cc"LABELED_STATEMENT" * (
        V"LABEL" * V"STATEMENT"
        ) / captNode,

    COMPOUND_STATEMENT = Cc"COMPOUND_STATEMENT" * (
        tC"{" * V"BLOCK_ITEM"^0 * tC"}"
        ) / captNode,

    BLOCK_ITEM =
        V"DECLARATION" +
        V"UNLABELED_STATEMENT" +
        V"LABEL",

    EXPRESSION_STATEMENT = Cc"EXPRESSION_STATEMENT" * (
        Cc(1) * V"EXPRESSION"^-1 * tC";" +
        Cc(2) * V"ATTRIBUTE_SPECIFIER"^1 * V"EXPRESSION" * tC";"
        ) / captNodeMode,

    SELECTION_STATEMENT = Cc"SELECTION_STATEMENT" * (
        Cc(1) * tC"if" * tC"(" * V"EXPRESSION" * tC")" * V"SECONDARY_BLOCK" +
        Cc(2) * tC"if" * tC"(" * V"EXPRESSION" * tC")" * V"SECONDARY_BLOCK" * tC"else" * V"SECONDARY_BLOCK" +
        Cc(3) * tC"switch" * tC"(" * V"EXPRESSION" * tC")" * V"SECONDARY_BLOCK"
        ) / captNodeMode,

    ITERATION_STATEMENT = Cc"ITERATION_STATEMENT" * (
        Cc(1) * tC"while" * tC"(" + V"EXPRESSION" + tC")" * V"SECONDARY_BLOCK" +
        Cc(2) * tC"do" * V"SECONDARY_BLOCK" * tC"while" * tC"(" * V"EXPRESSION" * tC")" * tC";" +
        Cc(3) * tC"for" * tC"(" * V"EXPRESSION"^-1 * tC";" * V"EXPRESSION"^-1 * tC";" *
                V"EXPRESSION"^-1 * tC")" * V"SECONDARY_BLOCK" +
        Cc(4) * tC"for" * tC"(" * V"DECLARATION" * V"EXPRESSION"^-1 * tC";" *
                V"EXPRESSION"^-1 * tC")" * V"SECONDARY_BLOCK"
        ) / captNodeMode,

    JUMP_STATEMENT = Cc"JUMP_STATEMENT" * (
        Cc(1) * tC"goto" * identifier * tC";" +
        Cc(2) * tC"continue" * tC";" +
        Cc(3) * tC"break" * tC";" +
        Cc(4) * tC"return" * V"EXPRESSION"^-1 * tC";"
        ) / captNodeMode,



    

    -- ############################################
    -- ####                                    ####
    -- ####        EXTERNAL DEFINITIONS        ####
    -- ####                                    ####
    -- ############################################
    
    -- P(-1) matches with the end of the document (read lpeg docs)
    TRANSLATION_UNIT = Cc"TRANSLATION_UNIT" * (
        V"EXTERNAL_DECLARATION"^0 * P(-1)
        ) / captNode,

    EXTERNAL_DECLARATION = Cc"EXTERNAL_DECLARATION" * (
        Cc(1) * V"FUNCTION_DEFINITION" +
        Cc(2) * V"DECLARATION" +
        Cc(-1) * (Cp() * P(2) / function(pos) return (pos+1)/2 end)
        ) / captNodeMode,

    FUNCTION_DEFINITION = Cc"FUNCTION_DEFINITION" * (
        V"ATTRIBUTE_SPECIFIER"^0 * V"DECLARATION_SPECIFIERS" * V"DECLARATOR" *
        V"FUNCTION_BODY"
        ) / captNode,

    FUNCTION_BODY =
        V"COMPOUND_STATEMENT",
    
})


return cpar
