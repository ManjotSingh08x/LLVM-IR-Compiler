%{
/* lexer.l — flex-based lexical analyzer for our C-like language
 *
 * how flex picks rules:
 *   longest match wins. ties go to whichever rule is listed first.
 *   some rules below rely on that ordering (marked ORDER).
 *
 * to hook this up with bison later:
 *   1. add "return TOK_X;" after each addToken() call
 *   2. drop main() and printTable(), let yyparse() drive things
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

// flex gives us yylineno for free, but columns we track ourselves
static int g_col     = 1;
static int tokLine   = 1;   // line where current lexeme starts
static int tokCol    = 1;   // column where current lexeme starts
static int litLine   = 1;   // where a string literal opened (for error msgs)
static int litCol    = 1;
static int cmtLine   = 1;   // where a block comment opened
static int cmtCol    = 1;

static std::string g_strBuf;   // builds up a string literal piece by piece

static void addToken(int id) {
    g_tokens.push_back({ std::string(yytext), id, tokLine });
}

// for string literals — their text comes from g_strBuf, not yytext
static void addTokenText(const std::string &text, int id) {
    g_tokens.push_back({ text, id, tokLine });
}

static void addError(const std::string &message) {
    g_errors.push_back({ message, tokLine, tokCol });
}

static void addErrorAt(int line, int col, const std::string &message) {
    g_errors.push_back({ message, line, col });
}

// which escape chars we allow after a backslash
static bool isValidEscape(char c) {
    return strchr("\\\'\"\0abfnrtv", c) != NULL;
}

// format a char nicely for error messages
static std::string showChar(char c) {
    if (c >= 32 && c < 127) return std::string(1, c);
    char buf[8];
    snprintf(buf, sizeof buf, "\\x%02x", (unsigned char)c);
    return buf;
}

// runs before every rule — saves where the lexeme started, updates column
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

/* named patterns */
DIGIT       [0-9]
HEXDIGIT    [0-9a-fA-F]
LETTER      [a-zA-Z_]
IDCHAR      [a-zA-Z0-9_]
EXPONENT    [eE][+-]?{DIGIT}+
CHARBODY    ([^'\\\n]|\\.)

/* exclusive start conditions for multi-line constructs */
%x BLOCK_COMMENT
%x STRING_LIT

%%

    /* --- comments and whitespace --- */

"/*"                    { cmtLine = tokLine; cmtCol = tokCol;
                          BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/"     { BEGIN(INITIAL); }
<BLOCK_COMMENT>[^*\n]+  { /* eat comment body */ }
<BLOCK_COMMENT>"*"      { /* lone star, not end of comment */ }
<BLOCK_COMMENT>\n       { }
<BLOCK_COMMENT><<EOF>>  { addErrorAt(cmtLine, cmtCol,
                                     "unterminated block comment");
                          BEGIN(INITIAL); yyterminate(); }

"//"[^\n]*              { /* line comment, skip it */ }

[ \t\r]+                { /* whitespace */ }
\n                      { }

    /* --- identifiers and keywords --- */
    /* one rule handles both; we just look up the text in the keyword table */

{LETTER}{IDCHAR}*       { auto it = keywordTable().find(yytext);
                          if (it != keywordTable().end()) addToken(it->second);
                          else                            addToken(TOK_IDENTIFIER);
                        }

    /* --- numeric literals --- */
    /* ORDER: valid forms must come before the catch-all malformed rule */

{DIGIT}+"."{DIGIT}*{EXPONENT}?   { addToken(TOK_FLOAT_LITERAL); }
"."{DIGIT}+{EXPONENT}?           { addToken(TOK_FLOAT_LITERAL); }
{DIGIT}+{EXPONENT}               { addToken(TOK_FLOAT_LITERAL); }

0[xX]{HEXDIGIT}+        { addToken(TOK_INT_LITERAL); }
0[bB][01]+              { addToken(TOK_INT_LITERAL); }
0[0-7]+                 { addToken(TOK_INT_LITERAL); }
{DIGIT}+                { errno = 0;
                          strtoll(yytext, NULL, 10);
                          if (errno == ERANGE)
                              addError(std::string("integer literal out of range: ")
                                       + yytext);
                          else
                              addToken(TOK_INT_LITERAL);
                        }

    /* ORDER: catch-all for malformed numbers like 123abc, 1.2.3, 0x, 1e */
({DIGIT}|"."{DIGIT})[0-9a-zA-Z_.]*  { addError(std::string("malformed numeric literal: ")
                                               + yytext); }

    /* --- character literals --- */
    /* ORDER: hex escape rule first, otherwise '\x41' looks like multi-char */

'\x{HEXDIGIT}{1,2}'    { addToken(TOK_CHAR_LITERAL); }
'{CHARBODY}'            { if (yytext[1] == '\\' && !isValidEscape(yytext[2]))
                              addError(std::string("invalid escape sequence: ")
                                       + yytext);
                          else
                              addToken(TOK_CHAR_LITERAL); }
''                      { addError("empty character literal"); }
'{CHARBODY}{2,}'        { addError(std::string("multi-character literal: ")
                                   + yytext); }
'{CHARBODY}*            { /* ORDER: last — no closing quote found */
                          addError("unterminated character literal"); }

    /* --- string literals --- */
    /* uses a start condition so we can report the opening line on errors */

\"                      { g_strBuf = "\"";
                          litLine = tokLine; litCol = tokCol;
                          BEGIN(STRING_LIT); }
<STRING_LIT>\"          { g_strBuf += '"';
                          addTokenText(g_strBuf, TOK_STRING_LITERAL);
                          BEGIN(INITIAL); }
<STRING_LIT>\\x{HEXDIGIT}{1,2}  { g_strBuf += yytext; }
<STRING_LIT>\\(.|\n)    { if (!isValidEscape(yytext[1]))
                              addError(std::string("invalid escape sequence: ")
                                       + yytext);
                          g_strBuf += yytext; }
<STRING_LIT>[^\\\n"]+   { g_strBuf += yytext; }
<STRING_LIT>\n          { addErrorAt(litLine, litCol,
                                     "unterminated string literal");
                          BEGIN(INITIAL); }
<STRING_LIT><<EOF>>     { addErrorAt(litLine, litCol,
                                     "unterminated string literal at end of file");
                          BEGIN(INITIAL); yyterminate(); }

    /* --- operators --- */
    /* multi-char ops listed first (longest match handles it anyway, but
       grouping them this way makes it easier to see what we support) */

"++"    { addToken(TOK_INC); }
"--"    { addToken(TOK_DEC); }
"+="    { addToken(TOK_PLUS_ASSIGN); }
"-="    { addToken(TOK_MINUS_ASSIGN); }
"*="    { addToken(TOK_STAR_ASSIGN); }
"/="    { addToken(TOK_SLASH_ASSIGN); }
"%="    { addToken(TOK_PERCENT_ASSIGN); }
"=="    { addToken(TOK_EQ); }
"!="    { addToken(TOK_NEQ); }
"<="    { addToken(TOK_LE); }
">="    { addToken(TOK_GE); }
"&&"    { addToken(TOK_AND); }
"||"    { addToken(TOK_OR); }
"<<"    { addToken(TOK_SHL); }
">>"    { addToken(TOK_SHR); }
"->"    { addToken(TOK_ARROW); }
"::"    { addToken(TOK_SCOPE); }

"+"     { addToken(TOK_PLUS); }
"-"     { addToken(TOK_MINUS); }
"*"     { addToken(TOK_STAR); }     // multiply, deref, or ptr — parser decides
"/"     { addToken(TOK_SLASH); }
"%"     { addToken(TOK_PERCENT); }
"="     { addToken(TOK_ASSIGN); }
"<"     { addToken(TOK_LT); }
">"     { addToken(TOK_GT); }
"!"     { addToken(TOK_NOT); }
"&"     { addToken(TOK_BITAND); }   // bitand, address-of, or ref — parser decides
"|"     { addToken(TOK_BITOR); }
"^"     { addToken(TOK_BITXOR); }
"~"     { addToken(TOK_BITNOT); }
"."     { addToken(TOK_DOT); }
"?"     { addToken(TOK_QUESTION); }
":"     { addToken(TOK_COLON); }

    /* --- punctuation --- */

"("     { addToken(TOK_LPAREN); }
")"     { addToken(TOK_RPAREN); }
"{"     { addToken(TOK_LBRACE); }
"}"     { addToken(TOK_RBRACE); }
"["     { addToken(TOK_LBRACKET); }
"]"     { addToken(TOK_RBRACKET); }
";"     { addToken(TOK_SEMI); }
","     { addToken(TOK_COMMA); }

    /* --- catch-all for illegal characters --- */

.       { addError("illegal character '" + showChar(yytext[0]) + "'"); }

%%

static void printTable() {
    // size the lexeme column to the widest entry
    size_t width = 6;   // at least as wide as "Lexeme"
    for (auto &t : g_tokens)
        if (t.lexeme.size() > width) width = t.lexeme.size();
    width += 2;

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

    yylex();   // scans the whole file in one go
    fclose(f);

    // clean file → print token table; errors → print error list
    if (!g_errors.empty()) {
        printErrors(argv[1]);
        return 1;
    }
    printTable();
    return 0;
}
