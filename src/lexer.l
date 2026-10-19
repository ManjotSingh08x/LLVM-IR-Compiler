%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include "token_types.h"

//structs
struct TokenRecord {
    std::string lexeme;
    int         id;
    int         line;
    std::string declType;   //what an identifier was declared as, filled by the parser
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
static int addToken(int id) {
    //the semantic value of every token is its position in g_tokens, which lets
    //the parser record a declared type against the right entry
    yylval = (int)g_tokens.size();
    g_tokens.push_back({ std::string(yytext), id, tokLine, std::string() });
    return id;
}

static int addTokenText(const std::string &text, int id) {
    yylval = (int)g_tokens.size();
    g_tokens.push_back({ text, id, tokLine, std::string() });
    return id;
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

//column and parser location tracking
#define YY_USER_ACTION                                    \
    tokLine = yylineno;                                   \
    tokCol  = g_col;                                      \
    yylloc.first_line = tokLine;                          \
    yylloc.first_column = tokCol;                         \
    for (int i = 0; i < yyleng; ++i) {                    \
        if (yytext[i] == '\n') g_col = 1; else ++g_col;   \
    }                                                     \
    yylloc.last_line = yylineno;                          \
    yylloc.last_column = (g_col > 1) ? g_col - 1 : 1;
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
                          if (it != keywordTable().end()) return addToken(it->second);
                          else                            return addToken(TOK_IDENTIFIER);
                        }                                         // letter or underscore followed by alphanumerics

{DIGIT}+"."{DIGIT}*{EXPONENT}?   { return addToken(TOK_FLOAT_LITERAL); }  // digits, dot, optional digits and exponent
"."{DIGIT}+{EXPONENT}?           { return addToken(TOK_FLOAT_LITERAL); }  // dot then digits, optional exponent
{DIGIT}+{EXPONENT}               { return addToken(TOK_FLOAT_LITERAL); }  // digits with mandatory exponent

0[xX]{HEXDIGIT}+        { return addToken(TOK_INT_LITERAL); }     // hexadecimal integer
0[bB][01]+              { return addToken(TOK_INT_LITERAL); }     // binary integer
0[0-7]+                 { return addToken(TOK_INT_LITERAL); }     // octal integer
{DIGIT}+                { errno = 0;
                          strtoll(yytext, NULL, 10);
                          if (errno == ERANGE)
                              addError(std::string("integer literal out of range: ")
                                       + yytext);
                          else
                              return addToken(TOK_INT_LITERAL);
                        }                                         // decimal integer with range check

({DIGIT}|"."{DIGIT})[0-9a-zA-Z_.]*  { addError(std::string("malformed numeric literal: ")
                                               + yytext); }      // starts like a number but has trailing junk


'\\x{HEXDIGIT}{1,2}'    { return addToken(TOK_CHAR_LITERAL); }   // hex escape char literal
'{CHARBODY}'            { if (yytext[1] == '\\' && !isValidEscape(yytext[2]))
                              addError(std::string("invalid escape sequence: ")
                                       + yytext);
                          else
                              return addToken(TOK_CHAR_LITERAL); } // single character or escape sequence
''                      { addError("empty character literal"); }  // empty quotes
'{CHARBODY}{2,}'        { addError(std::string("multi-character literal: ")
                                   + yytext); }                   // two or more chars between quotes
'{CHARBODY}*            { addError("unterminated character literal"); }  // opening quote with no closing quote


\"                      { g_strBuf = "\"";
                          litLine = tokLine; litCol = tokCol;
                          BEGIN(STRING_LIT); }                     // opening double quote
<STRING_LIT>\"          { g_strBuf += '"';
                          yylloc.first_line = litLine;
                          yylloc.first_column = litCol;
                          BEGIN(INITIAL);
                          return addTokenText(g_strBuf, TOK_STRING_LITERAL); } // closing double quote
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


"++"    { return addToken(TOK_INC); }             
"--"    { return addToken(TOK_DEC); }             
"+="    { return addToken(TOK_PLUS_ASSIGN); }      
"-="    { return addToken(TOK_MINUS_ASSIGN); }     
"*="    { return addToken(TOK_STAR_ASSIGN); }      
"/="    { return addToken(TOK_SLASH_ASSIGN); }     
"%="    { return addToken(TOK_PERCENT_ASSIGN); }   
"=="    { return addToken(TOK_EQ); }               
"!="    { return addToken(TOK_NEQ); }              
"<="    { return addToken(TOK_LE); }               
">="    { return addToken(TOK_GE); }               
"&&"    { return addToken(TOK_AND); }              
"||"    { return addToken(TOK_OR); }               
"<<"    { return addToken(TOK_SHL); }              
">>"    { return addToken(TOK_SHR); }              
"->"    { return addToken(TOK_ARROW); }            
"::"    { return addToken(TOK_SCOPE); }            

