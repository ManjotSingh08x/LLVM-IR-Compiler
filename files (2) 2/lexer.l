%{
/*
 * lexer.l — Lexical Analyzer, Assignment 1, CSC-305
 *
 * Design note (read before extending):
 *   This file ONLY recognizes lexemes and classifies them via token_types.h.
 *   It does not know about grammar, so it stays reusable as-is once a bison
 *   parser is added in Assignment 2 — at that point:
 *     1. change addToken(TOK_X); to addToken(TOK_X); return TOK_X;
 *     2. delete main()/printTable() and let bison's yyparse() drive yylex()
 *     3. nothing in the rules section below needs to change
 *   Keep it that way: don't hardcode assignment-1-only behavior into the
 *   rules, keep it in main()/printTable() instead.
 *
 * Two things every reader should know about how flex picks a rule:
 *   LONGEST MATCH  — of all rules that match at the current position, flex
 *                    takes the one matching the MOST characters.
 *   FIRST LISTED   — if two rules match the SAME number of characters, the
 *                    one written earlier in this file wins.
 *   Several rules below depend on that ordering. They are marked ORDER.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "token_types.h"

struct TokenRecord {
    std::string lexeme;
    int         id;
    int         line;
};

struct LexError {
    std::string message;
    int         line;
    int         col;
};

static std::vector<TokenRecord> g_tokens;
static std::vector<LexError>    g_errors;

// Column tracking. yylineno gives us the line for free; flex has no built-in
// column counter, so we advance one ourselves in YY_USER_ACTION below.
static int g_col     = 1;   // column of the next character to be read
static int tokLine   = 1;   // line   where the current lexeme started
static int tokCol    = 1;   // column where the current lexeme started
static int litLine   = 1;   // where an unterminated string literal opened
static int litCol    = 1;
static int cmtLine   = 1;   // where an unterminated block comment opened
static int cmtCol    = 1;

static std::string g_strBuf;   // accumulates a string literal across rules

static void addToken(int id) {
    g_tokens.push_back({ std::string(yytext), id, tokLine });
}

// Used for string literals, whose text is built up across several rules and
// therefore is not simply yytext.
static void addTokenText(const std::string &text, int id) {
    g_tokens.push_back({ text, id, tokLine });
}

static void addError(const std::string &message) {
    g_errors.push_back({ message, tokLine, tokCol });
}

static void addErrorAt(int line, int col, const std::string &message) {
    g_errors.push_back({ message, line, col });
}

// The escapes our language accepts after a backslash.
static bool isValidEscape(char c) {
    return strchr("\\'\"0abfnrtv", c) != NULL;
}

// Render a byte for an error message; non-printable bytes shown as \xNN.
static std::string showChar(char c) {
    if (c >= 32 && c < 127) return std::string(1, c);
    char buf[8];
    snprintf(buf, sizeof buf, "\\x%02x", (unsigned char)c);
    return buf;
}

// YY_USER_ACTION runs automatically before EVERY rule action below.
// It remembers where this lexeme started, then walks the matched text to
// advance the column counter (resetting to 1 after each newline).
#define YY_USER_ACTION                                    \
    tokLine = yylineno;                                   \
    tokCol  = g_col;                                      \
    for (int i = 0; i < yyleng; ++i) {                    \
        if (yytext[i] == '\n') g_col = 1; else ++g_col;   \
    }
%}

%option noyywrap
%option yylineno
%option nounput noinput

/* ------------------------------------------------------------------------
 * Named patterns. Each one is substituted wherever {NAME} appears below.
 * ------------------------------------------------------------------------ */

DIGIT       [0-9]
                              /* a single decimal digit */
HEXDIGIT    [0-9a-fA-F]
                              /* a single hexadecimal digit */
LETTER      [a-zA-Z_]
                              /* legal FIRST character of an identifier */
IDCHAR      [a-zA-Z0-9_]
                              /* legal LATER character of an identifier */
EXPONENT    [eE][+-]?{DIGIT}+
                              /* exponent suffix of a float: e10, E+10, e-3 */
CHARBODY    ([^'\\\n]|\\.)
                              /* one unit inside '...': either any character
                                 that is not a quote/backslash/newline, or a
                                 backslash followed by any one character */

/* Exclusive start conditions. "Exclusive" (%x) means that while we are inside
 * one, ONLY rules tagged with it can match — the normal rules are switched
 * off. We use them so an unterminated string or comment can report the line
 * where it OPENED, which a single regex cannot do. */
%x BLOCK_COMMENT
%x STRING_LIT

%%

    /* =====================================================================
     * COMMENTS AND WHITESPACE
     * ===================================================================== */

"/*"                    { /* opens a block comment */
                          cmtLine = tokLine; cmtCol = tokCol;
                          BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/"     { /* closes the block comment */
                          BEGIN(INITIAL); }
<BLOCK_COMMENT>[^*\n]+  { /* a run of characters that are neither '*' nor a
                             newline — the bulk of the comment body, matched
                             in one go for speed */ }
<BLOCK_COMMENT>"*"      { /* a lone '*' not followed by '/' */ }
<BLOCK_COMMENT>\n       { /* newline inside the comment; yylineno auto-counts */ }
<BLOCK_COMMENT><<EOF>>  { /* end of file while still inside the comment */
                          addErrorAt(cmtLine, cmtCol,
                                     "unterminated block comment");
                          BEGIN(INITIAL); yyterminate(); }

"//"[^\n]*              { /* '//' followed by any run of non-newline
                             characters — a line comment, discarded */ }

[ \t\r]+                { /* one or more spaces, tabs or carriage returns */ }
\n                      { /* a newline; yylineno tracks it automatically */ }

    /* =====================================================================
     * IDENTIFIERS AND KEYWORDS
     * A keyword is just an identifier that happens to be in keywordTable(),
     * so there is ONE rule here, not thirty.
     * ===================================================================== */

{LETTER}{IDCHAR}*       { /* a letter or underscore, then any number of
                             letters/digits/underscores: foo, _x, arr2 */
                          auto it = keywordTable().find(yytext);
                          if (it != keywordTable().end()) addToken(it->second);
                          else                            addToken(TOK_IDENTIFIER);
                        }

    /* =====================================================================
     * NUMERIC LITERALS
     * ORDER: every valid form must appear ABOVE the malformed-number rule at
     * the end of this block. On an equal-length match flex takes the rule
     * listed first, so putting the malformed rule last keeps it from
     * swallowing valid numbers.
     * ===================================================================== */

{DIGIT}+"."{DIGIT}*{EXPONENT}?   { /* digits, a dot, optional digits, optional
                                      exponent: 3.14, 2., 1.5e-3 */
                                   addToken(TOK_FLOAT_LITERAL); }
"."{DIGIT}+{EXPONENT}?           { /* a dot then digits: .5, .25e3 */
                                   addToken(TOK_FLOAT_LITERAL); }
{DIGIT}+{EXPONENT}               { /* digits then a mandatory exponent: 1e10 */
                                   addToken(TOK_FLOAT_LITERAL); }

0[xX]{HEXDIGIT}+        { /* 0x or 0X then one or more hex digits: 0x1F */
                          addToken(TOK_INT_LITERAL); }
0[bB][01]+              { /* 0b or 0B then one or more binary digits: 0b1010 */
                          addToken(TOK_INT_LITERAL); }
0[0-7]+                 { /* a leading 0 then octal digits: 0755 */
                          addToken(TOK_INT_LITERAL); }
{DIGIT}+                { /* a plain run of decimal digits: 0, 42, 1000 */
                          errno = 0;
                          strtoll(yytext, NULL, 10);
                          if (errno == ERANGE)
                              addError(std::string("integer literal out of range: ")
                                       + yytext);
                          else
                              addToken(TOK_INT_LITERAL);
                        }

({DIGIT}|"."{DIGIT})[0-9a-zA-Z_.]*  { /* ORDER: last of the number rules.
                                         Starts like a number (a digit, or a
                                         dot followed by a digit) and then
                                         runs on through letters, digits,
                                         underscores and dots. Longest match
                                         means this beats the valid rules only
                                         when there IS trailing junk:
                                         123abc, 1.2.3, 0x, 1e */
                                      addError(std::string("malformed numeric literal: ")
                                               + yytext); }

    /* =====================================================================
     * CHARACTER LITERALS
     * ORDER: the \x rule must come first, otherwise '\x41' looks like four
     * separate body units and gets flagged as a multi-character literal.
     * ===================================================================== */

'\\x{HEXDIGIT}{1,2}'    { /* quote, backslash-x, one or two hex digits, quote:
                             '\x41', '\x7' */
                          addToken(TOK_CHAR_LITERAL); }
'{CHARBODY}'            { /* quote, exactly ONE body unit, quote: 'a', '\n' */
                          if (yytext[1] == '\\' && !isValidEscape(yytext[2]))
                              addError(std::string("invalid escape sequence: ")
                                       + yytext);
                          else
                              addToken(TOK_CHAR_LITERAL); }
''                      { /* two quotes with nothing between them */
                          addError("empty character literal"); }
'{CHARBODY}{2,}'        { /* quote, TWO OR MORE body units, quote: 'ab' */
                          addError(std::string("multi-character literal: ")
                                   + yytext); }
'{CHARBODY}*            { /* ORDER: last. A quote and any number of body
                             units with NO closing quote — the earlier rules
                             all match longer when a closing quote exists */
                          addError("unterminated character literal"); }

    /* =====================================================================
     * STRING LITERALS
     * Handled with a start condition rather than one big regex so that an
     * unterminated string can be reported at the line where it OPENED.
     * ===================================================================== */

\"                      { /* an opening double quote */
                          g_strBuf = "\"";
                          litLine = tokLine; litCol = tokCol;
                          BEGIN(STRING_LIT); }
<STRING_LIT>\"          { /* the closing double quote */
                          g_strBuf += '"';
                          addTokenText(g_strBuf, TOK_STRING_LITERAL);
                          BEGIN(INITIAL); }
