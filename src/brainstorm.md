

Alright, so the lexer will output a bunch of tokens, and it will also output a token string for parsing. The index into the token string will be the index into the real token list, but for parsing all we really need is special short descriptors of the token itself. That is, the token type and some token id. So each token flag will just be two bytes.

This will of course all be recursive descent with some lookahead.