"+"     { return addToken(TOK_PLUS); }             
"-"     { return addToken(TOK_MINUS); }            
"*"     { return addToken(TOK_STAR); }             
"/"     { return addToken(TOK_SLASH); }            
"%"     { return addToken(TOK_PERCENT); }          
"="     { return addToken(TOK_ASSIGN); }           
"<"     { return addToken(TOK_LT); }               
">"     { return addToken(TOK_GT); }               
"!"     { return addToken(TOK_NOT); }              
"&"     { return addToken(TOK_BITAND); }           
"|"     { return addToken(TOK_BITOR); }            
"^"     { return addToken(TOK_BITXOR); }           
"~"     { return addToken(TOK_BITNOT); }           
"."     { return addToken(TOK_DOT); }              
"?"     { return addToken(TOK_QUESTION); }         
":"     { return addToken(TOK_COLON); }            


"("     { return addToken(TOK_LPAREN); }          
")"     { return addToken(TOK_RPAREN); }          
"{"     { return addToken(TOK_LBRACE); }          
"}"     { return addToken(TOK_RBRACE); }          
"["     { return addToken(TOK_LBRACKET); }        
"]"     { return addToken(TOK_RBRACKET); }        
";"     { return addToken(TOK_SEMI); }            
","     { return addToken(TOK_COMMA); }          


.       { addError("illegal character '" + showChar(yytext[0]) + "'"); }  // any unrecognized character

%%
//output
using namespace std;
void setDeclType(int index, const std::string &type){
    if(index >= 0 && index < (int)g_tokens.size() && g_tokens[index].declType.empty()){
        g_tokens[index].declType = type;
    }
}

std::string lexemeAt(int index){
    if(index >= 0 && index < (int)g_tokens.size()){
        return g_tokens[index].lexeme;
    }
    return std::string();
}

void printTable(){
    map<string, string> declaredTypes;
    for(size_t i = 0; i < g_tokens.size(); i++){
        if(g_tokens[i].id == TOK_IDENTIFIER && !g_tokens[i].declType.empty()){
            if(declaredTypes.find(g_tokens[i].lexeme) == declaredTypes.end()){
                declaredTypes[g_tokens[i].lexeme] = g_tokens[i].declType;
            }
        }
    }
    for(size_t i = 0; i < g_tokens.size(); i++){
        if(g_tokens[i].id == TOK_IDENTIFIER && g_tokens[i].declType.empty()){
            map<string, string>::const_iterator it = declaredTypes.find(g_tokens[i].lexeme);
            if(it != declaredTypes.end()){
                g_tokens[i].declType = it->second;
            }
        }
    }

    vector<string> firstColumn;
    firstColumn.reserve(g_tokens.size());

    size_t width = 6;
    for(size_t i = 0; i < g_tokens.size(); i++){
        string entry = g_tokens[i].lexeme;
        if(entry.size() > width){
            width = entry.size();
        }
        firstColumn.push_back(entry);
    }
    width += 2;

    cout << left << setw(width) << "Token" << "| Token_Type" << endl;
    cout << string(width, '-') << "+" << string(15, '-') << endl;
    for(size_t i = 0; i < g_tokens.size(); i++){
        cout << left << setw(width) << firstColumn[i] << "| "
             << tokenDisplayType(g_tokens[i].id, g_tokens[i].declType) << endl;
    }
    cout << endl;
    cout << "Total Tokens: " << g_tokens.size() << endl;
}

void printErrors(const char *filename) {
    for(size_t i = 0; i < g_errors.size(); i++){
        int line = g_errors[i].line;
        int col = g_errors[i].col;
        string message = g_errors[i].message;
        cout << filename << " line: " << line << " col: " << col << " error: " << message << endl;
    }
    cout << g_errors.size() << " lexical error(s) found." << endl;
}

size_t lexicalErrorCount() {
    return g_errors.size();
}
