#ifndef TOKEN_TYPES_H
#define TOKEN_TYPES_H

#include <string>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Fine-grained token IDs.
//
// Assignment 1 only needs to print a coarse category (keyword/identifier/...)
// but the parser (Assignment 2, bison) needs one distinct terminal per
// keyword/operator. Defining both here now means the .l file never has to be
// rewritten later — only this enum grows, and lexer.l just returns members
// of it. Add new keywords/operators HERE, nowhere else.
//
// Each group ends with a _END sentinel. tokenCategory() tests against those
// sentinels instead of against the last real token, so adding a keyword or
// operator can never silently break the category ranges.
// ---------------------------------------------------------------------------
enum TokenID {
    // ----- literals / identifiers -----
    TOK_IDENTIFIER = 258,   // start above ASCII range, mirrors bison convention
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_CHAR_LITERAL,
    TOK_STRING_LITERAL,

    // ----- keywords -----
    TOK_INT, TOK_CHAR, TOK_FLOAT, TOK_DOUBLE, TOK_VOID, TOK_BOOL, TOK_AUTO,
    TOK_IF, TOK_ELSE, TOK_FOR, TOK_WHILE, TOK_DO, TOK_SWITCH, TOK_CASE,
    TOK_DEFAULT, TOK_BREAK, TOK_CONTINUE, TOK_GOTO, TOK_RETURN,
    TOK_STRUCT, TOK_CLASS, TOK_PUBLIC, TOK_PRIVATE, TOK_PROTECTED,
    TOK_NEW, TOK_DELETE, TOK_TRUE, TOK_FALSE, TOK_THIS, TOK_NULL, TOK_SIZEOF,
    TOK__KEYWORD_END,       // sentinel — not a real token

    // ----- operators -----
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    TOK_INC, TOK_DEC,
    TOK_ASSIGN, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_STAR_ASSIGN,
    TOK_SLASH_ASSIGN, TOK_PERCENT_ASSIGN,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_BITAND, TOK_BITOR, TOK_BITXOR, TOK_BITNOT, TOK_SHL, TOK_SHR,
    TOK_ARROW, TOK_SCOPE, TOK_DOT,
    TOK_QUESTION, TOK_COLON,
    TOK__OPERATOR_END,      // sentinel — not a real token

    // ----- punctuation -----
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET, TOK_SEMI, TOK_COMMA,
    TOK__PUNCTUATION_END    // sentinel — not a real token
};

// Coarse category string — this is what Assignment 1's output table wants
// ("keyword" / "identifier" / ...). Later stages ignore this and use
// TokenID directly, so this function only needs to live in one place.
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

// Single source of truth for reserved words. Add a keyword by adding one
// line here + one enum entry above — lexer.l reads this table, doesn't hard
// code strings.
//
// Deliberately NOT keywords, because they are not in our language spec:
//   static, typedef, enum, union
// Also deliberately NOT keywords: printf, scanf were previously identifiers
// but are now keywords in our language.
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

#endif // TOKEN_TYPES_H
