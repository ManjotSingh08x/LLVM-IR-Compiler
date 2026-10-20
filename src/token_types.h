#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H

#include <cctype>
#include <string>
#include <unordered_map>
#include "ast/ast.h"  //the token value union refers to ast types
#include "declarator.h"
#include "parser.tab.h"

inline std::string tokenCategory(int tok) {
    if (tok == TOK_IDENTIFIER)    return "identifier";
    if (tok == TOK_INT_LITERAL)   return "integer_literal";
    if (tok == TOK_FLOAT_LITERAL) return "float_literal";
    if (tok == TOK_CHAR_LITERAL)  return "char_literal";
    if (tok == TOK_STRING_LITERAL)return "string_literal";
    if (tok >= TOK_INT    && tok < TOK__KEYWORD_END)     return "keyword";
    if (tok >= TOK_PLUS   && tok < TOK__OPERATOR_END)    return "operator";
    if (tok >= TOK_LPAREN && tok < TOK__PUNCTUATION_END) return "punctuator";
    return "unknown";
}

inline std::string tokenDisplayType(int tok, const std::string &declType) {
    if (tok == TOK_IDENTIFIER) {
        return declType.empty() ? std::string("IDENTIFIER") : declType;
    }
    std::string category = tokenCategory(tok);
    for (size_t i = 0; i < category.size(); i++) {
        category[i] = (char)toupper((unsigned char)category[i]);
    }
    return category;
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
        {"sizeof", TOK_SIZEOF}, {"static", TOK_STATIC}
    };
    return kw;
}

#endif 