<STRING_LIT>\\x{HEXDIGIT}{1,2}  { /* a hex escape inside the string: \x41 */
                                  g_strBuf += yytext; }
<STRING_LIT>\\(.|\n)    { /* a backslash and the one character after it,
                             newline included, so we can flag bad escapes */
                          if (!isValidEscape(yytext[1]))
                              addError(std::string("invalid escape sequence: ")
                                       + yytext);
                          g_strBuf += yytext; }
<STRING_LIT>[^\\\n"]+   { /* a run of ordinary string characters — anything
                             that is not a backslash, newline or quote */
                          g_strBuf += yytext; }
<STRING_LIT>\n          { /* a newline before the closing quote */
                          addErrorAt(litLine, litCol,
                                     "unterminated string literal");
                          BEGIN(INITIAL); }
<STRING_LIT><<EOF>>     { /* end of file before the closing quote */
                          addErrorAt(litLine, litCol,
                                     "unterminated string literal at end of file");
                          BEGIN(INITIAL); yyterminate(); }

    /* =====================================================================
     * OPERATORS
     * Two-character operators are listed before one-character ones. Strictly
     * this is unnecessary — flex's longest-match rule already prefers "+=" to
     * "+" — but reading them grouped this way makes the set easy to audit.
     * ===================================================================== */

