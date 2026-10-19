/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_IDENTIFIER = 258,
     TOK_INT_LITERAL = 259,
     TOK_FLOAT_LITERAL = 260,
     TOK_CHAR_LITERAL = 261,
     TOK_STRING_LITERAL = 262,
     TOK_INT = 263,
     TOK_CHAR = 264,
     TOK_FLOAT = 265,
     TOK_DOUBLE = 266,
     TOK_VOID = 267,
     TOK_BOOL = 268,
     TOK_AUTO = 269,
     TOK_DEFAULT = 270,
     TOK_BREAK = 271,
     TOK_CONTINUE = 272,
     TOK_GOTO = 273,
     TOK_RETURN = 274,
     TOK_IF = 275,
     TOK_ELSE = 276,
     TOK_FOR = 277,
     TOK_WHILE = 278,
     TOK_DO = 279,
     TOK_SWITCH = 280,
     TOK_CASE = 281,
     TOK_STRUCT = 282,
     TOK_CLASS = 283,
     TOK_PUBLIC = 284,
     TOK_PRIVATE = 285,
     TOK_PROTECTED = 286,
     TOK_NEW = 287,
     TOK_DELETE = 288,
     TOK_TRUE = 289,
     TOK_FALSE = 290,
     TOK_THIS = 291,
     TOK_NULL = 292,
     TOK_SIZEOF = 293,
     TOK_STATIC = 294,
     TOK__KEYWORD_END = 295,
     TOK_PLUS = 296,
     TOK_MINUS = 297,
     TOK_STAR = 298,
     TOK_SLASH = 299,
     TOK_PERCENT = 300,
     TOK_INC = 301,
     TOK_DEC = 302,
     TOK_ASSIGN = 303,
     TOK_PLUS_ASSIGN = 304,
     TOK_MINUS_ASSIGN = 305,
     TOK_STAR_ASSIGN = 306,
     TOK_SLASH_ASSIGN = 307,
     TOK_PERCENT_ASSIGN = 308,
     TOK_EQ = 309,
     TOK_NEQ = 310,
     TOK_LT = 311,
     TOK_GT = 312,
     TOK_LE = 313,
     TOK_GE = 314,
     TOK_AND = 315,
     TOK_OR = 316,
     TOK_NOT = 317,
     TOK_BITAND = 318,
     TOK_BITOR = 319,
     TOK_BITXOR = 320,
     TOK_BITNOT = 321,
     TOK_SHL = 322,
     TOK_SHR = 323,
     TOK_ARROW = 324,
     TOK_SCOPE = 325,
     TOK_DOT = 326,
     TOK_QUESTION = 327,
     TOK_COLON = 328,
     TOK__OPERATOR_END = 329,
     TOK_LPAREN = 330,
     TOK_RPAREN = 331,
     TOK_LBRACE = 332,
     TOK_RBRACE = 333,
     TOK_LBRACKET = 334,
     TOK_RBRACKET = 335,
     TOK_SEMI = 336,
     TOK_COMMA = 337,
     TOK__PUNCTUATION_END = 338,
     IDENTIFIER_EXPRESSION = 339,
     LOWER_THAN_ELSE = 340
   };
#endif
/* Tokens.  */
#define TOK_IDENTIFIER 258
#define TOK_INT_LITERAL 259
#define TOK_FLOAT_LITERAL 260
#define TOK_CHAR_LITERAL 261
#define TOK_STRING_LITERAL 262
#define TOK_INT 263
#define TOK_CHAR 264
#define TOK_FLOAT 265
#define TOK_DOUBLE 266
#define TOK_VOID 267
#define TOK_BOOL 268
#define TOK_AUTO 269
#define TOK_DEFAULT 270
#define TOK_BREAK 271
#define TOK_CONTINUE 272
#define TOK_GOTO 273
#define TOK_RETURN 274
#define TOK_IF 275
#define TOK_ELSE 276
#define TOK_FOR 277
#define TOK_WHILE 278
#define TOK_DO 279
#define TOK_SWITCH 280
#define TOK_CASE 281
#define TOK_STRUCT 282
#define TOK_CLASS 283
#define TOK_PUBLIC 284
#define TOK_PRIVATE 285
#define TOK_PROTECTED 286
#define TOK_NEW 287
#define TOK_DELETE 288
#define TOK_TRUE 289
#define TOK_FALSE 290
#define TOK_THIS 291
#define TOK_NULL 292
#define TOK_SIZEOF 293
#define TOK_STATIC 294
#define TOK__KEYWORD_END 295
#define TOK_PLUS 296
#define TOK_MINUS 297
#define TOK_STAR 298
#define TOK_SLASH 299
#define TOK_PERCENT 300
#define TOK_INC 301
#define TOK_DEC 302
#define TOK_ASSIGN 303
#define TOK_PLUS_ASSIGN 304
#define TOK_MINUS_ASSIGN 305
#define TOK_STAR_ASSIGN 306
#define TOK_SLASH_ASSIGN 307
#define TOK_PERCENT_ASSIGN 308
#define TOK_EQ 309
#define TOK_NEQ 310
#define TOK_LT 311
#define TOK_GT 312
#define TOK_LE 313
#define TOK_GE 314
#define TOK_AND 315
#define TOK_OR 316
#define TOK_NOT 317
#define TOK_BITAND 318
#define TOK_BITOR 319
#define TOK_BITXOR 320
#define TOK_BITNOT 321
#define TOK_SHL 322
#define TOK_SHR 323
#define TOK_ARROW 324
#define TOK_SCOPE 325
#define TOK_DOT 326
#define TOK_QUESTION 327
#define TOK_COLON 328
#define TOK__OPERATOR_END 329
#define TOK_LPAREN 330
#define TOK_RPAREN 331
#define TOK_LBRACE 332
#define TOK_RBRACE 333
#define TOK_LBRACKET 334
#define TOK_RBRACKET 335
#define TOK_SEMI 336
#define TOK_COMMA 337
#define TOK__PUNCTUATION_END 338
#define IDENTIFIER_EXPRESSION 339
#define LOWER_THAN_ELSE 340




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;
