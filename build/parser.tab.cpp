/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
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

#line 142 "build/parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOK_IDENTIFIER = 3,             /* TOK_IDENTIFIER  */
  YYSYMBOL_TOK_INT_LITERAL = 4,            /* TOK_INT_LITERAL  */
  YYSYMBOL_TOK_FLOAT_LITERAL = 5,          /* TOK_FLOAT_LITERAL  */
  YYSYMBOL_TOK_CHAR_LITERAL = 6,           /* TOK_CHAR_LITERAL  */
  YYSYMBOL_TOK_STRING_LITERAL = 7,         /* TOK_STRING_LITERAL  */
  YYSYMBOL_TOK_INT = 8,                    /* TOK_INT  */
  YYSYMBOL_TOK_CHAR = 9,                   /* TOK_CHAR  */
  YYSYMBOL_TOK_FLOAT = 10,                 /* TOK_FLOAT  */
  YYSYMBOL_TOK_DOUBLE = 11,                /* TOK_DOUBLE  */
  YYSYMBOL_TOK_VOID = 12,                  /* TOK_VOID  */
  YYSYMBOL_TOK_BOOL = 13,                  /* TOK_BOOL  */
  YYSYMBOL_TOK_AUTO = 14,                  /* TOK_AUTO  */
  YYSYMBOL_TOK_DEFAULT = 15,               /* TOK_DEFAULT  */
  YYSYMBOL_TOK_BREAK = 16,                 /* TOK_BREAK  */
  YYSYMBOL_TOK_CONTINUE = 17,              /* TOK_CONTINUE  */
  YYSYMBOL_TOK_GOTO = 18,                  /* TOK_GOTO  */
  YYSYMBOL_TOK_RETURN = 19,                /* TOK_RETURN  */
  YYSYMBOL_TOK_IF = 20,                    /* TOK_IF  */
  YYSYMBOL_TOK_ELSE = 21,                  /* TOK_ELSE  */
  YYSYMBOL_TOK_FOR = 22,                   /* TOK_FOR  */
  YYSYMBOL_TOK_WHILE = 23,                 /* TOK_WHILE  */
  YYSYMBOL_TOK_DO = 24,                    /* TOK_DO  */
  YYSYMBOL_TOK_SWITCH = 25,                /* TOK_SWITCH  */
  YYSYMBOL_TOK_CASE = 26,                  /* TOK_CASE  */
  YYSYMBOL_TOK_STRUCT = 27,                /* TOK_STRUCT  */
  YYSYMBOL_TOK_CLASS = 28,                 /* TOK_CLASS  */
  YYSYMBOL_TOK_PUBLIC = 29,                /* TOK_PUBLIC  */
  YYSYMBOL_TOK_PRIVATE = 30,               /* TOK_PRIVATE  */
  YYSYMBOL_TOK_PROTECTED = 31,             /* TOK_PROTECTED  */
  YYSYMBOL_TOK_NEW = 32,                   /* TOK_NEW  */
  YYSYMBOL_TOK_DELETE = 33,                /* TOK_DELETE  */
  YYSYMBOL_TOK_TRUE = 34,                  /* TOK_TRUE  */
  YYSYMBOL_TOK_FALSE = 35,                 /* TOK_FALSE  */
  YYSYMBOL_TOK_THIS = 36,                  /* TOK_THIS  */
  YYSYMBOL_TOK_NULL = 37,                  /* TOK_NULL  */
  YYSYMBOL_TOK_SIZEOF = 38,                /* TOK_SIZEOF  */
  YYSYMBOL_TOK__KEYWORD_END = 39,          /* TOK__KEYWORD_END  */
  YYSYMBOL_TOK_PLUS = 40,                  /* TOK_PLUS  */
  YYSYMBOL_TOK_MINUS = 41,                 /* TOK_MINUS  */
  YYSYMBOL_TOK_STAR = 42,                  /* TOK_STAR  */
  YYSYMBOL_TOK_SLASH = 43,                 /* TOK_SLASH  */
  YYSYMBOL_TOK_PERCENT = 44,               /* TOK_PERCENT  */
  YYSYMBOL_TOK_INC = 45,                   /* TOK_INC  */
  YYSYMBOL_TOK_DEC = 46,                   /* TOK_DEC  */
  YYSYMBOL_TOK_ASSIGN = 47,                /* TOK_ASSIGN  */
  YYSYMBOL_TOK_PLUS_ASSIGN = 48,           /* TOK_PLUS_ASSIGN  */
  YYSYMBOL_TOK_MINUS_ASSIGN = 49,          /* TOK_MINUS_ASSIGN  */
  YYSYMBOL_TOK_STAR_ASSIGN = 50,           /* TOK_STAR_ASSIGN  */
  YYSYMBOL_TOK_SLASH_ASSIGN = 51,          /* TOK_SLASH_ASSIGN  */
  YYSYMBOL_TOK_PERCENT_ASSIGN = 52,        /* TOK_PERCENT_ASSIGN  */
  YYSYMBOL_TOK_EQ = 53,                    /* TOK_EQ  */
  YYSYMBOL_TOK_NEQ = 54,                   /* TOK_NEQ  */
  YYSYMBOL_TOK_LT = 55,                    /* TOK_LT  */
  YYSYMBOL_TOK_GT = 56,                    /* TOK_GT  */
  YYSYMBOL_TOK_LE = 57,                    /* TOK_LE  */
  YYSYMBOL_TOK_GE = 58,                    /* TOK_GE  */
  YYSYMBOL_TOK_AND = 59,                   /* TOK_AND  */
  YYSYMBOL_TOK_OR = 60,                    /* TOK_OR  */
  YYSYMBOL_TOK_NOT = 61,                   /* TOK_NOT  */
  YYSYMBOL_TOK_BITAND = 62,                /* TOK_BITAND  */
  YYSYMBOL_TOK_BITOR = 63,                 /* TOK_BITOR  */
  YYSYMBOL_TOK_BITXOR = 64,                /* TOK_BITXOR  */
  YYSYMBOL_TOK_BITNOT = 65,                /* TOK_BITNOT  */
  YYSYMBOL_TOK_SHL = 66,                   /* TOK_SHL  */
  YYSYMBOL_TOK_SHR = 67,                   /* TOK_SHR  */
  YYSYMBOL_TOK_ARROW = 68,                 /* TOK_ARROW  */
  YYSYMBOL_TOK_SCOPE = 69,                 /* TOK_SCOPE  */
  YYSYMBOL_TOK_DOT = 70,                   /* TOK_DOT  */
  YYSYMBOL_TOK_QUESTION = 71,              /* TOK_QUESTION  */
  YYSYMBOL_TOK_COLON = 72,                 /* TOK_COLON  */
  YYSYMBOL_TOK__OPERATOR_END = 73,         /* TOK__OPERATOR_END  */
  YYSYMBOL_TOK_LPAREN = 74,                /* TOK_LPAREN  */
  YYSYMBOL_TOK_RPAREN = 75,                /* TOK_RPAREN  */
  YYSYMBOL_TOK_LBRACE = 76,                /* TOK_LBRACE  */
  YYSYMBOL_TOK_RBRACE = 77,                /* TOK_RBRACE  */
  YYSYMBOL_TOK_LBRACKET = 78,              /* TOK_LBRACKET  */
  YYSYMBOL_TOK_RBRACKET = 79,              /* TOK_RBRACKET  */
  YYSYMBOL_TOK_SEMI = 80,                  /* TOK_SEMI  */
  YYSYMBOL_TOK_COMMA = 81,                 /* TOK_COMMA  */
  YYSYMBOL_TOK__PUNCTUATION_END = 82,      /* TOK__PUNCTUATION_END  */
  YYSYMBOL_IDENTIFIER_EXPRESSION = 83,     /* IDENTIFIER_EXPRESSION  */
  YYSYMBOL_LOWER_THAN_ELSE = 84,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_YYACCEPT = 85,                  /* $accept  */
  YYSYMBOL_translation_unit = 86,          /* translation_unit  */
  YYSYMBOL_external_item = 87,             /* external_item  */
  YYSYMBOL_qualified_special_member_definition = 88, /* qualified_special_member_definition  */
  YYSYMBOL_record_definition = 89,         /* record_definition  */
  YYSYMBOL_record_items_opt = 90,          /* record_items_opt  */
  YYSYMBOL_class_definition = 91,          /* class_definition  */
  YYSYMBOL_inheritance_opt = 92,           /* inheritance_opt  */
  YYSYMBOL_base_list = 93,                 /* base_list  */
  YYSYMBOL_base_specifier = 94,            /* base_specifier  */
  YYSYMBOL_class_items_opt = 95,           /* class_items_opt  */
  YYSYMBOL_class_item = 96,                /* class_item  */
  YYSYMBOL_special_member_definition = 97, /* special_member_definition  */
  YYSYMBOL_special_member_declaration = 98, /* special_member_declaration  */
  YYSYMBOL_access_specifier = 99,          /* access_specifier  */
  YYSYMBOL_member_item = 100,              /* member_item  */
  YYSYMBOL_simple_type = 101,              /* simple_type  */
  YYSYMBOL_type_specifier = 102,           /* type_specifier  */
  YYSYMBOL_qualified_name = 103,           /* qualified_name  */
  YYSYMBOL_function_definition = 104,      /* function_definition  */
  YYSYMBOL_function_declaration = 105,     /* function_declaration  */
  YYSYMBOL_parameter_list_opt = 106,       /* parameter_list_opt  */
  YYSYMBOL_parameter_list = 107,           /* parameter_list  */
  YYSYMBOL_parameter_declaration = 108,    /* parameter_declaration  */
  YYSYMBOL_parameter_declarator_opt = 109, /* parameter_declarator_opt  */
  YYSYMBOL_parenthesized_parameter_declarator = 110, /* parenthesized_parameter_declarator  */
  YYSYMBOL_parameter_name_opt = 111,       /* parameter_name_opt  */
  YYSYMBOL_declaration = 112,              /* declaration  */
  YYSYMBOL_parenthesized_declarator = 113, /* parenthesized_declarator  */
  YYSYMBOL_init_declarator_list = 114,     /* init_declarator_list  */
  YYSYMBOL_init_declarator = 115,          /* init_declarator  */
  YYSYMBOL_array_dimensions_opt = 116,     /* array_dimensions_opt  */
  YYSYMBOL_array_dimensions = 117,         /* array_dimensions  */
  YYSYMBOL_initializer_opt = 118,          /* initializer_opt  */
  YYSYMBOL_initializer = 119,              /* initializer  */
  YYSYMBOL_initializer_list_opt = 120,     /* initializer_list_opt  */
  YYSYMBOL_initializer_list = 121,         /* initializer_list  */
  YYSYMBOL_local_declaration = 122,        /* local_declaration  */
  YYSYMBOL_local_declaration_body = 123,   /* local_declaration_body  */
  YYSYMBOL_named_type_declarator_list = 124, /* named_type_declarator_list  */
  YYSYMBOL_named_type_declarator = 125,    /* named_type_declarator  */
  YYSYMBOL_compound_statement = 126,       /* compound_statement  */
  YYSYMBOL_block_items_opt = 127,          /* block_items_opt  */
  YYSYMBOL_block_item = 128,               /* block_item  */
  YYSYMBOL_statement = 129,                /* statement  */
  YYSYMBOL_expression_statement = 130,     /* expression_statement  */
  YYSYMBOL_selection_statement = 131,      /* selection_statement  */
  YYSYMBOL_iteration_statement = 132,      /* iteration_statement  */
  YYSYMBOL_for_initializer = 133,          /* for_initializer  */
  YYSYMBOL_jump_statement = 134,           /* jump_statement  */
  YYSYMBOL_labeled_statement = 135,        /* labeled_statement  */
  YYSYMBOL_expression_opt = 136,           /* expression_opt  */
  YYSYMBOL_expression = 137,               /* expression  */
  YYSYMBOL_assignment_expression = 138,    /* assignment_expression  */
  YYSYMBOL_assignment_operator = 139,      /* assignment_operator  */
  YYSYMBOL_conditional_expression = 140,   /* conditional_expression  */
  YYSYMBOL_logical_or_expression = 141,    /* logical_or_expression  */
  YYSYMBOL_logical_and_expression = 142,   /* logical_and_expression  */
  YYSYMBOL_bitwise_or_expression = 143,    /* bitwise_or_expression  */
  YYSYMBOL_bitwise_xor_expression = 144,   /* bitwise_xor_expression  */
  YYSYMBOL_bitwise_and_expression = 145,   /* bitwise_and_expression  */
  YYSYMBOL_equality_expression = 146,      /* equality_expression  */
  YYSYMBOL_relational_expression = 147,    /* relational_expression  */
  YYSYMBOL_shift_expression = 148,         /* shift_expression  */
  YYSYMBOL_additive_expression = 149,      /* additive_expression  */
  YYSYMBOL_multiplicative_expression = 150, /* multiplicative_expression  */
  YYSYMBOL_unary_expression = 151,         /* unary_expression  */
  YYSYMBOL_delete_operand = 152,           /* delete_operand  */
  YYSYMBOL_allocation_type = 153,          /* allocation_type  */
  YYSYMBOL_allocation_suffix_opt = 154,    /* allocation_suffix_opt  */
  YYSYMBOL_allocation_dimensions = 155,    /* allocation_dimensions  */
  YYSYMBOL_postfix_expression = 156,       /* postfix_expression  */
  YYSYMBOL_argument_list_opt = 157,        /* argument_list_opt  */
  YYSYMBOL_argument_list = 158,            /* argument_list  */
  YYSYMBOL_primary_expression = 159,       /* primary_expression  */
  YYSYMBOL_lambda_expression = 160,        /* lambda_expression  */
  YYSYMBOL_capture_default = 161,          /* capture_default  */
  YYSYMBOL_capture_list = 162,             /* capture_list  */
  YYSYMBOL_capture = 163                   /* capture  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   632

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  85
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  79
/* YYNRULES -- Number of rules.  */
#define YYNRULES  230
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  439

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   339


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   110,   110,   111,   115,   116,   117,   118,   119,   120,
     121,   126,   129,   135,   140,   141,   145,   152,   153,   157,
     158,   162,   163,   167,   168,   172,   173,   178,   180,   185,
     187,   192,   193,   194,   198,   199,   200,   201,   202,   203,
     204,   205,   210,   211,   212,   213,   214,   215,   216,   220,
     221,   222,   226,   227,   231,   236,   241,   242,   246,   247,
     251,   255,   256,   257,   258,   259,   264,   266,   268,   273,
     274,   278,   280,   285,   288,   295,   296,   300,   302,   304,
     309,   310,   314,   315,   320,   321,   326,   327,   331,   332,
     333,   337,   338,   343,   347,   348,   349,   350,   354,   355,
     363,   365,   367,   369,   375,   379,   380,   384,   385,   386,
     390,   391,   392,   393,   394,   395,   399,   403,   404,   405,
     409,   410,   411,   416,   417,   418,   422,   423,   424,   425,
     429,   430,   431,   436,   437,   441,   445,   446,   450,   451,
     452,   453,   454,   455,   459,   460,   464,   465,   469,   470,
     474,   475,   479,   480,   484,   485,   489,   490,   491,   495,
     496,   497,   498,   499,   503,   504,   505,   509,   510,   511,
     515,   516,   517,   518,   522,   523,   524,   525,   526,   527,
     528,   529,   530,   531,   532,   533,   538,   539,   540,   541,
     542,   546,   547,   548,   552,   553,   554,   559,   560,   564,
     565,   566,   567,   568,   569,   570,   574,   575,   579,   580,
     584,   585,   586,   587,   588,   589,   590,   591,   592,   593,
     594,   598,   599,   600,   604,   605,   609,   610,   614,   615,
     616
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TOK_IDENTIFIER",
  "TOK_INT_LITERAL", "TOK_FLOAT_LITERAL", "TOK_CHAR_LITERAL",
  "TOK_STRING_LITERAL", "TOK_INT", "TOK_CHAR", "TOK_FLOAT", "TOK_DOUBLE",
  "TOK_VOID", "TOK_BOOL", "TOK_AUTO", "TOK_DEFAULT", "TOK_BREAK",
  "TOK_CONTINUE", "TOK_GOTO", "TOK_RETURN", "TOK_IF", "TOK_ELSE",
  "TOK_FOR", "TOK_WHILE", "TOK_DO", "TOK_SWITCH", "TOK_CASE", "TOK_STRUCT",
  "TOK_CLASS", "TOK_PUBLIC", "TOK_PRIVATE", "TOK_PROTECTED", "TOK_NEW",
  "TOK_DELETE", "TOK_TRUE", "TOK_FALSE", "TOK_THIS", "TOK_NULL",
  "TOK_SIZEOF", "TOK__KEYWORD_END", "TOK_PLUS", "TOK_MINUS", "TOK_STAR",
  "TOK_SLASH", "TOK_PERCENT", "TOK_INC", "TOK_DEC", "TOK_ASSIGN",
  "TOK_PLUS_ASSIGN", "TOK_MINUS_ASSIGN", "TOK_STAR_ASSIGN",
  "TOK_SLASH_ASSIGN", "TOK_PERCENT_ASSIGN", "TOK_EQ", "TOK_NEQ", "TOK_LT",
  "TOK_GT", "TOK_LE", "TOK_GE", "TOK_AND", "TOK_OR", "TOK_NOT",
  "TOK_BITAND", "TOK_BITOR", "TOK_BITXOR", "TOK_BITNOT", "TOK_SHL",
  "TOK_SHR", "TOK_ARROW", "TOK_SCOPE", "TOK_DOT", "TOK_QUESTION",
  "TOK_COLON", "TOK__OPERATOR_END", "TOK_LPAREN", "TOK_RPAREN",
  "TOK_LBRACE", "TOK_RBRACE", "TOK_LBRACKET", "TOK_RBRACKET", "TOK_SEMI",
  "TOK_COMMA", "TOK__PUNCTUATION_END", "IDENTIFIER_EXPRESSION",
  "LOWER_THAN_ELSE", "$accept", "translation_unit", "external_item",
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
  "array_dimensions", "initializer_opt", "initializer",
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
  "lambda_expression", "capture_default", "capture_list", "capture", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-305)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-134)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -305,   174,  -305,   -31,   -50,  -305,  -305,  -305,  -305,  -305,
    -305,  -305,    69,    79,  -305,  -305,  -305,  -305,    31,    44,
    -305,  -305,  -305,    73,  -305,    14,    52,    83,   118,  -305,
     155,   155,   120,   -45,  -305,  -305,    71,   175,    36,   146,
     153,   192,    33,    33,   241,   154,   296,   216,   169,  -305,
      44,   154,   190,  -305,  -305,  -305,  -305,   185,  -305,   272,
    -305,  -305,   201,   216,   216,  -305,  -305,   274,  -305,    29,
     210,   217,  -305,  -305,  -305,  -305,  -305,  -305,   279,   554,
    -305,  -305,  -305,  -305,   296,   296,   296,   296,   296,   296,
     296,   296,   296,    37,   225,  -305,  -305,  -305,   -40,   246,
     245,   247,   248,    41,   195,   103,   177,   182,   221,    64,
    -305,  -305,   536,  -305,   296,    33,  -305,   234,   154,    36,
    -305,    49,    15,    74,  -305,  -305,  -305,   235,   311,   312,
     -21,  -305,  -305,   -42,   154,  -305,   322,  -305,    98,    82,
    -305,  -305,  -305,  -305,  -305,  -305,  -305,  -305,  -305,   251,
    -305,  -305,  -305,   332,   265,   261,    58,  -305,  -305,   296,
     296,   296,   296,   296,   296,   296,   296,   296,   296,   296,
     296,   296,   296,   296,   296,   296,   296,   296,  -305,  -305,
    -305,  -305,  -305,  -305,   296,  -305,  -305,   340,   341,   296,
     296,   536,  -305,  -305,   266,   270,   273,  -305,   267,   275,
     347,   276,  -305,  -305,  -305,  -305,  -305,  -305,  -305,  -305,
     280,  -305,   283,  -305,   154,   -14,  -305,   235,   235,   356,
     356,  -305,  -305,  -305,  -305,  -305,   296,   296,  -305,   284,
      19,  -305,  -305,   154,   289,   290,    34,   246,  -305,   293,
     245,   247,   248,    41,   195,   195,   103,   103,   103,   103,
     177,   177,   182,   182,  -305,  -305,  -305,  -305,  -305,  -305,
    -305,   292,   287,   294,  -305,   298,   288,  -305,  -305,   270,
    -305,   154,   297,  -305,  -305,  -305,   301,  -305,  -305,  -305,
    -305,   302,   303,   410,   304,   305,   296,  -305,  -305,    81,
     306,   154,   154,   332,  -305,   296,  -305,   296,  -305,  -305,
     536,  -305,   307,   154,   216,   119,   235,   300,    78,   313,
     308,   309,   380,   296,   316,   317,   319,   489,   338,   296,
     384,  -305,    45,  -305,  -305,   351,  -305,  -305,  -305,  -305,
    -305,  -305,  -305,  -305,   358,  -305,  -305,   360,   437,   438,
     296,   270,   373,   374,  -305,  -305,  -305,    32,   378,  -305,
     154,   169,   169,  -305,   142,   451,   454,   489,   377,  -305,
     489,  -305,  -305,   379,   381,   296,   200,   296,   388,   439,
     296,   391,    44,   383,  -305,  -305,  -305,  -305,  -305,   386,
    -305,   270,   270,  -305,  -305,   143,   392,   296,   216,   235,
     235,  -305,    94,  -305,  -305,  -305,   393,    94,  -305,   389,
    -305,   395,   399,   401,   489,   383,  -305,  -305,  -305,  -305,
    -305,  -305,   402,  -305,   216,   216,  -305,   489,   296,   489,
     296,   489,  -305,  -305,  -305,  -305,   445,   394,  -305,   403,
    -305,   489,   296,   400,  -305,   404,  -305,   489,  -305
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,    50,    42,    43,    44,    45,    46,
      47,    48,     0,     0,     3,     8,     4,     5,    49,     0,
       6,     7,     9,     0,    10,     0,    51,    17,     0,    52,
       0,     0,    80,     0,    75,    72,     0,     0,     0,     0,
       0,     0,    80,    80,     0,    56,   133,    84,    81,    71,
       0,    56,     0,    22,    31,    32,    33,    18,    19,     0,
      14,    23,     0,    84,    84,    53,    50,     0,    49,    61,
       0,    57,    58,   210,   211,   212,   213,   214,     0,     0,
     215,   216,   218,   217,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   134,   135,   136,   144,   146,
     148,   150,   152,   154,   156,   159,   164,   167,   170,   174,
     199,   220,     0,    77,   133,    80,    76,     0,    56,     0,
      21,     0,     0,     0,    78,    79,    51,    80,     0,     0,
       0,    60,    65,     0,     0,   192,     0,   191,   194,     0,
     184,   177,   178,   181,   175,   176,   179,   182,   180,     0,
     228,   230,   224,   225,     0,     0,     0,   226,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   138,   139,
     140,   141,   142,   143,     0,   204,   205,     0,     0,   206,
       0,    88,    85,    86,     0,     0,     0,    20,     0,    50,
       0,     0,    34,    35,    38,    39,    15,    36,    37,    40,
       0,    24,     0,    26,    56,    84,    62,    80,    80,    69,
      69,   105,    55,    54,    59,   193,   206,     0,   183,   196,
       0,   219,   229,    56,     0,     0,     0,   147,   170,     0,
     149,   151,   153,   155,   157,   158,   160,   161,   162,   163,
     165,   166,   168,   169,   171,   172,   173,   137,   203,   202,
     208,     0,   207,     0,    91,     0,    89,    83,    11,     0,
      41,    56,     0,    13,    16,    25,     0,    74,    63,    64,
      70,     0,     0,     0,     0,     0,     0,   186,   187,   185,
       0,    56,    56,     0,   227,     0,   201,     0,   200,    87,
      90,    12,     0,    56,    84,     0,     0,     0,   210,     0,
       0,     0,     0,   133,     0,     0,     0,   133,     0,     0,
       0,   104,     0,    94,   107,     0,   110,   106,   108,   111,
     112,   113,   114,   115,     0,   195,   197,     0,     0,     0,
       0,     0,     0,     0,   145,   209,    92,     0,     0,    73,
      56,    67,    68,   109,    80,     0,     0,   133,    97,    98,
     133,   127,   128,     0,     0,     0,   123,     0,   210,     0,
       0,     0,     0,    95,    93,   116,   198,   189,   188,     0,
     221,     0,     0,    29,    27,     0,     0,   206,    84,    80,
      80,   130,     0,   132,   129,   126,     0,   210,   125,     0,
     124,     0,     0,     0,   133,    96,   190,   222,   223,    30,
      28,    66,     0,   100,    84,    84,    99,   133,   133,   133,
       0,   133,   131,   101,   102,   103,   117,     0,   120,     0,
     119,   133,   133,     0,   118,     0,   121,   133,   122
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -305,  -305,  -305,  -305,   480,  -305,   481,   456,  -305,   366,
    -305,  -305,  -305,  -305,   367,   369,     0,     9,   116,   496,
     497,   -43,  -305,   365,  -305,  -305,   281,   499,  -268,  -271,
     452,   -38,  -114,   -61,  -177,  -305,  -305,  -305,   137,  -305,
     124,  -126,  -305,  -305,  -304,  -305,  -305,  -305,  -305,  -305,
    -305,   -46,   -86,  -101,  -305,  -265,  -305,   359,   349,   355,
     357,   363,    65,   127,    67,    84,   233,  -305,  -305,  -305,
    -305,  -305,  -214,  -305,  -305,  -305,  -305,  -305,   310
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,    14,    15,   202,   121,   203,    39,    57,    58,
     122,   211,   204,   205,    59,   206,    68,    69,   115,   207,
     208,    70,    71,    72,   131,   132,   281,   209,    23,    33,
      34,    47,    48,   113,   192,   265,   266,   324,   325,   358,
     359,   326,   283,   327,   328,   329,   330,   331,   399,   332,
     333,   334,    95,    96,   184,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   289,   138,   228,
     229,   109,   261,   262,   110,   111,   155,   156,   157
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      94,    18,   124,   125,    63,    64,   149,   223,   117,   215,
      19,   193,   284,   369,   264,   323,   198,    36,   199,    25,
     159,   219,   287,     5,     6,     7,     8,     9,    10,    11,
     344,   160,   127,   112,   221,    49,    50,   150,   222,    53,
     150,   220,    12,    13,    54,    55,    56,    29,    29,    24,
     198,   373,   199,   391,   371,   288,   393,     5,     6,     7,
       8,     9,    10,    11,   114,    54,    55,    56,   194,   268,
     151,   128,    26,   151,   239,   196,    12,    13,   137,    37,
     200,   354,    27,   257,   152,   150,    30,    30,   260,   216,
     193,   129,   210,   233,   165,   166,   293,   354,   323,   153,
     422,   405,    44,   130,   263,    28,    31,    31,   221,   185,
     186,    46,   383,   426,   200,   428,   154,   430,   151,    28,
     355,    18,    18,   346,    38,   260,   201,   434,   -17,   152,
      19,    19,   187,   438,   188,    32,   355,   235,   189,   236,
     356,   285,   190,   301,   153,    51,    42,    43,   214,   338,
     357,   339,    46,    35,   277,    38,   356,    66,    29,   340,
      41,   230,     5,     6,     7,     8,     9,    10,    11,   171,
     172,   276,   226,   412,     2,     3,   227,     4,    52,   278,
     279,    67,     5,     6,     7,     8,     9,    10,    11,    44,
     290,   351,   352,   350,    45,    62,   345,    46,    46,   193,
     337,    12,    13,   397,    74,    75,    76,    77,     5,     6,
       7,     8,     9,    10,    11,   380,   387,   173,   174,   221,
      46,   384,    60,   409,   175,   176,   177,   320,   302,    61,
     244,   245,    78,    79,    80,    81,    82,    83,   250,   251,
      84,    85,    86,   349,    65,    87,    88,   114,   342,   343,
     167,   168,   169,   170,   379,   407,   408,   252,   253,   410,
     348,    89,    90,   112,   118,    91,   119,   364,   178,   179,
     180,   181,   182,   183,    92,   120,   123,   126,    93,   396,
     400,   401,   135,   322,   403,   133,   260,     5,     6,     7,
       8,     9,    10,    11,   246,   247,   248,   249,   134,    73,
      74,    75,    76,    77,   158,   161,   136,   386,   162,   195,
     164,   163,   140,    46,   217,   218,   388,   141,   142,   143,
     144,   145,   146,   147,   148,   225,   231,   413,    78,    79,
      80,    81,    82,    83,   429,   232,    84,    85,    86,   233,
     234,    87,    88,   258,   259,   267,   221,   270,   269,   271,
     272,   414,   415,   424,   425,   275,   273,    89,    90,   280,
     274,    91,   286,   291,   292,   295,   322,   296,   297,   300,
      92,   303,   427,   298,    93,   299,   304,   305,   306,   335,
     353,   341,   347,   363,   336,   360,   435,   372,   361,   362,
     365,   366,   238,   367,   238,   238,   238,   238,   238,   238,
     238,   238,   238,   238,   238,   238,   238,   238,   254,   255,
     256,   307,   370,   308,    74,    75,    76,    77,     5,     6,
       7,     8,     9,    10,    11,   309,   310,   311,   312,   313,
     314,   374,   315,   316,   317,   318,   319,   320,   375,   376,
     377,   378,    78,    79,    80,    81,    82,    83,   381,   382,
      84,    85,    86,   385,   389,    87,    88,   390,   392,   394,
     357,   395,   402,   404,    50,   406,   431,   411,   417,   418,
     419,    89,    90,   420,   432,    91,   421,   423,   433,   437,
     436,    16,    17,    40,    92,   197,   221,   321,    93,   212,
    -133,   213,   368,    74,    75,    76,    77,    20,    21,   224,
      22,   282,   116,   398,   309,   310,   311,   312,   313,   314,
     240,   315,   316,   317,   318,   319,   416,   241,   237,     0,
     242,    78,    79,    80,    81,    82,    83,   243,   238,    84,
      85,    86,     0,     0,    87,    88,     0,     0,     0,    73,
      74,    75,    76,    77,     0,     0,   294,     0,     0,     0,
      89,    90,   238,     0,    91,     0,     0,    73,    74,    75,
      76,    77,     0,    92,     0,   221,     0,    93,    78,    79,
      80,    81,    82,    83,     0,     0,    84,    85,    86,     0,
       0,    87,    88,     0,     0,     0,    78,    79,    80,    81,
      82,    83,     0,     0,    84,    85,    86,    89,    90,    87,
      88,    91,     0,     0,     0,     0,     0,     0,     0,     0,
      92,     0,   191,     0,    93,    89,    90,     0,     0,    91,
       0,     0,     0,     0,     0,     0,     0,     0,    92,     0,
       0,     0,   139
};

