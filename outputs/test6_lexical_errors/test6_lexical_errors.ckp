int main() {
    // Tests: illegal characters
    int x = 5 @ 3;
    int y = 4 $ 2;
    int z = 6 # 1;
    int w = 7 ` 1;

    // Tests: malformed numeric literals
    int bad1 = 123abc;
    int bad2 = 1.2.3;
    int bad3 = 0x;
    int bad4 = 1e;
    int small_test=000000000000000000000000000;

    // Tests: integer literal out of range
    int big  = 999999999999999999999999;

    // Tests: invalid character literals
    char e1 = '';
    char e2 = 'ab';
    char e3 = 'q;
    char e4 = '\q';

    // Tests: unterminated string and invalid string escape
    char *s1 = "unterminated here
    char *s2 = "bad escape \q inside";

    // Tests: unterminated block comment
    /* this block comment is never closed
    return 0;
}
