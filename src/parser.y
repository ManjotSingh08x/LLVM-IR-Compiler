%{
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
%}

%locations

%error-verbose

%token TOK_IDENTIFIER TOK_INT_LITERAL TOK_FLOAT_LITERAL TOK_CHAR_LITERAL TOK_STRING_LITERAL

%token TOK_INT TOK_CHAR TOK_FLOAT TOK_DOUBLE TOK_VOID TOK_BOOL TOK_AUTO
%token TOK_DEFAULT TOK_BREAK TOK_CONTINUE TOK_GOTO TOK_RETURN
%token TOK_IF TOK_ELSE TOK_FOR TOK_WHILE TOK_DO TOK_SWITCH TOK_CASE
%token TOK_STRUCT TOK_CLASS TOK_PUBLIC TOK_PRIVATE TOK_PROTECTED
%token TOK_NEW TOK_DELETE TOK_TRUE TOK_FALSE TOK_THIS TOK_NULL TOK_SIZEOF
%token TOK_STATIC
%token TOK__KEYWORD_END

%token TOK_PLUS TOK_MINUS TOK_STAR TOK_SLASH TOK_PERCENT
%token TOK_INC TOK_DEC
%token TOK_ASSIGN TOK_PLUS_ASSIGN TOK_MINUS_ASSIGN TOK_STAR_ASSIGN
%token TOK_SLASH_ASSIGN TOK_PERCENT_ASSIGN
%token TOK_EQ TOK_NEQ TOK_LT TOK_GT TOK_LE TOK_GE
%token TOK_AND TOK_OR TOK_NOT
%token TOK_BITAND TOK_BITOR TOK_BITXOR TOK_BITNOT TOK_SHL TOK_SHR
%token TOK_ARROW TOK_SCOPE TOK_DOT TOK_QUESTION TOK_COLON
%token TOK__OPERATOR_END

%token TOK_LPAREN TOK_RPAREN TOK_LBRACE TOK_RBRACE
%token TOK_LBRACKET TOK_RBRACKET TOK_SEMI TOK_COMMA
%token TOK__PUNCTUATION_END

%nonassoc IDENTIFIER_EXPRESSION
%nonassoc TOK_STAR TOK_BITAND
%nonassoc LOWER_THAN_ELSE
%nonassoc TOK_ELSE

%start translation_unit

%%

//top level
translation_unit
    : /* empty */
    | translation_unit external_item
    ;

external_item
    : record_definition
    | class_definition
    | function_definition
    | function_declaration
    | qualified_special_member_definition
    | declaration
    | static_item
    | error TOK_SEMI                  { yyerrok; }
    ;

//static applies to variables and functions, at file scope and inside a class
static_item
    : TOK_STATIC function_definition
    | TOK_STATIC function_declaration
    | TOK_STATIC declaration
    ;


qualified_special_member_definition
    : TOK_IDENTIFIER TOK_SCOPE TOK_IDENTIFIER
      TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($3, "PROCEDURE"); }
    | TOK_IDENTIFIER TOK_SCOPE TOK_BITNOT TOK_IDENTIFIER
      TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($4, "PROCEDURE"); }
    ;

record_definition
    : TOK_STRUCT TOK_IDENTIFIER inheritance_opt TOK_LBRACE record_items_opt TOK_RBRACE TOK_SEMI
                                                      { setDeclType($2, "STRUCT"); }
    ;

record_items_opt
    : /* empty */
    | record_items_opt member_item
    ;

class_definition
    : TOK_CLASS TOK_IDENTIFIER inheritance_opt TOK_LBRACE class_items_opt TOK_RBRACE TOK_SEMI
                                                      { setDeclType($2, "CLASS"); }
    ;

//the access specifier is optional, as it is in C++, and more than one base may
//be listed
inheritance_opt
    : /* empty */
    | TOK_COLON base_list
    ;

base_list
    : base_specifier
    | base_list TOK_COMMA base_specifier
    ;

base_specifier
    : access_specifier TOK_IDENTIFIER
    | TOK_IDENTIFIER
    ;

class_items_opt
    : /* empty */
    | class_items_opt class_item
    ;

class_item
    : access_specifier TOK_COLON
    | member_item
    ;


special_member_definition
    : TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($1, "PROCEDURE"); }
    | TOK_BITNOT TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($2, "PROCEDURE"); }
    ;

special_member_declaration
    : TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI
                                                      { setDeclType($1, "PROCEDURE"); }
    | TOK_BITNOT TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI
                                                      { setDeclType($2, "PROCEDURE"); }
    ;

