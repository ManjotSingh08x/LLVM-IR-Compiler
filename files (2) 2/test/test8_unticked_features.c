/* test8_unticked_features.c
 *
 * NEGATIVE TEST for the features left UNTICKED on the project proposal form.
 *
 * test7_all_features.c proves the lexer accepts everything we promised.
 * This file proves the opposite half: that the lexer REJECTS what we did
 * not promise, with an accurate message, instead of silently mis-lexing it
 * and leaving the parser to fail confusingly later.
 *
 * Running the lexer over this file must report EVERY line marked ERROR
 * below, and nothing else. Exit status must be 1.
 *
 * --------------------------------------------------------------------------
 * WHAT A LEXER CAN AND CANNOT REJECT
 *
 * A lexer sees tokens, not grammar. So it can only reject an unticked
 * feature when that feature has a distinctive SPELLING. It splits into
 * three groups:
 *
 *   GROUP A — reserved words. "typedef", "enum" and friends are single
 *             words, so the lexer recognizes them and reports them. See
 *             unsupportedTable() in token_types.h.
 *
 *   GROUP B — distinctive operator spellings. "..." and the compound
 *             bitwise assignments are unique character sequences, so the
 *             lexer recognizes those too.
 *
 *   GROUP C — everything else. Multi-level pointers are the clear case:
 *             "int **p" is two '*' tokens, and so is the perfectly legal
 *             "a * *p". They are identical at the token level, so no lexer
 *             can tell them apart. File manipulation is the same story —
 *             fopen and fclose are ordinary identifiers, indistinguishable
 *             from any other function name until the symbol table exists.
 *             Group C belongs to Assignment 2 and Assignment 3.
 *
 * The bottom half of this file documents Group C explicitly, so that when
 * the parser is written nobody assumes the lexer already handled it.
 * -------------------------------------------------------------------------- */


/* =========================================================================
 * GROUP A — reserved words for unticked features
 * Form item: "static keywords"
 * ====================================================================== */

int useStatic() {
    static int counter = 0;        /* ERROR: static is reserved, unsupported */
    return counter;
}


/* Form item: "typedef" */
typedef int Integer;               /* ERROR: typedef is reserved, unsupported */


/* Form item: "enum, union" */
enum Colour {                      /* ERROR: enum is reserved, unsupported */
    RED,
    GREEN
};

union Value {                      /* ERROR: union is reserved, unsupported */
    int  asInt;
    char asChar;
};


/* Form item: "until loop" */
int useUntil(int n) {
    until (n > 10) {               /* ERROR: until is reserved, unsupported */
        n++;
    }
    return n;
}


/* sizeof is not on the form at all, so it is unsupported too */
int useSizeof() {
    int v = 0;
    return sizeof(v);              /* ERROR: sizeof is reserved, unsupported */
}


/* =========================================================================
 * GROUP B — operator spellings for unticked features
 * Form item: "Function call with variable arguments"
 * ====================================================================== */

int sumAll(int first, ...);        /* ERROR: ... varargs unsupported */


/* Compound bitwise assignment is not on the form and not in the language.
 * These are the dangerous ones: without a rule for them the lexer would
 * happily split "a <<= 2" into "a" "<<" "=" "2" and report nothing at all. */
int useCompoundBitwise() {
    int a = 8;
    a <<= 2;                       /* ERROR: <<= unsupported */
    a >>= 1;                       /* ERROR: >>= unsupported */
    a  &= 3;                       /* ERROR: &= unsupported */
    a  |= 4;                       /* ERROR: |= unsupported */
    a  ^= 5;                       /* ERROR: ^= unsupported */
    return a;
}


/* =========================================================================
 * GROUP C — unticked features NO lexer can detect
 *
 * Everything below is commented out on purpose. It is here as a record of
 * what Assignment 2 and Assignment 3 must reject, not as something this
 * file expects an error for. Uncommenting any of it would produce ZERO
 * lexical errors, which is the whole point.
 * ====================================================================== */

/* Form item: "Multi-level pointers"
 *
 *     int **pp = &p;
 *
 * lexes as: int, **, pp ... except there is no "**" token. It lexes as two
 * separate '*' operators, exactly like the legal expression "a * *p". The
 * lexer cannot distinguish them. The PARSER must reject a declarator with
 * two '*' in a row.
 */

/* Form item: "File manipulation"
 *
 *     FILE *f = fopen("data.txt", "r");
 *     fclose(f);
 *
 * lexes cleanly: FILE, fopen and fclose are just identifiers. Nothing marks
 * them as special until a symbol table exists. SEMANTIC ANALYSIS must
 * reject them as undeclared, because the language declares no file API.
 */

/* Form item: "Function call with variable arguments", the call side
 *
 *     sumAll(1, 2, 3, 4);
 *
 * is an ordinary call with four arguments as far as the lexer is concerned.
 * Only the DECLARATION contains "...", which is why Group B above catches
 * the declaration and not the call. SEMANTIC ANALYSIS must check the
 * argument count against the declared parameter count.
 */
