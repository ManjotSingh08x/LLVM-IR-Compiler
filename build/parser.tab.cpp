/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1



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




/* Copy the first part of user declarations.  */
#line 1 "src/parser.y"

#include <cstdio>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int yylex(void);
void yyerror(const char *message);

extern FILE *yyin;

size_t lexicalErrorCount();
void printTable();
void printErrors(const char *filename);

//structs
struct SyntaxError {
    std::string message;
    int         line;
    int         col;
};

//globals
static std::vector<SyntaxError> g_syntaxErrors;
static const char *g_filename = NULL;

void        setDeclType(int index, const std::string &type);
std::string lexemeAt(int index);

struct PendingName {
    int index;      
    int stars;      
    int amps;       
    int dims;       
};

static std::vector<PendingName> g_pendingNames;

static void pendName(int index, int stars, int amps, int dims) {
    PendingName p;
    p.index = index;
    p.stars = stars;
    p.amps  = amps;
    p.dims  = dims;
    g_pendingNames.push_back(p);
}

static void applyPendingType(const std::string &type) {
    for (size_t i = 0; i < g_pendingNames.size(); i++) {
        std::string full = type;
        if (g_pendingNames[i].stars > 0 || g_pendingNames[i].amps > 0
            || g_pendingNames[i].dims > 0) {
            full += "_CLASSSPECIFIER";
        }
        setDeclType(g_pendingNames[i].index, full);
    }
    g_pendingNames.clear();
}