access_specifier
    : TOK_PUBLIC
    | TOK_PRIVATE
    | TOK_PROTECTED
    ;

member_item
    : record_definition
    | class_definition
    | function_definition
    | function_declaration
    | special_member_definition
    | special_member_declaration
    | declaration
    | static_item
    | error TOK_SEMI                  { yyerrok; }
    ;

//declarations
simple_type
    : TOK_INT
    | TOK_CHAR
    | TOK_FLOAT
    | TOK_DOUBLE
    | TOK_VOID
    | TOK_BOOL
    | TOK_AUTO
    ;

type_specifier
    : simple_type                                     { $$ = $1; }
    | TOK_IDENTIFIER                                  { $$ = $1; }
    | TOK_STRUCT TOK_IDENTIFIER                       { $$ = $2; }
    ;

qualified_name
    : TOK_IDENTIFIER                                  { $$ = $1; }
    | qualified_name TOK_SCOPE TOK_IDENTIFIER         { $$ = $3; }
    ;

function_definition
    : type_specifier qualified_name TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($2, "PROCEDURE"); }
    ;

function_declaration
    : type_specifier qualified_name TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI
                                                      { setDeclType($2, "PROCEDURE"); }
    ;

parameter_list_opt
    : /* empty */
    | parameter_list
    ;

parameter_list
    : parameter_declaration
    | parameter_list TOK_COMMA parameter_declaration
    ;

parameter_declaration
    : type_specifier parameter_declarator_opt         { applyPendingType(typeNameAt($1)); }
    ;

parameter_declarator_opt
    : /* empty */
    | TOK_IDENTIFIER array_dimensions_opt             { pendName($1, 0, 0, $2); }
    | TOK_STAR TOK_IDENTIFIER array_dimensions_opt    { pendName($2, 1, 0, $3); }
    | TOK_BITAND TOK_IDENTIFIER array_dimensions_opt  { pendName($2, 0, 1, $3); }
    | parenthesized_parameter_declarator
    ;


parenthesized_parameter_declarator
    : TOK_LPAREN TOK_STAR parameter_name_opt TOK_RPAREN
      TOK_LPAREN parameter_list_opt TOK_RPAREN        { setDeclType($3, "PROCEDURE"); }
    | TOK_LPAREN TOK_STAR parameter_name_opt TOK_RPAREN array_dimensions
                                                      { pendName($3, 1, 0, $5); }
    | TOK_LPAREN TOK_BITAND parameter_name_opt TOK_RPAREN array_dimensions
                                                      { pendName($3, 0, 1, $5); }
    ;

parameter_name_opt
    : /* empty */                                     { $$ = -1; }
    | TOK_IDENTIFIER                                  { $$ = $1; }
    ;

declaration
    : type_specifier init_declarator_list TOK_SEMI
                                                      { applyPendingType(typeNameAt($1)); }
    | parenthesized_declarator TOK_SEMI
    ;


parenthesized_declarator
    : simple_type TOK_LPAREN TOK_STAR TOK_IDENTIFIER TOK_RPAREN
      TOK_LPAREN parameter_list_opt TOK_RPAREN initializer_opt
                                                      { setDeclType($4, "PROCEDURE"); }
    | simple_type TOK_LPAREN TOK_STAR TOK_IDENTIFIER TOK_RPAREN
      array_dimensions initializer_opt
                                                      { pendName($4, 1, 0, $6);
                                                        applyPendingType(typeNameAt($1)); }
    ;

init_declarator_list
    : init_declarator
    | init_declarator_list TOK_COMMA init_declarator
    ;

init_declarator
    : qualified_name array_dimensions_opt initializer_opt
                                                      { pendName($1, 0, 0, $2); }
    | TOK_STAR qualified_name array_dimensions_opt initializer_opt
                                                      { pendName($2, 1, 0, $3); }
    | TOK_BITAND qualified_name array_dimensions_opt initializer_opt
                                                      { pendName($2, 0, 1, $3); }
    ;

array_dimensions_opt
    : /* empty */                                     { $$ = 0; }
    | array_dimensions                                { $$ = $1; }
    ;

array_dimensions
    : TOK_LBRACKET expression_opt TOK_RBRACKET        { $$ = 1; }
    | array_dimensions TOK_LBRACKET expression_opt TOK_RBRACKET
                                                      { $$ = $1 + 1; }
    ;

//both copy initialisation, int a = 1, and list initialisation, int a{1}
initializer_opt
    : /* empty */
    | TOK_ASSIGN initializer
    | braced_init_list
    ;


