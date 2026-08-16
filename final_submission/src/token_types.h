#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H

#include <string>
#include <unordered_map>

enum TokenID {
    //identifiers
    TOK_IDENTIFIER = 258,   
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_CHAR_LITERAL,
    TOK_STRING_LITERAL,

    // keywords 
    TOK_INT, TOK_CHAR, TOK_FLOAT, TOK_DOUBLE, TOK_VOID, TOK_BOOL, TOK_AUTO,
    TOK_DEFAULT, TOK_BREAK, TOK_CONTINUE, TOK_GOTO, TOK_RETURN,
    TOK_IF, TOK_ELSE, TOK_FOR, TOK_WHILE, TOK_DO, TOK_SWITCH, TOK_CASE,
    TOK_STRUCT, TOK_CLASS, TOK_PUBLIC, TOK_PRIVATE, TOK_PROTECTED,
    TOK_NEW, TOK_DELETE, TOK_TRUE, TOK_FALSE, TOK_THIS, TOK_NULL, TOK_SIZEOF,
    TOK__KEYWORD_END,       

    // operators
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    TOK_INC, TOK_DEC,
    TOK_ASSIGN, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_STAR_ASSIGN,
    TOK_SLASH_ASSIGN, TOK_PERCENT_ASSIGN,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_BITAND, TOK_BITOR, TOK_BITXOR, TOK_BITNOT, TOK_SHL, TOK_SHR,
    TOK_ARROW, TOK_SCOPE, TOK_DOT,
    TOK_QUESTION, TOK_COLON,
    TOK__OPERATOR_END,      

    // punctuation 
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET, TOK_SEMI, TOK_COMMA,
    TOK__PUNCTUATION_END    
};

inline std::string tokenCategory(int tok) {
    if (tok == TOK_IDENTIFIER)    return "identifier";
    if (tok == TOK_INT_LITERAL)   return "integer_literal";
    if (tok == TOK_FLOAT_LITERAL) return "float_literal";
    if (tok == TOK_CHAR_LITERAL)  return "char_literal";
    if (tok == TOK_STRING_LITERAL)return "string_literal";
    if (tok >= TOK_INT    && tok < TOK__KEYWORD_END)     return "keyword";
    if (tok >= TOK_PLUS   && tok < TOK__OPERATOR_END)    return "operator";
    if (tok >= TOK_LPAREN && tok < TOK__PUNCTUATION_END) return "punctuation";
    return "unknown";
}

inline const std::unordered_map<std::string, int>& keywordTable() {
    static const std::unordered_map<std::string, int> kw = {
        {"int", TOK_INT}, {"char", TOK_CHAR}, {"float", TOK_FLOAT},
        {"double", TOK_DOUBLE}, {"void", TOK_VOID}, {"bool", TOK_BOOL},
        {"auto", TOK_AUTO},
        {"if", TOK_IF}, {"else", TOK_ELSE}, {"for", TOK_FOR},
        {"while", TOK_WHILE}, {"do", TOK_DO}, {"switch", TOK_SWITCH},
        {"case", TOK_CASE}, {"default", TOK_DEFAULT}, {"break", TOK_BREAK},
        {"continue", TOK_CONTINUE}, {"goto", TOK_GOTO}, {"return", TOK_RETURN},
        {"struct", TOK_STRUCT}, {"class", TOK_CLASS}, {"public", TOK_PUBLIC},
        {"private", TOK_PRIVATE}, {"protected", TOK_PROTECTED},
        {"new", TOK_NEW}, {"delete", TOK_DELETE}, {"true", TOK_TRUE},
        {"false", TOK_FALSE}, {"this", TOK_THIS}, {"null", TOK_NULL},
        {"sizeof", TOK_SIZEOF}
    };
    return kw;
}

#endif 
