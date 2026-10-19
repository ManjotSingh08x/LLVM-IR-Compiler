/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_BUILD_PARSER_TAB_H_INCLUDED
# define YY_YY_BUILD_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOK_IDENTIFIER = 258,          /* TOK_IDENTIFIER  */
    TOK_INT_LITERAL = 259,         /* TOK_INT_LITERAL  */
    TOK_FLOAT_LITERAL = 260,       /* TOK_FLOAT_LITERAL  */
    TOK_CHAR_LITERAL = 261,        /* TOK_CHAR_LITERAL  */
    TOK_STRING_LITERAL = 262,      /* TOK_STRING_LITERAL  */
    TOK_INT = 263,                 /* TOK_INT  */
    TOK_CHAR = 264,                /* TOK_CHAR  */
    TOK_FLOAT = 265,               /* TOK_FLOAT  */
    TOK_DOUBLE = 266,              /* TOK_DOUBLE  */
    TOK_VOID = 267,                /* TOK_VOID  */
    TOK_BOOL = 268,                /* TOK_BOOL  */
    TOK_AUTO = 269,                /* TOK_AUTO  */
    TOK_DEFAULT = 270,             /* TOK_DEFAULT  */
    TOK_BREAK = 271,               /* TOK_BREAK  */
    TOK_CONTINUE = 272,            /* TOK_CONTINUE  */
    TOK_GOTO = 273,                /* TOK_GOTO  */
    TOK_RETURN = 274,              /* TOK_RETURN  */
    TOK_IF = 275,                  /* TOK_IF  */
    TOK_ELSE = 276,                /* TOK_ELSE  */
    TOK_FOR = 277,                 /* TOK_FOR  */
    TOK_WHILE = 278,               /* TOK_WHILE  */
    TOK_DO = 279,                  /* TOK_DO  */
    TOK_SWITCH = 280,              /* TOK_SWITCH  */
    TOK_CASE = 281,                /* TOK_CASE  */
    TOK_STRUCT = 282,              /* TOK_STRUCT  */
    TOK_CLASS = 283,               /* TOK_CLASS  */
    TOK_PUBLIC = 284,              /* TOK_PUBLIC  */
    TOK_PRIVATE = 285,             /* TOK_PRIVATE  */
    TOK_PROTECTED = 286,           /* TOK_PROTECTED  */
    TOK_NEW = 287,                 /* TOK_NEW  */
    TOK_DELETE = 288,              /* TOK_DELETE  */
    TOK_TRUE = 289,                /* TOK_TRUE  */
    TOK_FALSE = 290,               /* TOK_FALSE  */
    TOK_THIS = 291,                /* TOK_THIS  */
    TOK_NULL = 292,                /* TOK_NULL  */
    TOK_SIZEOF = 293,              /* TOK_SIZEOF  */
    TOK__KEYWORD_END = 294,        /* TOK__KEYWORD_END  */
    TOK_PLUS = 295,                /* TOK_PLUS  */
    TOK_MINUS = 296,               /* TOK_MINUS  */
    TOK_STAR = 297,                /* TOK_STAR  */
    TOK_SLASH = 298,               /* TOK_SLASH  */
    TOK_PERCENT = 299,             /* TOK_PERCENT  */
    TOK_INC = 300,                 /* TOK_INC  */
    TOK_DEC = 301,                 /* TOK_DEC  */
    TOK_ASSIGN = 302,              /* TOK_ASSIGN  */
    TOK_PLUS_ASSIGN = 303,         /* TOK_PLUS_ASSIGN  */
    TOK_MINUS_ASSIGN = 304,        /* TOK_MINUS_ASSIGN  */
    TOK_STAR_ASSIGN = 305,         /* TOK_STAR_ASSIGN  */
    TOK_SLASH_ASSIGN = 306,        /* TOK_SLASH_ASSIGN  */
    TOK_PERCENT_ASSIGN = 307,      /* TOK_PERCENT_ASSIGN  */
    TOK_EQ = 308,                  /* TOK_EQ  */
    TOK_NEQ = 309,                 /* TOK_NEQ  */
    TOK_LT = 310,                  /* TOK_LT  */
    TOK_GT = 311,                  /* TOK_GT  */
    TOK_LE = 312,                  /* TOK_LE  */
    TOK_GE = 313,                  /* TOK_GE  */
    TOK_AND = 314,                 /* TOK_AND  */
    TOK_OR = 315,                  /* TOK_OR  */
    TOK_NOT = 316,                 /* TOK_NOT  */
    TOK_BITAND = 317,              /* TOK_BITAND  */
    TOK_BITOR = 318,               /* TOK_BITOR  */
    TOK_BITXOR = 319,              /* TOK_BITXOR  */
    TOK_BITNOT = 320,              /* TOK_BITNOT  */
    TOK_SHL = 321,                 /* TOK_SHL  */
    TOK_SHR = 322,                 /* TOK_SHR  */
    TOK_ARROW = 323,               /* TOK_ARROW  */
    TOK_SCOPE = 324,               /* TOK_SCOPE  */
    TOK_DOT = 325,                 /* TOK_DOT  */
    TOK_QUESTION = 326,            /* TOK_QUESTION  */
    TOK_COLON = 327,               /* TOK_COLON  */
    TOK__OPERATOR_END = 328,       /* TOK__OPERATOR_END  */
    TOK_LPAREN = 329,              /* TOK_LPAREN  */
    TOK_RPAREN = 330,              /* TOK_RPAREN  */
    TOK_LBRACE = 331,              /* TOK_LBRACE  */
    TOK_RBRACE = 332,              /* TOK_RBRACE  */
    TOK_LBRACKET = 333,            /* TOK_LBRACKET  */
    TOK_RBRACKET = 334,            /* TOK_RBRACKET  */
    TOK_SEMI = 335,                /* TOK_SEMI  */
    TOK_COMMA = 336,               /* TOK_COMMA  */
    TOK__PUNCTUATION_END = 337,    /* TOK__PUNCTUATION_END  */
    IDENTIFIER_EXPRESSION = 338,   /* IDENTIFIER_EXPRESSION  */
    LOWER_THAN_ELSE = 339          /* LOWER_THAN_ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;

int yyparse (void);


#endif /* !YY_YY_BUILD_PARSER_TAB_H_INCLUDED  */