initializer
    : assignment_expression
    | braced_init_list
    ;

braced_init_list
    : TOK_LBRACE initializer_list_opt TOK_RBRACE
    ;

initializer_list_opt
    : /* empty */
    | initializer_list
    | initializer_list TOK_COMMA
    ;

initializer_list
    : initializer
    | initializer_list TOK_COMMA initializer
    ;

//local declarations
local_declaration
    : local_declaration_body TOK_SEMI
    | TOK_STATIC local_declaration_body TOK_SEMI
    ;

local_declaration_body
    : parenthesized_declarator
    | simple_type init_declarator_list                { applyPendingType(typeNameAt($1)); }
    | TOK_STRUCT TOK_IDENTIFIER init_declarator_list  { applyPendingType(typeNameAt($2)); }
    | TOK_IDENTIFIER named_type_declarator_list       { applyPendingType(typeNameAt($1)); }
    ;

named_type_declarator_list
    : named_type_declarator
    | named_type_declarator_list TOK_COMMA named_type_declarator
    ;

//the parenthesised form is offered only here, in the declarator used for local
//declarations of a named type. a function cannot be declared inside a block, so
//Dog d(3) is unambiguous there. at file scope the same shape would collide with
//a function declaration, which is the most vexing parse, so it is not offered
named_type_declarator
    : TOK_IDENTIFIER array_dimensions_opt initializer_opt
                                                      { pendName($1, 0, 0, $2); }
    | TOK_IDENTIFIER TOK_LPAREN argument_list_opt TOK_RPAREN
                                                      { pendName($1, 0, 0, 0); }
    | TOK_STAR TOK_IDENTIFIER array_dimensions_opt initializer_opt
                                                      { pendName($2, 1, 0, $3); }
    | TOK_BITAND TOK_IDENTIFIER array_dimensions_opt initializer_opt
                                                      { pendName($2, 0, 1, $3); }
    ;

//statements
compound_statement
    : TOK_LBRACE block_items_opt TOK_RBRACE
    ;

block_items_opt
    : /* empty */
    | block_items_opt block_item
    ;

block_item
    : local_declaration
    | statement
    | error TOK_SEMI                  { yyerrok; }
    ;

statement
    : compound_statement
    | expression_statement
    | selection_statement
    | iteration_statement
    | jump_statement
    | labeled_statement
    ;

expression_statement
    : expression_opt TOK_SEMI
    ;

selection_statement
    : TOK_IF TOK_LPAREN expression TOK_RPAREN statement %prec LOWER_THAN_ELSE
    | TOK_IF TOK_LPAREN expression TOK_RPAREN statement TOK_ELSE statement
    | TOK_SWITCH TOK_LPAREN expression TOK_RPAREN statement
    ;

iteration_statement
    : TOK_WHILE TOK_LPAREN expression TOK_RPAREN statement
    | TOK_DO statement TOK_WHILE TOK_LPAREN expression TOK_RPAREN TOK_SEMI
    | TOK_FOR TOK_LPAREN for_initializer TOK_SEMI expression_opt TOK_SEMI
      expression_opt TOK_RPAREN statement
    ;

for_initializer
    : /* empty */
    | expression
    | local_declaration_body
    ;

jump_statement
    : TOK_RETURN expression_opt TOK_SEMI
    | TOK_BREAK TOK_SEMI
    | TOK_CONTINUE TOK_SEMI
    | TOK_GOTO TOK_IDENTIFIER TOK_SEMI                { setDeclType($2, "LABEL"); }
    ;

labeled_statement
    : TOK_IDENTIFIER TOK_COLON statement              { setDeclType($1, "LABEL"); }
    | TOK_CASE conditional_expression TOK_COLON statement
    | TOK_DEFAULT TOK_COLON statement
    ;

//expressions
expression_opt
    : /* empty */
    | expression
    ;

expression
    : assignment_expression
    ;

assignment_expression
    : conditional_expression
    | unary_expression assignment_operator assignment_expression
    ;

assignment_operator
    : TOK_ASSIGN
    | TOK_PLUS_ASSIGN
    | TOK_MINUS_ASSIGN
    | TOK_STAR_ASSIGN
    | TOK_SLASH_ASSIGN
    | TOK_PERCENT_ASSIGN
    ;

conditional_expression
    : logical_or_expression
    | logical_or_expression TOK_QUESTION expression TOK_COLON conditional_expression
    ;