"++"    { /* increment */                addToken(TOK_INC); }
"--"    { /* decrement */                addToken(TOK_DEC); }
"+="    { /* add and assign */           addToken(TOK_PLUS_ASSIGN); }
"-="    { /* subtract and assign */      addToken(TOK_MINUS_ASSIGN); }
"*="    { /* multiply and assign */      addToken(TOK_STAR_ASSIGN); }
"/="    { /* divide and assign */        addToken(TOK_SLASH_ASSIGN); }
"%="    { /* modulo and assign */        addToken(TOK_PERCENT_ASSIGN); }
"=="    { /* equality */                 addToken(TOK_EQ); }
"!="    { /* inequality */               addToken(TOK_NEQ); }
"<="    { /* less or equal */            addToken(TOK_LE); }
">="    { /* greater or equal */         addToken(TOK_GE); }
"&&"    { /* logical and */              addToken(TOK_AND); }
"||"    { /* logical or */               addToken(TOK_OR); }
"<<"    { /* left shift */               addToken(TOK_SHL); }
">>"    { /* right shift */              addToken(TOK_SHR); }
"->"    { /* member access via pointer */addToken(TOK_ARROW); }
"::"    { /* scope resolution */         addToken(TOK_SCOPE); }

"+"     { /* add */                      addToken(TOK_PLUS); }
"-"     { /* subtract or negate */       addToken(TOK_MINUS); }
"*"     { /* multiply, dereference, or pointer declarator — telling these
             apart is the parser's job, not ours */
                                         addToken(TOK_STAR); }
