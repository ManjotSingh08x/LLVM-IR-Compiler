%{
/*
 * lexer.l — Lexical Analyzer, Assignment 1, CSC-305
 *
 * Design note (read before extending):
 *   This file ONLY recognizes lexemes and classifies them via token_types.h.
 *   It does not know about grammar, so it stays reusable as-is once a bison
 *   parser is added in Assignment 2 — at that point:
 *     1. swap main()'s driver loop for bison's generated parser calling yylex()
 *     2. change `return TOK_X;` to also set yylval where needed
 *     3. nothing in the rules section below needs to change
 *   Keep it that way: don't hardcode assignment-1-only behavior into the
 *   rules, keep it in main()/printTable() instead.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
    std::string badText;
    int         line;
};

static std::vector<TokenRecord> g_tokens;
static std::vector<LexError>    g_errors;

static void addToken(int id) {
    g_tokens.push_back({ std::string(yytext), id, yylineno });
}

static void addError() {
    g_errors.push_back({ std::string(yytext), yylineno });
}
%}

%option noyywrap
%option yylineno

DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDCHAR      [a-zA-Z0-9_]

%x BLOCK_COMMENT

%%

"/*"                    { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/"     { BEGIN(INITIAL); }
<BLOCK_COMMENT>\n       { /* yylineno auto-increments, just consume */ }
<BLOCK_COMMENT>.        { /* consume comment body */ }

"//"[^\n]*              { /* line comment, skip */ }

[ \t\r]+                { /* whitespace, skip */ }
\n                       { /* newline, skip (yylineno tracks it) */ }

{LETTER}{IDCHAR}*        {
                            auto it = keywordTable().find(yytext);
                            if (it != keywordTable().end()) addToken(it->second);
                            else addToken(TOK_IDENTIFIER);
                         }

{DIGIT}+"."{DIGIT}+([eE][+-]?{DIGIT}+)?   { addToken(TOK_FLOAT_LITERAL); }
{DIGIT}+                                  { addToken(TOK_INT_LITERAL); }

\'([^\'\\\n]|\\.)\'      { addToken(TOK_CHAR_LITERAL); }
\"([^\"\\\n]|\\.)*\"     { addToken(TOK_STRING_LITERAL); }

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
"*"     { addToken(TOK_STAR); }
"/"     { addToken(TOK_SLASH); }
"%"     { addToken(TOK_PERCENT); }
"="     { addToken(TOK_ASSIGN); }
"<"     { addToken(TOK_LT); }
">"     { addToken(TOK_GT); }
"!"     { addToken(TOK_NOT); }
"&"     { addToken(TOK_BITAND); }
"|"     { addToken(TOK_BITOR); }
"^"     { addToken(TOK_BITXOR); }
"~"     { addToken(TOK_BITNOT); }

"("     { addToken(TOK_LPAREN); }
")"     { addToken(TOK_RPAREN); }
"{"     { addToken(TOK_LBRACE); }
"}"     { addToken(TOK_RBRACE); }
"["     { addToken(TOK_LBRACKET); }
"]"     { addToken(TOK_RBRACKET); }
";"     { addToken(TOK_SEMI); }
","     { addToken(TOK_COMMA); }
"."     { addToken(TOK_DOT); }
":"     { addToken(TOK_COLON); }

.       { addError(); }

%%

static void printTable() {
    std::cout << std::left << std::setw(20) << "Lexeme"
              << "Token" << "\n";
    std::cout << std::string(40, '-') << "\n";
    for (auto &t : g_tokens) {
        std::cout << std::left << std::setw(20) << t.lexeme
                   << tokenCategory(t.id) << "\n";
    }
}

static void printErrors() {
    std::cerr << "Lexical errors found: " << g_errors.size() << "\n";
    for (auto &e : g_errors) {
        std::cerr << "  line " << e.line << ": unexpected character '"
                   << e.badText << "'\n";
    }
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

    int tok;
    while ((tok = yylex()) != 0) {
        // token already recorded by addToken() inside the matching rule
        (void)tok;
    }
    fclose(f);

    if (!g_errors.empty()) {
        printErrors();
        return 2;
    }
    printTable();
    return 0;
}