logical_or_expression
    : logical_and_expression
    | logical_or_expression TOK_OR logical_and_expression
    ;

logical_and_expression
    : bitwise_or_expression
    | logical_and_expression TOK_AND bitwise_or_expression
    ;

bitwise_or_expression
    : bitwise_xor_expression
    | bitwise_or_expression TOK_BITOR bitwise_xor_expression
    ;

bitwise_xor_expression
    : bitwise_and_expression
    | bitwise_xor_expression TOK_BITXOR bitwise_and_expression
    ;

bitwise_and_expression
    : equality_expression
    | bitwise_and_expression TOK_BITAND equality_expression
    ;

equality_expression
    : relational_expression
    | equality_expression TOK_EQ relational_expression
    | equality_expression TOK_NEQ relational_expression
    ;

relational_expression
    : shift_expression
    | relational_expression TOK_LT shift_expression
    | relational_expression TOK_GT shift_expression
    | relational_expression TOK_LE shift_expression
    | relational_expression TOK_GE shift_expression
    ;

shift_expression
    : additive_expression
    | shift_expression TOK_SHL additive_expression
    | shift_expression TOK_SHR additive_expression
    ;

additive_expression
    : multiplicative_expression
    | additive_expression TOK_PLUS multiplicative_expression
    | additive_expression TOK_MINUS multiplicative_expression
    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression TOK_STAR unary_expression
    | multiplicative_expression TOK_SLASH unary_expression
    | multiplicative_expression TOK_PERCENT unary_expression
    ;

unary_expression
    : postfix_expression
    | TOK_INC unary_expression
    | TOK_DEC unary_expression
    | TOK_PLUS unary_expression
    | TOK_MINUS unary_expression
    | TOK_NOT unary_expression
    | TOK_BITNOT unary_expression
    | TOK_STAR unary_expression
    | TOK_BITAND unary_expression
    | TOK_NEW allocation_type allocation_suffix_opt
    | TOK_DELETE unary_expression
    | TOK_DELETE TOK_LBRACKET TOK_RBRACKET delete_operand
    ;


delete_operand
    : TOK_IDENTIFIER
    | TOK_THIS
    | delete_operand TOK_DOT TOK_IDENTIFIER
    | delete_operand TOK_ARROW TOK_IDENTIFIER
    | delete_operand TOK_LBRACKET expression TOK_RBRACKET
    ;

allocation_type
    : simple_type
    | TOK_IDENTIFIER
    | TOK_STRUCT TOK_IDENTIFIER
    ;

allocation_suffix_opt
    : /* empty */
    | TOK_LPAREN argument_list_opt TOK_RPAREN
    | braced_init_list
    | allocation_dimensions
    | allocation_dimensions braced_init_list
    ;


allocation_dimensions
    : TOK_LBRACKET expression TOK_RBRACKET
    | allocation_dimensions TOK_LBRACKET expression TOK_RBRACKET
    ;

postfix_expression
    : primary_expression
    | postfix_expression TOK_LBRACKET expression TOK_RBRACKET
    | postfix_expression TOK_LPAREN argument_list_opt TOK_RPAREN
    | postfix_expression TOK_DOT TOK_IDENTIFIER
    | postfix_expression TOK_ARROW TOK_IDENTIFIER
    | postfix_expression TOK_INC
    | postfix_expression TOK_DEC
    ;

argument_list_opt
    : /* empty */
    | argument_list
    ;

argument_list
    : assignment_expression
    | argument_list TOK_COMMA assignment_expression
    ;

primary_expression
    : TOK_IDENTIFIER %prec IDENTIFIER_EXPRESSION
    | TOK_INT_LITERAL
    | TOK_FLOAT_LITERAL
    | TOK_CHAR_LITERAL
    | TOK_STRING_LITERAL
    | TOK_TRUE
    | TOK_FALSE
    | TOK_NULL
    | TOK_THIS
    | TOK_LPAREN expression TOK_RPAREN
    | lambda_expression
    ;

lambda_expression
    : TOK_LBRACKET TOK_RBRACKET TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
    | TOK_LBRACKET capture_default TOK_RBRACKET TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
    | TOK_LBRACKET capture_list TOK_RBRACKET TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
    ;

capture_default
    : TOK_ASSIGN
    | TOK_BITAND
    ;

capture_list
    : capture
    | capture_list TOK_COMMA capture
    ;

capture
    : TOK_IDENTIFIER
    | TOK_BITAND TOK_IDENTIFIER
    | TOK_THIS
    ;

%%

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