"/"     { /* divide */                   addToken(TOK_SLASH); }
"%"     { /* modulo */                   addToken(TOK_PERCENT); }
"="     { /* assignment */               addToken(TOK_ASSIGN); }
"<"     { /* less than */                addToken(TOK_LT); }
">"     { /* greater than */             addToken(TOK_GT); }
"!"     { /* logical not */              addToken(TOK_NOT); }
"&"     { /* bitwise and, address-of, or reference declarator — again the
             parser decides which */
                                         addToken(TOK_BITAND); }
"|"     { /* bitwise or */               addToken(TOK_BITOR); }
"^"     { /* bitwise xor */              addToken(TOK_BITXOR); }
"~"     { /* bitwise not */              addToken(TOK_BITNOT); }
"."     { /* member access */            addToken(TOK_DOT); }
"?"     { /* ternary conditional */      addToken(TOK_QUESTION); }
":"     { /* ternary branch, case label, access specifier, base class */
                                         addToken(TOK_COLON); }

    /* =====================================================================
     * PUNCTUATION
     * ===================================================================== */

"("     { addToken(TOK_LPAREN); }
")"     { addToken(TOK_RPAREN); }
"{"     { addToken(TOK_LBRACE); }
"}"     { addToken(TOK_RBRACE); }
"["     { addToken(TOK_LBRACKET); }
"]"     { addToken(TOK_RBRACKET); }
";"     { addToken(TOK_SEMI); }
","     { addToken(TOK_COMMA); }

    /* =====================================================================
     * ANYTHING ELSE
     * ===================================================================== */

.       { /* a single character no rule above accepted: @, $, #, backtick.
             We record it and keep scanning, because the assignment asks for
             ALL lexical errors, not just the first */
          addError("illegal character '" + showChar(yytext[0]) + "'"); }

%%

static void printTable() {
    // Size the Lexeme column to the widest lexeme so a long string literal
    // cannot push the Token column out of alignment.
    size_t width = 6;                       // length of the word "Lexeme"
    for (auto &t : g_tokens)
        if (t.lexeme.size() > width) width = t.lexeme.size();
    width += 2;                             // gap between the columns

    std::cout << std::left << std::setw((int)width) << "Lexeme" << "Token\n";
    std::cout << std::string(width + 16, '-') << "\n";
    for (auto &t : g_tokens)
        std::cout << std::left << std::setw((int)width) << t.lexeme
                  << tokenCategory(t.id) << "\n";
    std::cout << std::string(width + 16, '-') << "\n";
    std::cout << g_tokens.size() << " tokens.\n";
}

static void printErrors(const char *filename) {
    for (auto &e : g_errors)
        std::cout << filename << ":" << e.line << ":" << e.col
                  << ": error: " << e.message << "\n";
    std::cout << g_errors.size() << " lexical error(s) found.\n";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <source-file>\n";
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        std::cerr << "error: cannot open " << argv[1] << "\n";
        return 1;
    }
    yyin = f;

    // No rule above executes `return`, so this single call scans the whole
    // file — every token and error is recorded by addToken()/addError() as
    // it goes — and then returns 0 at end of input.
    yylex();
    fclose(f);

    // The assignment asks for the table only when the program is clean, and
    // for the full error list otherwise.
    if (!g_errors.empty()) {
        printErrors(argv[1]);
        return 1;
    }
    printTable();
    return 0;
}