static const yytype_int16 yycheck[] =
{
      46,     1,    63,    64,    42,    43,    92,   133,    51,   123,
       1,   112,   226,   317,   191,   283,     1,     3,     3,    69,
      60,    42,     3,     8,     9,    10,    11,    12,    13,    14,
     295,    71,     3,    47,    76,    80,    81,     3,    80,     3,
       3,    62,    27,    28,    29,    30,    31,     3,     3,    80,
       1,   322,     3,   357,   319,    36,   360,     8,     9,    10,
      11,    12,    13,    14,    78,    29,    30,    31,   114,   195,
      36,    42,     3,    36,   160,   118,    27,    28,    78,    65,
      65,     3,     3,   184,    47,     3,    42,    42,   189,   127,
     191,    62,    77,    74,    53,    54,    62,     3,   366,    62,
     404,   372,    69,    74,   190,    74,    62,    62,    76,    45,
      46,    78,    80,   417,    65,   419,    79,   421,    36,    74,
      42,   121,   122,   300,    72,   226,    77,   431,    76,    47,
     121,   122,    68,   437,    70,    19,    42,    79,    74,    81,
      62,   227,    78,   269,    62,    74,    30,    31,    74,    68,
      72,    70,    78,    80,   215,    72,    62,     3,     3,    78,
      42,    79,     8,     9,    10,    11,    12,    13,    14,    66,
      67,   214,    74,   387,     0,     1,    78,     3,     3,   217,
     218,    27,     8,     9,    10,    11,    12,    13,    14,    69,
     233,   305,   306,    74,    74,     3,   297,    78,    78,   300,
     286,    27,    28,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,   341,    74,    40,    41,    76,
      78,   347,    76,    80,    42,    43,    44,    27,   271,    76,
     165,   166,    32,    33,    34,    35,    36,    37,   171,   172,
      40,    41,    42,   304,     3,    45,    46,    78,   291,   292,
      55,    56,    57,    58,   340,   381,   382,   173,   174,   385,
     303,    61,    62,    47,    74,    65,    81,   313,    47,    48,
      49,    50,    51,    52,    74,     3,    75,     3,    78,   365,
     366,   367,     3,   283,   370,    75,   387,     8,     9,    10,
      11,    12,    13,    14,   167,   168,   169,   170,    81,     3,
       4,     5,     6,     7,    79,    59,    27,   350,    63,    75,
      62,    64,    79,    78,     3,     3,   354,    84,    85,    86,
      87,    88,    89,    90,    91,     3,    75,   388,    32,    33,
      34,    35,    36,    37,   420,     3,    40,    41,    42,    74,
      79,    45,    46,     3,     3,    79,    76,    80,    75,    74,
       3,   389,   390,   414,   415,    72,    80,    61,    62,     3,
      80,    65,    78,    74,    74,    72,   366,    75,    81,    81,
      74,    74,   418,    79,    78,    77,    75,    75,    75,    75,
      80,    75,    75,     3,    79,    72,   432,     3,    80,    80,
      74,    74,   159,    74,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,     1,    74,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    80,    22,    23,    24,    25,    26,    27,    80,    79,
       3,     3,    32,    33,    34,    35,    36,    37,    75,    75,
      40,    41,    42,    75,     3,    45,    46,     3,    81,    80,
      72,    80,    23,    72,    81,    79,    21,    75,    75,    80,
      75,    61,    62,    74,    80,    65,    75,    75,    75,    75,
      80,     1,     1,    27,    74,   119,    76,    77,    78,   122,
      80,   122,     3,     4,     5,     6,     7,     1,     1,   134,
       1,   220,    50,   366,    15,    16,    17,    18,    19,    20,
     161,    22,    23,    24,    25,    26,   392,   162,   159,    -1,
     163,    32,    33,    34,    35,    36,    37,   164,   295,    40,
      41,    42,    -1,    -1,    45,    46,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,   236,    -1,    -1,    -1,
      61,    62,   319,    -1,    65,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    74,    -1,    76,    -1,    78,    32,    33,
      34,    35,    36,    37,    -1,    -1,    40,    41,    42,    -1,
      -1,    45,    46,    -1,    -1,    -1,    32,    33,    34,    35,
      36,    37,    -1,    -1,    40,    41,    42,    61,    62,    45,
      46,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    -1,    76,    -1,    78,    61,    62,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    -1,
      -1,    -1,    78
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    86,     0,     1,     3,     8,     9,    10,    11,    12,
      13,    14,    27,    28,    87,    88,    89,    91,   101,   102,
     104,   105,   112,   113,    80,    69,     3,     3,    74,     3,
      42,    62,   103,   114,   115,    80,     3,    65,    72,    92,
      92,    42,   103,   103,    69,    74,    78,   116,   117,    80,
      81,    74,     3,     3,    29,    30,    31,    93,    94,    99,
      76,    76,     3,   116,   116,     3,     3,    27,   101,   102,
     106,   107,   108,     3,     4,     5,     6,     7,    32,    33,
      34,    35,    36,    37,    40,    41,    42,    45,    46,    61,
      62,    65,    74,    78,   136,   137,   138,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   156,
     159,   160,    47,   118,    78,   103,   115,   106,    74,    81,
       3,    90,    95,    75,   118,   118,     3,     3,    42,    62,
      74,   109,   110,    75,    81,     3,    27,   101,   153,    78,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   137,
       3,    36,    47,    62,    79,   161,   162,   163,    79,    60,
      71,    59,    63,    64,    62,    53,    54,    55,    56,    57,
      58,    66,    67,    40,    41,    42,    43,    44,    47,    48,
      49,    50,    51,    52,   139,    45,    46,    68,    70,    74,
      78,    76,   119,   138,   136,    75,   106,    94,     1,     3,
      65,    77,    89,    91,    97,    98,   100,   104,   105,   112,
      77,    96,    99,   100,    74,   117,   116,     3,     3,    42,
      62,    76,    80,   126,   108,     3,    74,    78,   154,   155,
      79,    75,     3,    74,    79,    79,    81,   142,   151,   137,
     143,   144,   145,   146,   147,   147,   148,   148,   148,   148,
     149,   149,   150,   150,   151,   151,   151,   138,     3,     3,
     138,   157,   158,   137,   119,   120,   121,    79,   126,    75,
      80,    74,     3,    80,    80,    72,   106,   118,   116,   116,
       3,   111,   111,   127,   157,   137,    78,     3,    36,   152,
     106,    74,    74,    62,   163,    72,    75,    81,    79,    77,
      81,   126,   106,    74,    75,    75,    75,     1,     3,    15,
      16,    17,    18,    19,    20,    22,    23,    24,    25,    26,
      27,    77,   101,   113,   122,   123,   126,   128,   129,   130,
     131,   132,   134,   135,   136,    75,    79,   137,    68,    70,
      78,    75,   106,   106,   140,   138,   119,    75,   106,   118,
      74,   117,   117,    80,     3,    42,    62,    72,   124,   125,
      72,    80,    80,     3,   136,    74,    74,    74,     3,   129,
      74,   140,     3,   114,    80,    80,    79,     3,     3,   137,
     126,    75,    75,    80,   126,    75,   106,    74,   116,     3,
       3,   129,    81,   129,    80,    80,   137,     3,   123,   133,
     137,   137,    23,   137,    72,   114,    79,   126,   126,    80,
     126,    75,   157,   118,   116,   116,   125,    75,    80,    75,
      74,    75,   129,    75,   118,   118,   129,   136,   129,   137,
     129,    21,    80,    75,   129,   136,    80,    75,   129
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    85,    86,    86,    87,    87,    87,    87,    87,    87,
      87,    88,    88,    89,    90,    90,    91,    92,    92,    93,
      93,    94,    94,    95,    95,    96,    96,    97,    97,    98,
      98,    99,    99,    99,   100,   100,   100,   100,   100,   100,
     100,   100,   101,   101,   101,   101,   101,   101,   101,   102,
     102,   102,   103,   103,   104,   105,   106,   106,   107,   107,
     108,   109,   109,   109,   109,   109,   110,   110,   110,   111,
     111,   112,   112,   113,   113,   114,   114,   115,   115,   115,
     116,   116,   117,   117,   118,   118,   119,   119,   120,   120,
     120,   121,   121,   122,   123,   123,   123,   123,   124,   124,
     125,   125,   125,   125,   126,   127,   127,   128,   128,   128,
     129,   129,   129,   129,   129,   129,   130,   131,   131,   131,
     132,   132,   132,   133,   133,   133,   134,   134,   134,   134,
     135,   135,   135,   136,   136,   137,   138,   138,   139,   139,
     139,   139,   139,   139,   140,   140,   141,   141,   142,   142,
     143,   143,   144,   144,   145,   145,   146,   146,   146,   147,
     147,   147,   147,   147,   148,   148,   148,   149,   149,   149,
     150,   150,   150,   150,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   152,   152,   152,   152,
     152,   153,   153,   153,   154,   154,   154,   155,   155,   156,
     156,   156,   156,   156,   156,   156,   157,   157,   158,   158,
     159,   159,   159,   159,   159,   159,   159,   159,   159,   159,
     159,   160,   160,   160,   161,   161,   162,   162,   163,   163,
     163
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       2,     7,     8,     7,     0,     2,     7,     0,     2,     1,
       3,     2,     1,     0,     2,     2,     1,     5,     6,     5,
       6,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     3,     6,     6,     0,     1,     1,     3,
       2,     0,     2,     3,     3,     1,     7,     5,     5,     0,
       1,     3,     2,     9,     7,     1,     3,     3,     4,     4,
       0,     1,     3,     4,     0,     2,     1,     3,     0,     1,
       2,     1,     3,     2,     1,     2,     3,     2,     1,     3,
       3,     4,     4,     4,     3,     0,     2,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     2,     5,     7,     5,
       5,     7,     9,     0,     1,     1,     3,     2,     2,     3,
       3,     4,     3,     0,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     5,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     2,     4,     1,     1,     3,     3,
       4,     1,     1,     2,     0,     3,     1,     3,     4,     1,
       4,     4,     3,     3,     2,     2,     0,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     6,     7,     7,     1,     1,     1,     3,     1,     2,
       1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 10: /* external_item: error TOK_SEMI  */
#line 121 "src/parser.y"
                                      { yyerrok; }
#line 2054 "build/parser.tab.c"
    break;

  case 11: /* qualified_special_member_definition: TOK_IDENTIFIER TOK_SCOPE TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement  */
#line 128 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2060 "build/parser.tab.c"
    break;

  case 12: /* qualified_special_member_definition: TOK_IDENTIFIER TOK_SCOPE TOK_BITNOT TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement  */
#line 131 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2066 "build/parser.tab.c"
    break;

  case 13: /* record_definition: TOK_STRUCT TOK_IDENTIFIER inheritance_opt TOK_LBRACE record_items_opt TOK_RBRACE TOK_SEMI  */
#line 136 "src/parser.y"
                                                      { setDeclType(yyvsp[-5], "STRUCT"); }
#line 2072 "build/parser.tab.c"
    break;

  case 16: /* class_definition: TOK_CLASS TOK_IDENTIFIER inheritance_opt TOK_LBRACE class_items_opt TOK_RBRACE TOK_SEMI  */
#line 146 "src/parser.y"
                                                      { setDeclType(yyvsp[-5], "CLASS"); }
#line 2078 "build/parser.tab.c"
    break;

  case 27: /* special_member_definition: TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement  */
#line 179 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2084 "build/parser.tab.c"
    break;

  case 28: /* special_member_definition: TOK_BITNOT TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement  */
#line 181 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2090 "build/parser.tab.c"
    break;

  case 29: /* special_member_declaration: TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI  */
#line 186 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2096 "build/parser.tab.c"
    break;

  case 30: /* special_member_declaration: TOK_BITNOT TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI  */
#line 188 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2102 "build/parser.tab.c"
    break;

  case 41: /* member_item: error TOK_SEMI  */
#line 205 "src/parser.y"
                                      { yyerrok; }
#line 2108 "build/parser.tab.c"
    break;

  case 49: /* type_specifier: simple_type  */
#line 220 "src/parser.y"
                                                      { yyval = yyvsp[0]; }
#line 2114 "build/parser.tab.c"
    break;

  case 50: /* type_specifier: TOK_IDENTIFIER  */
#line 221 "src/parser.y"
                                                      { yyval = yyvsp[0]; }
#line 2120 "build/parser.tab.c"
    break;

  case 51: /* type_specifier: TOK_STRUCT TOK_IDENTIFIER  */
#line 222 "src/parser.y"
                                                      { yyval = yyvsp[0]; }
#line 2126 "build/parser.tab.c"
    break;

  case 52: /* qualified_name: TOK_IDENTIFIER  */
#line 226 "src/parser.y"
                                                      { yyval = yyvsp[0]; }
#line 2132 "build/parser.tab.c"
    break;

  case 53: /* qualified_name: qualified_name TOK_SCOPE TOK_IDENTIFIER  */
#line 227 "src/parser.y"
                                                      { yyval = yyvsp[0]; }
#line 2138 "build/parser.tab.c"
    break;

  case 54: /* function_definition: type_specifier qualified_name TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement  */
#line 232 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2144 "build/parser.tab.c"
    break;

  case 55: /* function_declaration: type_specifier qualified_name TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI  */
#line 237 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2150 "build/parser.tab.c"
    break;

  case 60: /* parameter_declaration: type_specifier parameter_declarator_opt  */
#line 251 "src/parser.y"
                                                      { applyPendingType(typeNameAt(yyvsp[-1])); }
#line 2156 "build/parser.tab.c"
    break;

  case 62: /* parameter_declarator_opt: TOK_IDENTIFIER array_dimensions_opt  */
#line 256 "src/parser.y"
                                                      { pendName(yyvsp[-1], 0, 0, yyvsp[0]); }
#line 2162 "build/parser.tab.c"
    break;

  case 63: /* parameter_declarator_opt: TOK_STAR TOK_IDENTIFIER array_dimensions_opt  */
#line 257 "src/parser.y"
                                                      { pendName(yyvsp[-1], 1, 0, yyvsp[0]); }
#line 2168 "build/parser.tab.c"
    break;

  case 64: /* parameter_declarator_opt: TOK_BITAND TOK_IDENTIFIER array_dimensions_opt  */
#line 258 "src/parser.y"
                                                      { pendName(yyvsp[-1], 0, 1, yyvsp[0]); }
#line 2174 "build/parser.tab.c"
    break;

  case 66: /* parenthesized_parameter_declarator: TOK_LPAREN TOK_STAR parameter_name_opt TOK_RPAREN TOK_LPAREN parameter_list_opt TOK_RPAREN  */
#line 265 "src/parser.y"
                                                      { setDeclType(yyvsp[-4], "PROCEDURE"); }
#line 2180 "build/parser.tab.c"
    break;

  case 67: /* parenthesized_parameter_declarator: TOK_LPAREN TOK_STAR parameter_name_opt TOK_RPAREN array_dimensions  */
#line 267 "src/parser.y"
                                                      { pendName(yyvsp[-2], 1, 0, yyvsp[0]); }
#line 2186 "build/parser.tab.c"
    break;

  case 68: /* parenthesized_parameter_declarator: TOK_LPAREN TOK_BITAND parameter_name_opt TOK_RPAREN array_dimensions  */
#line 269 "src/parser.y"
                                                      { pendName(yyvsp[-2], 0, 1, yyvsp[0]); }
#line 2192 "build/parser.tab.c"
    break;

  case 69: /* parameter_name_opt: %empty  */
#line 273 "src/parser.y"
                                                      { yyval = -1; }
#line 2198 "build/parser.tab.c"
    break;

  case 70: /* parameter_name_opt: TOK_IDENTIFIER  */
#line 274 "src/parser.y"
                                                      { yyval = yyvsp[0]; }
#line 2204 "build/parser.tab.c"
    break;

  case 71: /* declaration: type_specifier init_declarator_list TOK_SEMI  */
#line 279 "src/parser.y"
                                                      { applyPendingType(typeNameAt(yyvsp[-2])); }
#line 2210 "build/parser.tab.c"
    break;

  case 73: /* parenthesized_declarator: simple_type TOK_LPAREN TOK_STAR TOK_IDENTIFIER TOK_RPAREN TOK_LPAREN parameter_list_opt TOK_RPAREN initializer_opt  */
#line 287 "src/parser.y"
                                                      { setDeclType(yyvsp[-5], "PROCEDURE"); }
#line 2216 "build/parser.tab.c"
    break;

  case 74: /* parenthesized_declarator: simple_type TOK_LPAREN TOK_STAR TOK_IDENTIFIER TOK_RPAREN array_dimensions initializer_opt  */
#line 290 "src/parser.y"
                                                      { pendName(yyvsp[-3], 1, 0, yyvsp[-1]);
                                                        applyPendingType(typeNameAt(yyvsp[-6])); }
#line 2223 "build/parser.tab.c"
    break;

  case 77: /* init_declarator: qualified_name array_dimensions_opt initializer_opt  */
#line 301 "src/parser.y"
                                                      { pendName(yyvsp[-2], 0, 0, yyvsp[-1]); }
#line 2229 "build/parser.tab.c"
    break;

  case 78: /* init_declarator: TOK_STAR qualified_name array_dimensions_opt initializer_opt  */
#line 303 "src/parser.y"
                                                      { pendName(yyvsp[-2], 1, 0, yyvsp[-1]); }
#line 2235 "build/parser.tab.c"
    break;

  case 79: /* init_declarator: TOK_BITAND qualified_name array_dimensions_opt initializer_opt  */
#line 305 "src/parser.y"
                                                      { pendName(yyvsp[-2], 0, 1, yyvsp[-1]); }
#line 2241 "build/parser.tab.c"
    break;

  case 80: /* array_dimensions_opt: %empty  */
#line 309 "src/parser.y"
                                                      { yyval = 0; }
#line 2247 "build/parser.tab.c"
    break;

  case 81: /* array_dimensions_opt: array_dimensions  */
#line 310 "src/parser.y"
                                                      { yyval = yyvsp[0]; }
#line 2253 "build/parser.tab.c"
    break;

  case 82: /* array_dimensions: TOK_LBRACKET expression_opt TOK_RBRACKET  */
#line 314 "src/parser.y"
                                                      { yyval = 1; }
#line 2259 "build/parser.tab.c"
    break;

  case 83: /* array_dimensions: array_dimensions TOK_LBRACKET expression_opt TOK_RBRACKET  */
#line 316 "src/parser.y"
                                                      { yyval = yyvsp[-3] + 1; }
#line 2265 "build/parser.tab.c"
    break;

  case 95: /* local_declaration_body: simple_type init_declarator_list  */
#line 348 "src/parser.y"
                                                      { applyPendingType(typeNameAt(yyvsp[-1])); }
#line 2271 "build/parser.tab.c"
    break;

  case 96: /* local_declaration_body: TOK_STRUCT TOK_IDENTIFIER init_declarator_list  */
#line 349 "src/parser.y"
                                                      { applyPendingType(typeNameAt(yyvsp[-1])); }
#line 2277 "build/parser.tab.c"
    break;

  case 97: /* local_declaration_body: TOK_IDENTIFIER named_type_declarator_list  */
#line 350 "src/parser.y"
                                                      { applyPendingType(typeNameAt(yyvsp[-1])); }
#line 2283 "build/parser.tab.c"
    break;

  case 100: /* named_type_declarator: TOK_IDENTIFIER array_dimensions_opt initializer_opt  */
#line 364 "src/parser.y"
                                                      { pendName(yyvsp[-2], 0, 0, yyvsp[-1]); }
#line 2289 "build/parser.tab.c"
    break;

  case 101: /* named_type_declarator: TOK_IDENTIFIER TOK_LPAREN argument_list_opt TOK_RPAREN  */
#line 366 "src/parser.y"
                                                      { pendName(yyvsp[-3], 0, 0, 0); }
#line 2295 "build/parser.tab.c"
    break;

  case 102: /* named_type_declarator: TOK_STAR TOK_IDENTIFIER array_dimensions_opt initializer_opt  */
#line 368 "src/parser.y"
                                                      { pendName(yyvsp[-2], 1, 0, yyvsp[-1]); }
#line 2301 "build/parser.tab.c"
    break;

  case 103: /* named_type_declarator: TOK_BITAND TOK_IDENTIFIER array_dimensions_opt initializer_opt  */
#line 370 "src/parser.y"
                                                      { pendName(yyvsp[-2], 0, 1, yyvsp[-1]); }
#line 2307 "build/parser.tab.c"
    break;

  case 109: /* block_item: error TOK_SEMI  */
#line 386 "src/parser.y"
                                      { yyerrok; }
#line 2313 "build/parser.tab.c"
    break;

  case 129: /* jump_statement: TOK_GOTO TOK_IDENTIFIER TOK_SEMI  */
#line 425 "src/parser.y"
                                                      { setDeclType(yyvsp[-1], "LABEL"); }
#line 2319 "build/parser.tab.c"
    break;

  case 130: /* labeled_statement: TOK_IDENTIFIER TOK_COLON statement  */
#line 429 "src/parser.y"
                                                      { setDeclType(yyvsp[-2], "LABEL"); }
#line 2325 "build/parser.tab.c"
    break;


#line 2329 "build/parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 619 "src/parser.y"


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
