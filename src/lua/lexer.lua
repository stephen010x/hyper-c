
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


-- // start at:
-- // https://port70.net/~nsz/c/c23/n3220.html#A.1



-- DON'T HANDLE THE PREPROCESSOR, EXCEPT FOR NECCESSARY ELEMENTS. 
-- LIKE LINE NUMBER, ETC. OTHERWISE JUST IGNORE AND MIRROR ALL PREPROCESSOR TOKENS



