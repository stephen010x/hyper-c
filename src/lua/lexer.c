#if 0

#include "luabc.h"


// These are the actual matching functions, not the constructors. So they need to be fast.

// So the pos can allow me to index into the table itself, which means the subject data
// should be used to detect false matches as quickly as possible. This means encoding the type
// and extra.
// The actual encoders can be in lua.

// match [, captures...] = keyword(subject, pos [, captures...])
int lhook_keyword(lua_State *L) {

}




int lhook_punctuator(lua_State *L) {

}




int lhook_iskeyword(lua_State *L) {

}




int lhook_ispunctuator(lua_State *L) {

}




int lhook_isidentifier(lua_State *L) {

}




int lhook_isconstant(lua_State *L) {

}




int lhook_isstring_literal(lua_State *L) {

}




int lhook_encode_tokens(lua_State *L) {

}

#endif
