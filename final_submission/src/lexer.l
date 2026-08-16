%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "token_types.h"

//structs
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

//globals
static std::vector<TokenRecord> g_tokens;
static std::vector<LexError>    g_errors;

static int g_col     = 1;
static int tokLine   = 1;
static int tokCol    = 1;
static int litLine   = 1;
static int litCol    = 1;
static int cmtLine   = 1;
static int cmtCol    = 1;

static std::string g_strBuf;

//helpers
static void addToken(int id) {
    g_tokens.push_back({ std::string(yytext), id, tokLine });
}

static void addTokenText(const std::string &text, int id) {
    g_tokens.push_back({ text, id, tokLine });
}

static void addError(const std::string &message) {
    g_errors.push_back({ message, tokLine, tokCol });
}

static void addErrorAt(int line, int col, const std::string &message) {
    g_errors.push_back({ message, line, col });
}

static bool isValidEscape(char c) {
    return strchr("\\\\'\"0abfnrtv", c) != NULL;
}

static std::string showChar(char c) {
    if (c >= 32 && c < 127) return std::string(1, c);
    char buf[8];
    snprintf(buf, sizeof buf, "\\x%02x", (unsigned char)c);
    return buf;
}

//column-tracking macro
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


DIGIT       [0-9]
HEXDIGIT    [0-9a-fA-F]
LETTER      [a-zA-Z_]
IDCHAR      [a-zA-Z0-9_]
EXPONENT    [eE][+-]?{DIGIT}+
CHARBODY    ([^'\\\n]|\\.)


%x BLOCK_COMMENT
%x STRING_LIT

%%


"/*"                    { cmtLine = tokLine; cmtCol = tokCol;
                          BEGIN(BLOCK_COMMENT); }                  // opening block comment
<BLOCK_COMMENT>"*/"     { BEGIN(INITIAL); }                       // closing block comment
<BLOCK_COMMENT>[^*\n]+  { }                                       // comment body chars (not * or newline)
<BLOCK_COMMENT>"*"      { }                                       // lone * not followed by /
<BLOCK_COMMENT>\n        { }                                      // newline inside block comment
<BLOCK_COMMENT><<EOF>>   { addErrorAt(cmtLine, cmtCol,
                                     "unterminated block comment");
                          BEGIN(INITIAL); yyterminate(); }         // EOF inside block comment

"//"[^\n]*              { }                                       // line comment

[ \t\r]+                { }                                       // horizontal whitespace
\n                      { }                                       // newline


{LETTER}{IDCHAR}*       { auto it = keywordTable().find(yytext);
                          if (it != keywordTable().end()) addToken(it->second);
                          else                            addToken(TOK_IDENTIFIER);
                        }                                         // letter or underscore followed by alphanumerics

{DIGIT}+"."{DIGIT}*{EXPONENT}?   { addToken(TOK_FLOAT_LITERAL); }  // digits, dot, optional digits and exponent
"."{DIGIT}+{EXPONENT}?           { addToken(TOK_FLOAT_LITERAL); }  // dot then digits, optional exponent
{DIGIT}+{EXPONENT}               { addToken(TOK_FLOAT_LITERAL); }  // digits with mandatory exponent

0[xX]{HEXDIGIT}+        { addToken(TOK_INT_LITERAL); }            // hexadecimal integer
0[bB][01]+              { addToken(TOK_INT_LITERAL); }            // binary integer
0[0-7]+                 { addToken(TOK_INT_LITERAL); }            // octal integer
{DIGIT}+                { errno = 0;
                          strtoll(yytext, NULL, 10);
                          if (errno == ERANGE)
                              addError(std::string("integer literal out of range: ")
                                       + yytext);
                          else
                              addToken(TOK_INT_LITERAL);
                        }                                         // decimal integer with range check

({DIGIT}|"."{DIGIT})[0-9a-zA-Z_.]*  { addError(std::string("malformed numeric literal: ")
                                               + yytext); }      // starts like a number but has trailing junk


'\\x{HEXDIGIT}{1,2}'    { addToken(TOK_CHAR_LITERAL); }          // hex escape char literal
'{CHARBODY}'            { if (yytext[1] == '\\' && !isValidEscape(yytext[2]))
                              addError(std::string("invalid escape sequence: ")
                                       + yytext);
                          else
                              addToken(TOK_CHAR_LITERAL); }       // single character or escape sequence
''                      { addError("empty character literal"); }  // empty quotes
'{CHARBODY}{2,}'        { addError(std::string("multi-character literal: ")
                                   + yytext); }                   // two or more chars between quotes
'{CHARBODY}*            { addError("unterminated character literal"); }  // opening quote with no closing quote


\"                      { g_strBuf = "\"";
                          litLine = tokLine; litCol = tokCol;
                          BEGIN(STRING_LIT); }                     // opening double quote
<STRING_LIT>\"          { g_strBuf += '"';
                          addTokenText(g_strBuf, TOK_STRING_LITERAL);
                          BEGIN(INITIAL); }                        // closing double quote