static std::string typeNameAt(int index) {
    std::string name = lexemeAt(index);
    for (size_t i = 0; i < name.size(); i++) {
        name[i] = (char)toupper((unsigned char)name[i]);
    }
    return name;
}


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

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


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 359 "build/parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   697

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  81
/* YYNRULES -- Number of rules.  */
#define YYNRULES  240
/* YYNRULES -- Number of states.  */
#define YYNSTATES  453

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   340

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      19,    21,    24,    27,    30,    33,    41,    50,    58,    59,
      62,    70,    71,    74,    76,    80,    83,    85,    86,    89,
      92,    94,   100,   107,   113,   120,   122,   124,   126,   128,
     130,   132,   134,   136,   138,   140,   142,   145,   147,   149,
     151,   153,   155,   157,   159,   161,   163,   166,   168,   172,
     179,   186,   187,   189,   191,   195,   198,   199,   202,   206,
     210,   212,   220,   226,   232,   233,   235,   239,   242,   252,
     260,   262,   266,   270,   275,   280,   281,   283,   287,   292,
     293,   296,   298,   300,   302,   306,   307,   309,   312,   314,
     318,   321,   325,   327,   330,   334,   337,   339,   343,   347,
     352,   357,   362,   366,   367,   370,   372,   374,   377,   379,
     381,   383,   385,   387,   389,   392,   398,   406,   412,   418,
     426,   436,   437,   439,   441,   445,   448,   451,   455,   459,
     464,   468,   469,   471,   473,   475,   479,   481,   483,   485,
     487,   489,   491,   493,   499,   501,   505,   507,   511,   513,
     517,   519,   523,   525,   529,   531,   535,   539,   541,   545,
     549,   553,   557,   559,   563,   567,   569,   573,   577,   579,
     583,   587,   591,   593,   596,   599,   602,   605,   608,   611,
     614,   617,   621,   624,   629,   631,   633,   637,   641,   646,
     648,   650,   653,   654,   658,   660,   662,   665,   669,   674,
     676,   681,   686,   690,   694,   697,   700,   701,   703,   705,
     709,   711,   713,   715,   717,   719,   721,   723,   725,   727,
     731,   733,   740,   748,   756,   758,   760,   762,   766,   768,
     771
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      87,     0,    -1,    -1,    87,    88,    -1,    91,    -1,    93,
      -1,   106,    -1,   107,    -1,    90,    -1,   114,    -1,    89,
      -1,     1,    81,    -1,    39,   106,    -1,    39,   107,    -1,
      39,   114,    -1,     3,    70,     3,    75,   108,    76,   129,
      -1,     3,    70,    66,     3,    75,   108,    76,   129,    -1,
      27,     3,    94,    77,    92,    78,    81,    -1,    -1,    92,
     102,    -1,    28,     3,    94,    77,    97,    78,    81,    -1,
      -1,    73,    95,    -1,    96,    -1,    95,    82,    96,    -1,
     101,     3,    -1,     3,    -1,    -1,    97,    98,    -1,   101,
      73,    -1,   102,    -1,     3,    75,   108,    76,   129,    -1,
      66,     3,    75,   108,    76,   129,    -1,     3,    75,   108,
      76,    81,    -1,    66,     3,    75,   108,    76,    81,    -1,
      29,    -1,    30,    -1,    31,    -1,    91,    -1,    93,    -1,
     106,    -1,   107,    -1,    99,    -1,   100,    -1,   114,    -1,
      89,    -1,     1,    81,    -1,     8,    -1,     9,    -1,    10,
      -1,    11,    -1,    12,    -1,    13,    -1,    14,    -1,   103,
      -1,     3,    -1,    27,     3,    -1,     3,    -1,   105,    70,
       3,    -1,   104,   105,    75,   108,    76,   129,    -1,   104,
     105,    75,   108,    76,    81,    -1,    -1,   109,    -1,   110,
      -1,   109,    82,   110,    -1,   104,   111,    -1,    -1,     3,
     118,    -1,    43,     3,   118,    -1,    63,     3,   118,    -1,
     112,    -1,    75,    43,   113,    76,    75,   108,    76,    -1,
      75,    43,   113,    76,   119,    -1,    75,    63,   113,    76,
     119,    -1,    -1,     3,    -1,   104,   116,    81,    -1,   115,
      81,    -1,   103,    75,    43,     3,    76,    75,   108,    76,
     120,    -1,   103,    75,    43,     3,    76,   119,   120,    -1,
     117,    -1,   116,    82,   117,    -1,   105,   118,   120,    -1,
      43,   105,   118,   120,    -1,    63,   105,   118,   120,    -1,
      -1,   119,    -1,    79,   139,    80,    -1,   119,    79,   139,
      80,    -1,    -1,    48,   121,    -1,   122,    -1,   141,    -1,
     122,    -1,    77,   123,    78,    -1,    -1,   124,    -1,   124,
      82,    -1,   121,    -1,   124,    82,   121,    -1,   126,    81,
      -1,    39,   126,    81,    -1,   115,    -1,   103,   116,    -1,
      27,     3,   116,    -1,     3,   127,    -1,   128,    -1,   127,
      82,   128,    -1,     3,   118,   120,    -1,     3,    75,   160,
      76,    -1,    43,     3,   118,   120,    -1,    63,     3,   118,
     120,    -1,    77,   130,    78,    -1,    -1,   130,   131,    -1,
     125,    -1,   132,    -1,     1,    81,    -1,   129,    -1,   133,
      -1,   134,    -1,   135,    -1,   137,    -1,   138,    -1,   139,
      81,    -1,    20,    75,   140,    76,   132,    -1,    20,    75,
     140,    76,   132,    21,   132,    -1,    25,    75,   140,    76,
     132,    -1,    23,    75,   140,    76,   132,    -1,    24,   132,
      23,    75,   140,    76,    81,    -1,    22,    75,   136,    81,
     139,    81,   139,    76,   132,    -1,    -1,   140,    -1,   126,
      -1,    19,   139,    81,    -1,    16,    81,    -1,    17,    81,
      -1,    18,     3,    81,    -1,     3,    73,   132,    -1,    26,
     143,    73,   132,    -1,    15,    73,   132,    -1,    -1,   140,
      -1,   141,    -1,   143,    -1,   154,   142,   141,    -1,    48,
      -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,
      -1,   144,    -1,   144,    72,   140,    73,   143,    -1,   145,
      -1,   144,    61,   145,    -1,   146,    -1,   145,    60,   146,
      -1,   147,    -1,   146,    64,   147,    -1,   148,    -1,   147,
      65,   148,    -1,   149,    -1,   148,    63,   149,    -1,   150,
      -1,   149,    54,   150,    -1,   149,    55,   150,    -1,   151,
      -1,   150,    56,   151,    -1,   150,    57,   151,    -1,   150,
      58,   151,    -1,   150,    59,   151,    -1,   152,    -1,   151,
      67,   152,    -1,   151,    68,   152,    -1,   153,    -1,   152,
      41,   153,    -1,   152,    42,   153,    -1,   154,    -1,   153,
      43,   154,    -1,   153,    44,   154,    -1,   153,    45,   154,
      -1,   159,    -1,    46,   154,    -1,    47,   154,    -1,    41,
     154,    -1,    42,   154,    -1,    62,   154,    -1,    66,   154,
      -1,    43,   154,    -1,    63,   154,    -1,    32,   156,   157,
      -1,    33,   154,    -1,    33,    79,    80,   155,    -1,     3,
      -1,    36,    -1,   155,    71,     3,    -1,   155,    69,     3,
      -1,   155,    79,   140,    80,    -1,   103,    -1,     3,    -1,
      27,     3,    -1,    -1,    75,   160,    76,    -1,   122,    -1,
     158,    -1,   158,   122,    -1,    79,   140,    80,    -1,   158,
      79,   140,    80,    -1,   162,    -1,   159,    79,   140,    80,
      -1,   159,    75,   160,    76,    -1,   159,    71,     3,    -1,
     159,    69,     3,    -1,   159,    46,    -1,   159,    47,    -1,
      -1,   161,    -1,   141,    -1,   161,    82,   141,    -1,     3,
      -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,    34,
      -1,    35,    -1,    37,    -1,    36,    -1,    75,   140,    76,
      -1,   163,    -1,    79,    80,    75,   108,    76,   129,    -1,
      79,   164,    80,    75,   108,    76,   129,    -1,    79,   165,
      80,    75,   108,    76,   129,    -1,    48,    -1,    63,    -1,
     166,    -1,   165,    82,   166,    -1,     3,    -1,    63,     3,
      -1,    36,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   110,   110,   112,   116,   117,   118,   119,   120,   121,
     122,   123,   128,   129,   130,   135,   138,   144,   148,   150,
     154,   160,   162,   166,   167,   171,   172,   175,   177,   181,
     182,   187,   189,   194,   196,   201,   202,   203,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   220,   221,   222,
     223,   224,   225,   226,   230,   231,   232,   236,   237,   241,
     246,   250,   252,   256,   257,   261,   264,   266,   267,   268,
     269,   274,   276,   278,   283,   284,   288,   290,   295,   298,
     305,   306,   310,   312,   314,   319,   320,   324,   325,   330,
     332,   333,   338,   339,   343,   346,   348,   349,   353,   354,
     359,   360,   364,   365,   366,   367,   371,   372,   380,   382,
     384,   386,   392,   395,   397,   401,   402,   403,   407,   408,
     409,   410,   411,   412,   416,   420,   421,   422,   426,   427,
     428,   432,   434,   435,   439,   440,   441,   442,   446,   447,
     448,   452,   454,   458,   462,   463,   467,   468,   469,   470,
     471,   472,   476,   477,   481,   482,   486,   487,   491,   492,
     496,   497,   501,   502,   506,   507,   508,   512,   513,   514,
     515,   516,   520,   521,   522,   526,   527,   528,   532,   533,
     534,   535,   539,   540,   541,   542,   543,   544,   545,   546,
     547,   548,   549,   550,   555,   556,   557,   558,   559,   563,
     564,   565,   568,   570,   571,   572,   573,   578,   579,   583,
     584,   585,   586,   587,   588,   589,   592,   594,   598,   599,
     603,   604,   605,   606,   607,   608,   609,   610,   611,   612,
     613,   617,   618,   619,   623,   624,   628,   629,   633,   634,
     635
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENTIFIER", "TOK_INT_LITERAL",
  "TOK_FLOAT_LITERAL", "TOK_CHAR_LITERAL", "TOK_STRING_LITERAL", "TOK_INT",
  "TOK_CHAR", "TOK_FLOAT", "TOK_DOUBLE", "TOK_VOID", "TOK_BOOL",
  "TOK_AUTO", "TOK_DEFAULT", "TOK_BREAK", "TOK_CONTINUE", "TOK_GOTO",
  "TOK_RETURN", "TOK_IF", "TOK_ELSE", "TOK_FOR", "TOK_WHILE", "TOK_DO",
  "TOK_SWITCH", "TOK_CASE", "TOK_STRUCT", "TOK_CLASS", "TOK_PUBLIC",
  "TOK_PRIVATE", "TOK_PROTECTED", "TOK_NEW", "TOK_DELETE", "TOK_TRUE",
  "TOK_FALSE", "TOK_THIS", "TOK_NULL", "TOK_SIZEOF", "TOK_STATIC",
  "TOK__KEYWORD_END", "TOK_PLUS", "TOK_MINUS", "TOK_STAR", "TOK_SLASH",
  "TOK_PERCENT", "TOK_INC", "TOK_DEC", "TOK_ASSIGN", "TOK_PLUS_ASSIGN",
  "TOK_MINUS_ASSIGN", "TOK_STAR_ASSIGN", "TOK_SLASH_ASSIGN",
  "TOK_PERCENT_ASSIGN", "TOK_EQ", "TOK_NEQ", "TOK_LT", "TOK_GT", "TOK_LE",
  "TOK_GE", "TOK_AND", "TOK_OR", "TOK_NOT", "TOK_BITAND", "TOK_BITOR",
  "TOK_BITXOR", "TOK_BITNOT", "TOK_SHL", "TOK_SHR", "TOK_ARROW",
  "TOK_SCOPE", "TOK_DOT", "TOK_QUESTION", "TOK_COLON", "TOK__OPERATOR_END",
  "TOK_LPAREN", "TOK_RPAREN", "TOK_LBRACE", "TOK_RBRACE", "TOK_LBRACKET",
  "TOK_RBRACKET", "TOK_SEMI", "TOK_COMMA", "TOK__PUNCTUATION_END",
  "IDENTIFIER_EXPRESSION", "LOWER_THAN_ELSE", "$accept",
  "translation_unit", "external_item", "static_item",
  "qualified_special_member_definition", "record_definition",
  "record_items_opt", "class_definition", "inheritance_opt", "base_list",
  "base_specifier", "class_items_opt", "class_item",
  "special_member_definition", "special_member_declaration",
  "access_specifier", "member_item", "simple_type", "type_specifier",
  "qualified_name", "function_definition", "function_declaration",
  "parameter_list_opt", "parameter_list", "parameter_declaration",
  "parameter_declarator_opt", "parenthesized_parameter_declarator",
  "parameter_name_opt", "declaration", "parenthesized_declarator",
  "init_declarator_list", "init_declarator", "array_dimensions_opt",
  "array_dimensions", "initializer_opt", "initializer", "braced_init_list",
  "initializer_list_opt", "initializer_list", "local_declaration",
  "local_declaration_body", "named_type_declarator_list",
  "named_type_declarator", "compound_statement", "block_items_opt",
  "block_item", "statement", "expression_statement", "selection_statement",
  "iteration_statement", "for_initializer", "jump_statement",
  "labeled_statement", "expression_opt", "expression",
  "assignment_expression", "assignment_operator", "conditional_expression",
  "logical_or_expression", "logical_and_expression",
  "bitwise_or_expression", "bitwise_xor_expression",
  "bitwise_and_expression", "equality_expression", "relational_expression",
  "shift_expression", "additive_expression", "multiplicative_expression",
  "unary_expression", "delete_operand", "allocation_type",
  "allocation_suffix_opt", "allocation_dimensions", "postfix_expression",
  "argument_list_opt", "argument_list", "primary_expression",
  "lambda_expression", "capture_default", "capture_list", "capture", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    86,    87,    87,    88,    88,    88,    88,    88,    88,
      88,    88,    89,    89,    89,    90,    90,    91,    92,    92,
      93,    94,    94,    95,    95,    96,    96,    97,    97,    98,
      98,    99,    99,   100,   100,   101,   101,   101,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   103,   103,   103,
     103,   103,   103,   103,   104,   104,   104,   105,   105,   106,
     107,   108,   108,   109,   109,   110,   111,   111,   111,   111,
     111,   112,   112,   112,   113,   113,   114,   114,   115,   115,
     116,   116,   117,   117,   117,   118,   118,   119,   119,   120,
     120,   120,   121,   121,   122,   123,   123,   123,   124,   124,
     125,   125,   126,   126,   126,   126,   127,   127,   128,   128,
     128,   128,   129,   130,   130,   131,   131,   131,   132,   132,
     132,   132,   132,   132,   133,   134,   134,   134,   135,   135,
     135,   136,   136,   136,   137,   137,   137,   137,   138,   138,
     138,   139,   139,   140,   141,   141,   142,   142,   142,   142,
     142,   142,   143,   143,   144,   144,   145,   145,   146,   146,
     147,   147,   148,   148,   149,   149,   149,   150,   150,   150,
     150,   150,   151,   151,   151,   152,   152,   152,   153,   153,
     153,   153,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   155,   155,   155,   155,   155,   156,
     156,   156,   157,   157,   157,   157,   157,   158,   158,   159,
     159,   159,   159,   159,   159,   159,   160,   160,   161,   161,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
     162,   163,   163,   163,   164,   164,   165,   165,   166,   166,
     166
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     7,     8,     7,     0,     2,
       7,     0,     2,     1,     3,     2,     1,     0,     2,     2,
       1,     5,     6,     5,     6,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     3,     6,
       6,     0,     1,     1,     3,     2,     0,     2,     3,     3,
       1,     7,     5,     5,     0,     1,     3,     2,     9,     7,
       1,     3,     3,     4,     4,     0,     1,     3,     4,     0,
       2,     1,     1,     1,     3,     0,     1,     2,     1,     3,
       2,     3,     1,     2,     3,     2,     1,     3,     3,     4,
       4,     4,     3,     0,     2,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     5,     7,     5,     5,     7,
       9,     0,     1,     1,     3,     2,     2,     3,     3,     4,
       3,     0,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     5,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     1,     3,
       3,     3,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     3,     2,     4,     1,     1,     3,     3,     4,     1,
       1,     2,     0,     3,     1,     1,     2,     3,     4,     1,
       4,     4,     3,     3,     2,     2,     0,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     6,     7,     7,     1,     1,     1,     3,     1,     2,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,    55,    47,    48,    49,    50,    51,
      52,    53,     0,     0,     0,     3,    10,     8,     4,     5,
      54,     0,     6,     7,     9,     0,    11,     0,    56,    21,
      55,     0,    12,    13,    14,     0,    57,     0,     0,    85,
       0,    80,    77,     0,     0,     0,     0,     0,    56,     0,
      85,    85,     0,    61,   141,    89,    86,    76,     0,    61,
       0,    26,    35,    36,    37,    22,    23,     0,    18,    27,
       0,    89,    89,    58,    54,    66,     0,    62,    63,   220,
     221,   222,   223,   224,     0,     0,   225,   226,   228,   227,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   142,   143,   144,   152,   154,   156,   158,   160,   162,
     164,   167,   172,   175,   178,   182,   209,   230,     0,    95,
      82,    91,   141,    85,    81,     0,    61,     0,    25,     0,
       0,     0,    83,    84,    85,     0,     0,     0,    65,    70,
       0,     0,   200,     0,   199,   202,     0,   192,   185,   186,
     189,   183,   184,   187,   190,   188,     0,   238,   240,   234,
     235,     0,     0,     0,   236,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   146,   147,   148,   149,   150,
     151,     0,   214,   215,     0,     0,   216,     0,    90,    93,
      92,    98,     0,    96,     0,     0,     0,    24,     0,    55,
       0,     0,    45,    38,    39,    42,    43,    19,    40,    41,
      44,     0,    28,     0,    30,    61,    89,    67,    85,    85,
      74,    74,   113,    60,    59,    64,   201,   216,     0,   204,
     191,   205,     0,   229,   239,    61,     0,     0,     0,   155,
     178,     0,   157,   159,   161,   163,   165,   166,   168,   169,
     170,   171,   173,   174,   176,   177,   179,   180,   181,   145,
     213,   212,   218,     0,   217,     0,    94,    97,    88,    15,
       0,    46,    61,     0,    17,    20,    29,     0,    79,    68,
      69,    75,     0,     0,     0,     0,     0,     0,   206,   194,
     195,   193,     0,    61,    61,     0,   237,     0,   211,     0,
     210,    99,    16,     0,    61,    89,     0,     0,     0,   220,
       0,     0,     0,     0,   141,     0,     0,     0,   141,     0,
       0,     0,     0,   112,     0,   102,   115,     0,   118,   114,
     116,   119,   120,   121,   122,   123,     0,   203,   207,     0,
       0,     0,     0,     0,     0,     0,   153,   219,     0,     0,
      78,    61,    72,    73,   117,    85,     0,     0,   141,   105,
     106,   141,   135,   136,     0,     0,     0,   131,     0,   220,
       0,     0,     0,     0,     0,     0,   103,   100,   124,   208,
     197,   196,     0,   231,     0,     0,    33,    31,     0,     0,
     216,    89,    85,    85,   138,     0,   140,   137,   134,     0,
     220,   133,     0,   132,     0,     0,     0,   141,   104,   101,
     198,   232,   233,    34,    32,    71,     0,   108,    89,    89,
     107,   141,   141,   141,     0,   141,   139,   109,   110,   111,
     125,     0,   128,     0,   127,   141,   141,     0,   126,     0,
     129,   141,   130
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    15,   212,    17,   213,   129,   214,    46,    65,
      66,   130,   222,   215,   216,    67,   217,    74,    75,   123,
     218,   219,    76,    77,    78,   138,   139,   292,   220,    25,
      40,    41,    55,    56,   120,   198,   121,   202,   203,   336,
     337,   369,   370,   338,   294,   339,   340,   341,   342,   343,
     412,   344,   345,   346,   101,   102,   191,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   301,
     145,   240,   241,   115,   273,   274,   116,   117,   162,   163,
     164
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -308
static const yytype_int16 yypact[] =
{
    -308,   336,  -308,   -46,    -9,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,   100,   118,   359,  -308,  -308,  -308,  -308,  -308,
       1,    34,  -308,  -308,  -308,    51,  -308,    21,   -23,    81,
    -308,   143,  -308,  -308,  -308,   120,  -308,   165,   165,   112,
      96,  -308,  -308,    95,   189,   130,   131,   136,  -308,   192,
      52,    52,   226,   359,   600,    -4,   163,  -308,    34,   359,
     169,  -308,  -308,  -308,  -308,   164,  -308,   247,  -308,  -308,
     179,    -4,    -4,  -308,  -308,    17,   181,   176,  -308,  -308,
    -308,  -308,  -308,  -308,   431,   618,  -308,  -308,  -308,  -308,
     600,   600,   600,   600,   600,   600,   600,   600,   600,    15,
     180,  -308,  -308,  -308,   -13,   199,   203,   196,   200,   177,
     146,   154,   197,   167,   225,   110,  -308,  -308,   552,   552,
    -308,  -308,   600,    52,  -308,   193,   359,   130,  -308,   288,
     206,    60,  -308,  -308,   204,   268,   276,     3,  -308,  -308,
      66,   359,  -308,   283,  -308,    87,    16,  -308,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,   211,  -308,  -308,  -308,
     285,   217,   213,   -33,  -308,  -308,   600,   600,   600,   600,
     600,   600,   600,   600,   600,   600,   600,   600,   600,   600,
     600,   600,   600,   600,   600,  -308,  -308,  -308,  -308,  -308,
    -308,   600,  -308,  -308,   292,   300,   600,   600,  -308,  -308,
    -308,  -308,   227,   222,   228,   229,   231,  -308,   230,   235,
     309,   232,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
    -308,   233,  -308,   244,  -308,   359,   -10,  -308,   204,   204,
     315,   315,  -308,  -308,  -308,  -308,  -308,   600,   600,  -308,
    -308,    14,    62,  -308,  -308,   359,   246,   248,    36,   199,
    -308,   249,   203,   196,   200,   177,   146,   146,   154,   154,
     154,   154,   197,   197,   167,   167,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,   243,   242,   245,  -308,   552,  -308,  -308,
     229,  -308,   359,   251,  -308,  -308,  -308,   252,  -308,  -308,
    -308,  -308,   253,   254,   390,   255,   260,   600,  -308,  -308,
    -308,    65,   258,   359,   359,   285,  -308,   600,  -308,   600,
    -308,  -308,  -308,   259,   359,    -4,    74,   204,   261,    82,
     270,   271,   272,   338,   600,   280,   281,   284,   457,   290,
     600,   348,   498,  -308,    19,  -308,  -308,   293,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,   295,  -308,  -308,   299,
     377,   378,   600,   229,   306,   307,  -308,  -308,    88,   308,
    -308,   359,   163,   163,  -308,   101,   382,   384,   457,   329,
    -308,   457,  -308,  -308,   337,   339,   600,   534,   600,   316,
     365,   600,   317,    34,    85,   340,   346,  -308,  -308,  -308,
    -308,  -308,   350,  -308,   229,   229,  -308,  -308,   117,   343,
     600,    -4,   204,   204,  -308,    85,  -308,  -308,  -308,   362,
      85,  -308,   354,  -308,   370,   372,   373,   457,   346,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,   374,  -308,    -4,    -4,
    -308,   457,   600,   457,   600,   457,  -308,  -308,  -308,  -308,
     427,   376,  -308,   375,  -308,   457,   600,   385,  -308,   379,
    -308,   457,  -308
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -308,  -308,  -308,   453,  -308,   458,  -308,   469,   449,  -308,
     357,  -308,  -308,  -308,  -308,   355,   356,     0,    22,   103,
      39,    42,   -52,  -308,   347,  -308,  -308,   256,    44,  -277,
    -293,   437,   -45,  -120,   -68,  -106,  -103,  -308,  -308,  -308,
    -307,  -308,    91,  -130,  -308,  -308,  -245,  -308,  -308,  -308,
    -308,  -308,  -308,   -54,   -96,  -110,  -308,  -273,  -308,   331,
     334,   344,   335,   345,    68,    50,    70,    73,   -64,  -308,
    -308,  -308,  -308,  -308,  -225,  -308,  -308,  -308,  -308,  -308,
     266
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -142
static const yytype_int16 yytable[] =
{
     100,    20,   156,   132,   133,    71,    72,   125,   200,   200,
     234,   226,   295,   201,    20,   199,   199,   335,   157,   157,
     134,   147,    36,    21,    43,   385,   148,   149,   150,   151,
     152,   153,   154,   155,   356,    26,    21,    36,   118,   157,
      22,   386,   239,    23,   118,    24,   230,   247,   166,   248,
      45,   158,   158,    32,   -21,   335,    33,   382,    34,   167,
     135,    27,    37,   159,   159,   299,   231,   119,   204,   122,
     411,   251,   158,   119,   206,   279,    35,    37,   160,   160,
     136,   269,    38,   380,   144,   365,   272,    44,   365,   227,
     418,   119,   137,   297,    35,   161,   242,    38,   300,   305,
     335,   275,   250,    28,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   266,   267,
     268,    29,    52,   404,    39,   366,   406,   272,   366,    20,
      20,    54,    42,    61,   350,   225,   351,   245,   298,    54,
      50,    51,   296,   232,   352,   367,    48,   233,   367,   361,
     312,    21,    21,    54,    45,   368,   192,   193,   288,    62,
      63,    64,   237,    49,   119,   232,   238,   200,    36,   396,
      59,   311,   436,   287,   199,   426,   400,    57,    58,   194,
      54,   195,    52,   289,   290,   196,   440,    53,   442,   197,
     444,    54,    60,   302,   232,    70,   362,   363,   423,   357,
     448,   349,   174,   175,   176,   177,   452,   208,    68,   209,
     182,   183,   184,    69,     5,     6,     7,     8,     9,    10,
      11,   178,   179,   393,   258,   259,   260,   261,   397,    73,
     313,   172,   173,    12,    13,    62,    63,    64,   180,   181,
     256,   257,   122,   250,   126,    14,   127,   360,   262,   263,
     128,   354,   355,   264,   265,   131,   392,   140,   141,   168,
     165,   170,   359,   171,   421,   422,   250,   169,   424,   205,
     375,   228,   210,   185,   186,   187,   188,   189,   190,   229,
     409,   413,   414,    54,   221,   416,   236,   243,   244,   208,
     272,   209,   245,   246,   334,   270,     5,     6,     7,     8,
       9,    10,    11,   271,   277,   276,   232,   280,   278,   399,
     282,   281,   283,   284,   285,    12,    13,   286,   291,   308,
     401,   303,   307,   304,   309,   310,   314,    14,   315,   316,
     317,   347,   334,   427,   353,   358,     2,     3,   443,     4,
     348,   374,   364,   371,     5,     6,     7,     8,     9,    10,
      11,   383,   372,   373,   210,   376,   377,   428,   429,   378,
     438,   439,    30,    12,    13,   381,   211,     5,     6,     7,
       8,     9,    10,    11,   387,    14,   388,   334,   441,   389,
     390,   391,   394,   395,   398,   402,    31,   403,   415,   368,
     417,   318,   449,   319,    80,    81,    82,    83,     5,     6,
       7,     8,     9,    10,    11,   320,   321,   322,   323,   324,
     325,   405,   326,   327,   328,   329,   330,   331,   407,   425,
     408,   419,    84,    85,    86,    87,    88,    89,    58,   332,
     420,    90,    91,    92,   142,   432,    93,    94,   431,     5,
       6,     7,     8,     9,    10,    11,   433,   434,   445,   435,
     437,   447,    95,    96,    16,   451,    97,   446,   143,    18,
     379,    80,    81,    82,    83,    98,   450,   232,   333,    99,
      19,  -141,   320,   321,   322,   323,   324,   325,    47,   326,
     327,   328,   329,   330,   207,   223,   224,   293,   235,    84,
      85,    86,    87,    88,    89,   124,   430,   249,    90,    91,
      92,   384,   252,    93,    94,   254,     5,     6,     7,     8,
       9,    10,    11,   253,   306,     0,   255,     0,     0,    95,
      96,     0,     0,    97,     0,   331,     0,     0,     0,     0,
       0,     0,    98,     0,   232,     0,    99,   410,    80,    81,
      82,    83,     5,     6,     7,     8,     9,    10,    11,     0,
       0,     0,     0,     0,     0,    79,    80,    81,    82,    83,
       0,   331,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,     0,     0,     0,    90,    91,    92,     0,     0,
      93,    94,     0,     0,    84,    85,    86,    87,    88,    89,
       0,     0,     0,    90,    91,    92,    95,    96,    93,    94,
      97,     0,     0,    79,    80,    81,    82,    83,     0,    98,
       0,     0,     0,    99,    95,    96,     0,     0,    97,     0,
       0,    79,    80,    81,    82,    83,     0,    98,     0,   119,
       0,    99,    84,    85,    86,    87,    88,    89,     0,     0,
       0,    90,    91,    92,     0,     0,    93,    94,     0,     0,
      84,    85,    86,    87,    88,    89,     0,     0,     0,    90,
      91,    92,    95,    96,    93,    94,    97,     0,     0,     0,
       0,     0,     0,     0,     0,    98,     0,     0,     0,    99,
      95,    96,     0,     0,    97,     0,     0,     0,     0,     0,
       0,     0,     0,    98,     0,     0,     0,   146
};

static const yytype_int16 yycheck[] =
{
      54,     1,    98,    71,    72,    50,    51,    59,   118,   119,
     140,   131,   237,   119,    14,   118,   119,   294,     3,     3,
       3,    85,     3,     1,     3,   332,    90,    91,    92,    93,
      94,    95,    96,    97,   307,    81,    14,     3,    48,     3,
       1,   334,   145,     1,    48,     1,    43,    80,    61,    82,
      73,    36,    36,    14,    77,   332,    14,   330,    14,    72,
      43,    70,    43,    48,    48,     3,    63,    77,   122,    79,
     377,   167,    36,    77,   126,   205,    75,    43,    63,    63,
      63,   191,    63,   328,    84,     3,   196,    66,     3,   134,
     383,    77,    75,    79,    75,    80,    80,    63,    36,    63,
     377,   197,   166,     3,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,     3,    70,   368,    21,    43,   371,   237,    43,   129,
     130,    79,    81,     3,    69,    75,    71,    75,   241,    79,
      37,    38,   238,    77,    79,    63,     3,    81,    63,    75,
     280,   129,   130,    79,    73,    73,    46,    47,   226,    29,
      30,    31,    75,    43,    77,    77,    79,   277,     3,    81,
      75,   277,   417,   225,   277,   400,    75,    81,    82,    69,
      79,    71,    70,   228,   229,    75,   431,    75,   433,    79,
     435,    79,     3,   245,    77,     3,   316,   317,    81,   309,
     445,   297,    56,    57,    58,    59,   451,     1,    77,     3,
      43,    44,    45,    77,     8,     9,    10,    11,    12,    13,
      14,    67,    68,   353,   174,   175,   176,   177,   358,     3,
     282,    54,    55,    27,    28,    29,    30,    31,    41,    42,
     172,   173,    79,   307,    75,    39,    82,   315,   178,   179,
       3,   303,   304,   180,   181,    76,   352,    76,    82,    60,
      80,    65,   314,    63,   394,   395,   330,    64,   398,    76,
     324,     3,    66,    48,    49,    50,    51,    52,    53,     3,
     376,   377,   378,    79,    78,   381,     3,    76,     3,     1,
     400,     3,    75,    80,   294,     3,     8,     9,    10,    11,
      12,    13,    14,     3,    82,    78,    77,    76,    80,   361,
      75,    81,     3,    81,    81,    27,    28,    73,     3,    76,
     365,    75,    73,    75,    82,    80,    75,    39,    76,    76,
      76,    76,   332,   401,    76,    76,     0,     1,   434,     3,
      80,     3,    81,    73,     8,     9,    10,    11,    12,    13,
      14,     3,    81,    81,    66,    75,    75,   402,   403,    75,
     428,   429,     3,    27,    28,    75,    78,     8,     9,    10,
      11,    12,    13,    14,    81,    39,    81,   377,   432,    80,
       3,     3,    76,    76,    76,     3,    27,     3,    23,    73,
      73,     1,   446,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    82,    22,    23,    24,    25,    26,    27,    81,    76,
      81,    81,    32,    33,    34,    35,    36,    37,    82,    39,
      80,    41,    42,    43,     3,    81,    46,    47,    76,     8,
       9,    10,    11,    12,    13,    14,    76,    75,    21,    76,
      76,    76,    62,    63,     1,    76,    66,    81,    27,     1,
       3,     4,     5,     6,     7,    75,    81,    77,    78,    79,
       1,    81,    15,    16,    17,    18,    19,    20,    29,    22,
      23,    24,    25,    26,   127,   130,   130,   231,   141,    32,
      33,    34,    35,    36,    37,    58,   405,   166,    41,    42,
      43,     3,   168,    46,    47,   170,     8,     9,    10,    11,
      12,    13,    14,   169,   248,    -1,   171,    -1,    -1,    62,
      63,    -1,    -1,    66,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    75,    -1,    77,    -1,    79,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    27,    -1,    -1,    -1,    -1,    32,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      46,    47,    -1,    -1,    32,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    62,    63,    46,    47,
      66,    -1,    -1,     3,     4,     5,     6,     7,    -1,    75,
      -1,    -1,    -1,    79,    62,    63,    -1,    -1,    66,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    75,    -1,    77,
      -1,    79,    32,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    46,    47,    -1,    -1,
      32,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    43,    62,    63,    46,    47,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    79,
      62,    63,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    75,    -1,    -1,    -1,    79
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    87,     0,     1,     3,     8,     9,    10,    11,    12,
      13,    14,    27,    28,    39,    88,    89,    90,    91,    93,
     103,   104,   106,   107,   114,   115,    81,    70,     3,     3,
       3,    27,   106,   107,   114,    75,     3,    43,    63,   105,
     116,   117,    81,     3,    66,    73,    94,    94,     3,    43,
     105,   105,    70,    75,    79,   118,   119,    81,    82,    75,
       3,     3,    29,    30,    31,    95,    96,   101,    77,    77,
       3,   118,   118,     3,   103,   104,   108,   109,   110,     3,
       4,     5,     6,     7,    32,    33,    34,    35,    36,    37,
      41,    42,    43,    46,    47,    62,    63,    66,    75,    79,
     139,   140,   141,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   159,   162,   163,    48,    77,
     120,   122,    79,   105,   117,   108,    75,    82,     3,    92,
      97,    76,   120,   120,     3,    43,    63,    75,   111,   112,
      76,    82,     3,    27,   103,   156,    79,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   140,     3,    36,    48,
      63,    80,   164,   165,   166,    80,    61,    72,    60,    64,
      65,    63,    54,    55,    56,    57,    58,    59,    67,    68,
      41,    42,    43,    44,    45,    48,    49,    50,    51,    52,
      53,   142,    46,    47,    69,    71,    75,    79,   121,   122,
     141,   121,   123,   124,   139,    76,   108,    96,     1,     3,
      66,    78,    89,    91,    93,    99,   100,   102,   106,   107,
     114,    78,    98,   101,   102,    75,   119,   118,     3,     3,
      43,    63,    77,    81,   129,   110,     3,    75,    79,   122,
     157,   158,    80,    76,     3,    75,    80,    80,    82,   145,
     154,   140,   146,   147,   148,   149,   150,   150,   151,   151,
     151,   151,   152,   152,   153,   153,   154,   154,   154,   141,
       3,     3,   141,   160,   161,   140,    78,    82,    80,   129,
      76,    81,    75,     3,    81,    81,    73,   108,   120,   118,
     118,     3,   113,   113,   130,   160,   140,    79,   122,     3,
      36,   155,   108,    75,    75,    63,   166,    73,    76,    82,
      80,   121,   129,   108,    75,    76,    76,    76,     1,     3,
      15,    16,    17,    18,    19,    20,    22,    23,    24,    25,
      26,    27,    39,    78,   103,   115,   125,   126,   129,   131,
     132,   133,   134,   135,   137,   138,   139,    76,    80,   140,
      69,    71,    79,    76,   108,   108,   143,   141,    76,   108,
     120,    75,   119,   119,    81,     3,    43,    63,    73,   127,
     128,    73,    81,    81,     3,   139,    75,    75,    75,     3,
     132,    75,   143,     3,     3,   126,   116,    81,    81,    80,
       3,     3,   140,   129,    76,    76,    81,   129,    76,   108,
      75,   118,     3,     3,   132,    82,   132,    81,    81,   140,
       3,   126,   136,   140,   140,    23,   140,    73,   116,    81,
      80,   129,   129,    81,   129,    76,   160,   120,   118,   118,
     128,    76,    81,    76,    75,    76,   132,    76,   120,   120,
     132,   139,   132,   140,   132,    21,    81,    76,   132,   139,
      81,    76,   132
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 11:
#line 123 "src/parser.y"
    { yyerrok; ;}
    break;

  case 15:
#line 137 "src/parser.y"
    { setDeclType((yyvsp[(3) - (7)]), "PROCEDURE"); ;}
    break;

  case 16:
#line 140 "src/parser.y"
    { setDeclType((yyvsp[(4) - (8)]), "PROCEDURE"); ;}
    break;

  case 17:
#line 145 "src/parser.y"
    { setDeclType((yyvsp[(2) - (7)]), "STRUCT"); ;}
    break;

  case 20:
#line 155 "src/parser.y"
    { setDeclType((yyvsp[(2) - (7)]), "CLASS"); ;}
    break;

  case 31:
#line 188 "src/parser.y"
    { setDeclType((yyvsp[(1) - (5)]), "PROCEDURE"); ;}
    break;

  case 32:
#line 190 "src/parser.y"
    { setDeclType((yyvsp[(2) - (6)]), "PROCEDURE"); ;}
    break;

  case 33:
#line 195 "src/parser.y"
    { setDeclType((yyvsp[(1) - (5)]), "PROCEDURE"); ;}
    break;

  case 34:
#line 197 "src/parser.y"
    { setDeclType((yyvsp[(2) - (6)]), "PROCEDURE"); ;}
    break;

  case 46:
#line 215 "src/parser.y"
    { yyerrok; ;}
    break;

  case 54:
#line 230 "src/parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 55:
#line 231 "src/parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 56:
#line 232 "src/parser.y"
    { (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 57:
#line 236 "src/parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 58:
#line 237 "src/parser.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 59:
#line 242 "src/parser.y"
    { setDeclType((yyvsp[(2) - (6)]), "PROCEDURE"); ;}
    break;

  case 60:
#line 247 "src/parser.y"
    { setDeclType((yyvsp[(2) - (6)]), "PROCEDURE"); ;}
    break;

  case 65:
#line 261 "src/parser.y"
    { applyPendingType(typeNameAt((yyvsp[(1) - (2)]))); ;}
    break;

  case 67:
#line 266 "src/parser.y"
    { pendName((yyvsp[(1) - (2)]), 0, 0, (yyvsp[(2) - (2)])); ;}
    break;

  case 68:
#line 267 "src/parser.y"
    { pendName((yyvsp[(2) - (3)]), 1, 0, (yyvsp[(3) - (3)])); ;}
    break;

  case 69:
#line 268 "src/parser.y"
    { pendName((yyvsp[(2) - (3)]), 0, 1, (yyvsp[(3) - (3)])); ;}
    break;

  case 71:
#line 275 "src/parser.y"
    { setDeclType((yyvsp[(3) - (7)]), "PROCEDURE"); ;}
    break;

  case 72:
#line 277 "src/parser.y"
    { pendName((yyvsp[(3) - (5)]), 1, 0, (yyvsp[(5) - (5)])); ;}
    break;

  case 73:
#line 279 "src/parser.y"
    { pendName((yyvsp[(3) - (5)]), 0, 1, (yyvsp[(5) - (5)])); ;}
    break;

  case 74:
#line 283 "src/parser.y"
    { (yyval) = -1; ;}
    break;

  case 75:
#line 284 "src/parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 76:
#line 289 "src/parser.y"
    { applyPendingType(typeNameAt((yyvsp[(1) - (3)]))); ;}
    break;

  case 78:
#line 297 "src/parser.y"
    { setDeclType((yyvsp[(4) - (9)]), "PROCEDURE"); ;}
    break;

  case 79:
#line 300 "src/parser.y"
    { pendName((yyvsp[(4) - (7)]), 1, 0, (yyvsp[(6) - (7)]));
                                                        applyPendingType(typeNameAt((yyvsp[(1) - (7)]))); ;}
    break;

  case 82:
#line 311 "src/parser.y"
    { pendName((yyvsp[(1) - (3)]), 0, 0, (yyvsp[(2) - (3)])); ;}
    break;

  case 83:
#line 313 "src/parser.y"
    { pendName((yyvsp[(2) - (4)]), 1, 0, (yyvsp[(3) - (4)])); ;}
    break;

  case 84:
#line 315 "src/parser.y"
    { pendName((yyvsp[(2) - (4)]), 0, 1, (yyvsp[(3) - (4)])); ;}
    break;

  case 85:
#line 319 "src/parser.y"
    { (yyval) = 0; ;}
    break;

  case 86:
#line 320 "src/parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 87:
#line 324 "src/parser.y"
    { (yyval) = 1; ;}
    break;

  case 88:
#line 326 "src/parser.y"
    { (yyval) = (yyvsp[(1) - (4)]) + 1; ;}
    break;

  case 103:
#line 365 "src/parser.y"
    { applyPendingType(typeNameAt((yyvsp[(1) - (2)]))); ;}
    break;

  case 104:
#line 366 "src/parser.y"
    { applyPendingType(typeNameAt((yyvsp[(2) - (3)]))); ;}
    break;

  case 105:
#line 367 "src/parser.y"
    { applyPendingType(typeNameAt((yyvsp[(1) - (2)]))); ;}
    break;

  case 108:
#line 381 "src/parser.y"
    { pendName((yyvsp[(1) - (3)]), 0, 0, (yyvsp[(2) - (3)])); ;}
    break;

  case 109:
#line 383 "src/parser.y"
    { pendName((yyvsp[(1) - (4)]), 0, 0, 0); ;}
    break;

  case 110:
#line 385 "src/parser.y"
    { pendName((yyvsp[(2) - (4)]), 1, 0, (yyvsp[(3) - (4)])); ;}
    break;

  case 111:
#line 387 "src/parser.y"
    { pendName((yyvsp[(2) - (4)]), 0, 1, (yyvsp[(3) - (4)])); ;}
    break;

  case 117:
#line 403 "src/parser.y"
    { yyerrok; ;}
    break;

  case 137:
#line 442 "src/parser.y"
    { setDeclType((yyvsp[(2) - (3)]), "LABEL"); ;}
    break;

  case 138:
#line 446 "src/parser.y"
    { setDeclType((yyvsp[(1) - (3)]), "LABEL"); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2315 "build/parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 638 "src/parser.y"


//output
void yyerror(const char *message) {
    if (lexicalErrorCount() != 0) return;

    std::string detail = message;
    const std::string prefix = "syntax error, ";
    if (detail.compare(0, prefix.size(), prefix) == 0) {
        detail.erase(0, prefix.size());
    } else if (detail == "syntax error") {
        detail = "invalid token sequence";
    }

    g_syntaxErrors.push_back({ detail, yylloc.first_line, yylloc.first_column });
}

static void printSyntaxErrors() {
    for(size_t i = 0; i < g_syntaxErrors.size(); i++){
        cout << g_filename << " line: " << g_syntaxErrors[i].line
             << " col: " << g_syntaxErrors[i].col
             << " syntax error: " << g_syntaxErrors[i].message << endl;
    }
    cout << g_syntaxErrors.size() << " syntax error(s) found." << endl;
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
    g_filename = argv[1];
    yyin = f;
    int parseResult = yyparse();
    while (yylex() != 0) { } // finish scanning after a parse error
    fclose(f);
    if (lexicalErrorCount() != 0) {
        printErrors(argv[1]);
        return 1;
    }
    if (parseResult != 0 || !g_syntaxErrors.empty()) {
        printSyntaxErrors();
        return 1;
    }
    std::cout << "Parsing successful." << std::endl << std::endl;
    printTable();
    return 0;
}

