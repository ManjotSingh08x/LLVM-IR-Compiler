// test6_lexical_errors.c
// Every class of lexical error the analyzer detects. Running this file
// must report ALL of them, not just the first.

int main() {
    int x = 5 @ 3;                  // illegal character
    int y = 4 $ 2;                  // illegal character
    int z = 6 # 1;                  // illegal character
    int w = 7 ` 1;                  // illegal character

    int bad1 = 123abc;              // malformed numeric literal
    int bad2 = 1.2.3;               // malformed numeric literal
    int bad3 = 0x;                  // malformed numeric literal
    int bad4 = 1e;                  // malformed numeric literal
    int small_test=000000000000000000000000000;
    int big  = 999999999999999999999999;  // integer literal out of range

    char e1 = '';                   // empty character literal
    char e2 = 'ab';                 // multi-character literal
    char e3 = 'q;                   // unterminated character literal
    char e4 = '\q';                 // invalid escape sequence

    char *s1 = "unterminated here
    char *s2 = "bad escape \q inside";

    /* this block comment is never closed
    return 0;
}