<STRING_LIT>\\x{HEXDIGIT}{1,2}  { g_strBuf += yytext; }          // hex escape inside string
<STRING_LIT>\\(.|\n)    { if (!isValidEscape(yytext[1]))
                              addError(std::string("invalid escape sequence: ")
                                       + yytext);
                          g_strBuf += yytext; }                   // backslash escape inside string
<STRING_LIT>[^\\\n"]+   { g_strBuf += yytext; }                  // run of ordinary string characters
<STRING_LIT>\n          { addErrorAt(litLine, litCol,
                                     "unterminated string literal");
                          BEGIN(INITIAL); }                        // newline before closing quote
<STRING_LIT><<EOF>>     { addErrorAt(litLine, litCol,
                                     "unterminated string literal at end of file");
                          BEGIN(INITIAL); yyterminate(); }         // EOF before closing quote


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
"."     { addToken(TOK_DOT); }             
"?"     { addToken(TOK_QUESTION); }        
":"     { addToken(TOK_COLON); }          


"("     { addToken(TOK_LPAREN); }          
")"     { addToken(TOK_RPAREN); }          
"{"     { addToken(TOK_LBRACE); }          
"}"     { addToken(TOK_RBRACE); }          
"["     { addToken(TOK_LBRACKET); }        
"]"     { addToken(TOK_RBRACKET); }        
";"     { addToken(TOK_SEMI); }            
","     { addToken(TOK_COMMA); }          


.       { addError("illegal character '" + showChar(yytext[0]) + "'"); }  // any unrecognized character

%%
//output
using namespace std;
static void printTable(){
    size_t width = 6;
    for(size_t i = 0; i < g_tokens.size(); i++){
        if(g_tokens[i].lexeme.size() > width){
            width = g_tokens[i].lexeme.size();
        }
    }
    width += 2;
    cout << left << setw(width) << "Lexeme" << "Token" << endl;
    cout << string(width + 16, '-') << endl;
    for(size_t i = 0; i < g_tokens.size(); i++){
        string lexeme = g_tokens[i].lexeme;
        string category = tokenCategory(g_tokens[i].id);
        cout << left << setw(width) << lexeme << category << endl;
    }
    cout << "Total Tokens: " << g_tokens.size() << endl;
}

static void printErrors(const char *filename) {
    for(size_t i = 0; i < g_errors.size(); i++){
        int line = g_errors[i].line;
        int col = g_errors[i].col;
        string message = g_errors[i].message;
        cout << filename << " line: " << line << " col: " << col << " error: " << message << endl;
    }
    cout << g_errors.size() << " lexical error(s) found." << endl;
}

//main
int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "usage: " << argv[0] << " <source-file>\n";
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        cerr << "error: cannot open " << argv[1] << "\n";
        return 1;
    }
    yyin = f;
    yylex(); // since return is not called, this will scan the entire file
    fclose(f);
    if (!g_errors.empty()) {
        printErrors(argv[1]);
        return 1;
    }
    printTable();
    return 0;
}
