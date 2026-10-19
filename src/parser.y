%{
#include <cstdio>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ast/ast.h"
#include "ast/ast_printer.h"

using namespace std;
using ast::NodeList;

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
static std::unique_ptr<ast::Program> g_program;

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

//ast construction helpers. the parser stack holds raw pointers, a node takes
//ownership of its children when it is built from them

//line and column of a location, as the last two constructor arguments
#define LOC(at) (at).first_line, (at).first_column

template <class T>
static std::unique_ptr<T> own(T *node) {
    return std::unique_ptr<T>(node);
}

//moves a value off the parser stack and frees the holder
template <class T>
static T takeValue(T *holder) {
    T value = std::move(*holder);
    delete holder;
    return value;
}

template <class T>
static NodeList<T> take(NodeList<T> *list) {
    return list ? takeValue(list) : NodeList<T>();
}

template <class T>
static NodeList<T> *single(T *item) {
    NodeList<T> *list = new NodeList<T>();
    list->emplace_back(item);
    return list;
}

template <class T, class U>
static NodeList<T> *append(NodeList<T> *list, U *item) {
    list->emplace_back(item);
    return list;
}

template <class T>
static NodeList<T> *appendAll(NodeList<T> *list, NodeList<T> *more) {
    for (size_t i = 0; i < more->size(); i++) {
        list->push_back(std::move((*more)[i]));
    }
    delete more;
    return list;
}

static ast::TypeSpec *typeSpec(ast::TypeSpecKind kind, int tok, int line, int col) {
    return new ast::TypeSpec(kind, lexemeAt(tok), tok, line, col);
}

static ast::QualifiedName *nameOf(int tok) {
    if (tok < 0) return nullptr;
    ast::QualifiedName *name = new ast::QualifiedName();
    name->parts.push_back(lexemeAt(tok));
    name->lastTok = tok;
    return name;
}

//Class::member, used for out of class constructors and destructors
static ast::QualifiedName memberName(int classTok, const std::string &member, int memberTok) {
    ast::QualifiedName name;
    name.parts.push_back(lexemeAt(classTok));
    name.parts.push_back(member);
    name.lastTok = memberTok;
    return name;
}

static ast::Declarator *makeDeclarator(ast::QualifiedName *name, int line, int col, int stars,
                                       bool isReference, NodeList<ast::Expr> *dims,
                                       ast::Initializer *init) {
    ast::Declarator *declarator = new ast::Declarator();
    if (name) declarator->name = takeValue(name);
    declarator->line = line;
    declarator->col = col;
    declarator->pointerDepth = stars;
    declarator->isReference = isReference;
    declarator->dims = take(dims);
    declarator->setInitializer(init);
    return declarator;
}

static ast::VarDecl *makeVar(std::unique_ptr<ast::TypeSpec> type, ast::Declarator *declarator) {
    int line = declarator->line;
    int col = declarator->col;
    return new ast::VarDecl(std::move(type), takeValue(declarator), line, col);
}

//one VarDecl per declarator, each with its own copy of the shared type
static NodeList<ast::Decl> *makeVarDecls(ast::TypeSpec *type, NodeList<ast::Declarator> *declarators) {
    NodeList<ast::Decl> *decls = new NodeList<ast::Decl>();
    for (size_t i = 0; i < declarators->size(); i++) {
        decls->emplace_back(makeVar(type->clone(), (*declarators)[i].release()));
    }
    delete type;
    delete declarators;
    return decls;
}

static ast::ParamDecl *makeParam(ast::TypeSpec *type, ast::Declarator *declarator, int line, int col) {
    ast::Declarator value;
    if (declarator) {
        line = declarator->line;
        col = declarator->col;
        value = takeValue(declarator);
    }
    return new ast::ParamDecl(own(type), std::move(value), line, col);
}

static void markStatic(NodeList<ast::Decl> *decls) {
    for (size_t i = 0; i < decls->size(); i++) {
        ast::Decl *decl = (*decls)[i].get();
        if (decl->getKind() == ast::ASTKind::VarDecl) {
            static_cast<ast::VarDecl *>(decl)->isStatic = true;
        } else if (decl->getKind() == ast::ASTKind::FunctionDecl) {
            static_cast<ast::FunctionDecl *>(decl)->isStatic = true;
        }
    }
}
%}

%locations

%error-verbose

%union {
    int                               tok;
    int                               op;
    ast::Decl                        *decl;
    ast::Stmt                        *stmt;
    ast::Expr                        *expr;
    ast::TypeSpec                    *type;
    ast::CompoundStmt                *block;
    ast::ParamDecl                   *param;
    ast::NewExpr                     *newExpr;
    ast::QualifiedName               *qname;
    ast::Declarator                  *dtor;
    ast::Initializer                 *init;
    ast::BaseSpecifier               *base;
    ast::LambdaCapture               *capture;
    ast::NodeList<ast::Decl>         *decls;
    ast::NodeList<ast::Stmt>         *stmts;
    ast::NodeList<ast::Expr>         *exprs;
    ast::NodeList<ast::ParamDecl>    *params;
    ast::NodeList<ast::Declarator>   *dtors;
    std::vector<ast::BaseSpecifier>  *bases;
    std::vector<ast::LambdaCapture>  *captures;
}

%token <tok> TOK_IDENTIFIER TOK_INT_LITERAL TOK_FLOAT_LITERAL TOK_CHAR_LITERAL TOK_STRING_LITERAL

%token <tok> TOK_INT TOK_CHAR TOK_FLOAT TOK_DOUBLE TOK_VOID TOK_BOOL TOK_AUTO
%token <tok> TOK_DEFAULT TOK_BREAK TOK_CONTINUE TOK_GOTO TOK_RETURN
%token <tok> TOK_IF TOK_ELSE TOK_FOR TOK_WHILE TOK_DO TOK_SWITCH TOK_CASE
%token <tok> TOK_STRUCT TOK_CLASS TOK_PUBLIC TOK_PRIVATE TOK_PROTECTED
%token <tok> TOK_NEW TOK_DELETE TOK_TRUE TOK_FALSE TOK_THIS TOK_NULL TOK_SIZEOF
%token <tok> TOK_STATIC
%token <tok> TOK__KEYWORD_END

%token <tok> TOK_PLUS TOK_MINUS TOK_STAR TOK_SLASH TOK_PERCENT
%token <tok> TOK_INC TOK_DEC
%token <tok> TOK_ASSIGN TOK_PLUS_ASSIGN TOK_MINUS_ASSIGN TOK_STAR_ASSIGN
%token <tok> TOK_SLASH_ASSIGN TOK_PERCENT_ASSIGN
%token <tok> TOK_EQ TOK_NEQ TOK_LT TOK_GT TOK_LE TOK_GE
%token <tok> TOK_AND TOK_OR TOK_NOT
%token <tok> TOK_BITAND TOK_BITOR TOK_BITXOR TOK_BITNOT TOK_SHL TOK_SHR
%token <tok> TOK_ARROW TOK_SCOPE TOK_DOT TOK_QUESTION TOK_COLON
%token <tok> TOK__OPERATOR_END

%token <tok> TOK_LPAREN TOK_RPAREN TOK_LBRACE TOK_RBRACE
%token <tok> TOK_LBRACKET TOK_RBRACKET TOK_SEMI TOK_COMMA
%token <tok> TOK__PUNCTUATION_END

%type <decls>    translation_unit external_item static_item record_items_opt class_items_opt
%type <decls>    class_item member_item declaration local_declaration_body
%type <decl>     qualified_special_member_definition record_definition class_definition
%type <decl>     special_member_definition special_member_declaration
%type <decl>     function_definition function_declaration parenthesized_declarator
%type <bases>    inheritance_opt base_list
%type <base>     base_specifier
%type <op>       access_specifier assignment_operator capture_default
%type <type>     simple_type type_specifier allocation_type
%type <qname>    qualified_name
%type <params>   parameter_list_opt parameter_list
%type <param>    parameter_declaration
%type <dtor>     parameter_declarator_opt parenthesized_parameter_declarator
%type <dtor>     init_declarator named_type_declarator
%type <dtors>    init_declarator_list named_type_declarator_list
%type <tok>      parameter_name_opt
%type <exprs>    array_dimensions_opt array_dimensions initializer_list_opt initializer_list
%type <exprs>    allocation_dimensions argument_list_opt argument_list
%type <init>     initializer_opt
%type <expr>     initializer braced_init_list
%type <block>    compound_statement
%type <stmts>    block_items_opt
%type <stmt>     local_declaration block_item statement expression_statement
%type <stmt>     selection_statement iteration_statement for_initializer jump_statement
%type <stmt>     labeled_statement
%type <expr>     expression_opt expression assignment_expression conditional_expression
%type <expr>     logical_or_expression logical_and_expression bitwise_or_expression
%type <expr>     bitwise_xor_expression bitwise_and_expression equality_expression
%type <expr>     relational_expression shift_expression additive_expression
%type <expr>     multiplicative_expression unary_expression delete_operand
%type <expr>     postfix_expression primary_expression lambda_expression
%type <newExpr>  allocation_suffix_opt
%type <captures> capture_list
%type <capture>  capture

//values thrown away during error recovery
%destructor { delete $$; }
    translation_unit external_item static_item record_items_opt class_items_opt
    class_item member_item declaration local_declaration_body
    qualified_special_member_definition record_definition class_definition
    special_member_definition special_member_declaration
    function_definition function_declaration parenthesized_declarator
    inheritance_opt base_list base_specifier
    simple_type type_specifier allocation_type qualified_name
    parameter_list_opt parameter_list parameter_declaration
    parameter_declarator_opt parenthesized_parameter_declarator
    init_declarator named_type_declarator init_declarator_list named_type_declarator_list
    array_dimensions_opt array_dimensions initializer_list_opt initializer_list
    allocation_dimensions argument_list_opt argument_list
    initializer_opt initializer braced_init_list
    compound_statement block_items_opt
    local_declaration block_item statement expression_statement
    selection_statement iteration_statement for_initializer jump_statement labeled_statement
    expression_opt expression assignment_expression conditional_expression
    logical_or_expression logical_and_expression bitwise_or_expression
    bitwise_xor_expression bitwise_and_expression equality_expression
    relational_expression shift_expression additive_expression
    multiplicative_expression unary_expression delete_operand
    postfix_expression primary_expression lambda_expression
    allocation_suffix_opt capture_list capture

%nonassoc IDENTIFIER_EXPRESSION
%nonassoc TOK_STAR TOK_BITAND
%nonassoc LOWER_THAN_ELSE
%nonassoc TOK_ELSE

%start program

%%

//top level
program
    : translation_unit                { g_program.reset(new ast::Program(take($1))); }
    ;

translation_unit
    : /* empty */                     { $$ = new NodeList<ast::Decl>(); }
    | translation_unit external_item  { $$ = appendAll($1, $2); }
    ;

external_item
    : record_definition               { $$ = single($1); }
    | class_definition                { $$ = single($1); }
    | function_definition             { $$ = single($1); }
    | function_declaration            { $$ = single($1); }
    | qualified_special_member_definition
                                      { $$ = single($1); }
    | declaration                     { $$ = $1; }
    | static_item                     { $$ = $1; }
    | error TOK_SEMI                  { yyerrok; $$ = new NodeList<ast::Decl>(); }
    ;

//static applies to variables and functions, at file scope and inside a class
static_item
    : TOK_STATIC function_definition  { $$ = single($2); markStatic($$); }
    | TOK_STATIC function_declaration { $$ = single($2); markStatic($$); }
    | TOK_STATIC declaration          { $$ = $2; markStatic($$); }
    ;


qualified_special_member_definition
    : TOK_IDENTIFIER TOK_SCOPE TOK_IDENTIFIER
      TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($3, "PROCEDURE");
                                                        $$ = new ast::ConstructorDecl(memberName($1, lexemeAt($3), $3),
                                                                                      take($5), own($7), LOC(@3)); }
    | TOK_IDENTIFIER TOK_SCOPE TOK_BITNOT TOK_IDENTIFIER
      TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($4, "PROCEDURE");
                                                        $$ = new ast::DestructorDecl(memberName($1, "~" + lexemeAt($4), $4),
                                                                                     take($6), own($8), LOC(@3)); }
    ;

record_definition
    : TOK_STRUCT TOK_IDENTIFIER inheritance_opt TOK_LBRACE record_items_opt TOK_RBRACE TOK_SEMI
                                                      { setDeclType($2, "STRUCT");
                                                        $$ = new ast::RecordDecl(false, lexemeAt($2), takeValue($3),
                                                                                 take($5), LOC(@1)); }
    ;

record_items_opt
    : /* empty */                                     { $$ = new NodeList<ast::Decl>(); }
    | record_items_opt member_item                    { $$ = appendAll($1, $2); }
    ;

class_definition
    : TOK_CLASS TOK_IDENTIFIER inheritance_opt TOK_LBRACE class_items_opt TOK_RBRACE TOK_SEMI
                                                      { setDeclType($2, "CLASS");
                                                        $$ = new ast::RecordDecl(true, lexemeAt($2), takeValue($3),
                                                                                 take($5), LOC(@1)); }
    ;

//the access specifier is optional, as it is in C++, and more than one base may
//be listed
inheritance_opt
    : /* empty */                     { $$ = new std::vector<ast::BaseSpecifier>(); }
    | TOK_COLON base_list             { $$ = $2; }
    ;

base_list
    : base_specifier                  { $$ = new std::vector<ast::BaseSpecifier>();
                                        $$->push_back(takeValue($1)); }
    | base_list TOK_COMMA base_specifier
                                      { $$ = $1; $$->push_back(takeValue($3)); }
    ;

base_specifier
    : access_specifier TOK_IDENTIFIER { $$ = new ast::BaseSpecifier{ (ast::AccessKind)$1, lexemeAt($2) }; }
    | TOK_IDENTIFIER                  { $$ = new ast::BaseSpecifier{ ast::AccessKind::None, lexemeAt($1) }; }
    ;

class_items_opt
    : /* empty */                     { $$ = new NodeList<ast::Decl>(); }
    | class_items_opt class_item      { $$ = appendAll($1, $2); }
    ;

class_item
    : access_specifier TOK_COLON      { $$ = single<ast::Decl>(new ast::AccessSpecDecl((ast::AccessKind)$1, LOC(@1))); }
    | member_item                     { $$ = $1; }
    ;

special_member_definition
    : TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($1, "PROCEDURE");
                                                        $$ = new ast::ConstructorDecl(takeValue(nameOf($1)), take($3),
                                                                                      own($5), LOC(@1)); }
    | TOK_BITNOT TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($2, "PROCEDURE");
                                                        ast::QualifiedName name = takeValue(nameOf($2));
                                                        name.parts.back() = "~" + name.parts.back();
                                                        $$ = new ast::DestructorDecl(std::move(name), take($4),
                                                                                     own($6), LOC(@1)); }
    ;

special_member_declaration
    : TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI
                                                      { setDeclType($1, "PROCEDURE");
                                                        $$ = new ast::ConstructorDecl(takeValue(nameOf($1)), take($3),
                                                                                      nullptr, LOC(@1)); }
    | TOK_BITNOT TOK_IDENTIFIER TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI
                                                      { setDeclType($2, "PROCEDURE");
                                                        ast::QualifiedName name = takeValue(nameOf($2));
                                                        name.parts.back() = "~" + name.parts.back();
                                                        $$ = new ast::DestructorDecl(std::move(name), take($4),
                                                                                     nullptr, LOC(@1)); }
    ;

access_specifier
    : TOK_PUBLIC                      { $$ = (int)ast::AccessKind::Public; }
    | TOK_PRIVATE                     { $$ = (int)ast::AccessKind::Private; }
    | TOK_PROTECTED                   { $$ = (int)ast::AccessKind::Protected; }
    ;

member_item
    : record_definition               { $$ = single($1); }
    | class_definition                { $$ = single($1); }
    | function_definition             { $$ = single($1); }
    | function_declaration            { $$ = single($1); }
    | special_member_definition       { $$ = single($1); }
    | special_member_declaration      { $$ = single($1); }
    | declaration                     { $$ = $1; }
    | static_item                     { $$ = $1; }
    | error TOK_SEMI                  { yyerrok; $$ = new NodeList<ast::Decl>(); }
    ;

//declarations
simple_type
    : TOK_INT                         { $$ = typeSpec(ast::TypeSpecKind::Builtin, $1, LOC(@1)); }
    | TOK_CHAR                        { $$ = typeSpec(ast::TypeSpecKind::Builtin, $1, LOC(@1)); }
    | TOK_FLOAT                       { $$ = typeSpec(ast::TypeSpecKind::Builtin, $1, LOC(@1)); }
    | TOK_DOUBLE                      { $$ = typeSpec(ast::TypeSpecKind::Builtin, $1, LOC(@1)); }
    | TOK_VOID                        { $$ = typeSpec(ast::TypeSpecKind::Builtin, $1, LOC(@1)); }
    | TOK_BOOL                        { $$ = typeSpec(ast::TypeSpecKind::Builtin, $1, LOC(@1)); }
    | TOK_AUTO                        { $$ = typeSpec(ast::TypeSpecKind::Builtin, $1, LOC(@1)); }
    ;

type_specifier
    : simple_type                                     { $$ = $1; }
    | TOK_IDENTIFIER                                  { $$ = typeSpec(ast::TypeSpecKind::Named, $1, LOC(@1)); }
    | TOK_STRUCT TOK_IDENTIFIER                       { $$ = typeSpec(ast::TypeSpecKind::Struct, $2, LOC(@1)); }
    ;

qualified_name
    : TOK_IDENTIFIER                                  { $$ = nameOf($1); }
    | qualified_name TOK_SCOPE TOK_IDENTIFIER         { $$ = $1;
                                                        $$->parts.push_back(lexemeAt($3));
                                                        $$->lastTok = $3; }
    ;

function_definition
    : type_specifier qualified_name TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                                      { setDeclType($2->lastTok, "PROCEDURE");
                                                        $$ = new ast::FunctionDecl(own($1), takeValue($2), take($4),
                                                                                   own($6), LOC(@2)); }
    ;

function_declaration
    : type_specifier qualified_name TOK_LPAREN parameter_list_opt TOK_RPAREN TOK_SEMI
                                                      { setDeclType($2->lastTok, "PROCEDURE");
                                                        $$ = new ast::FunctionDecl(own($1), takeValue($2), take($4),
                                                                                   nullptr, LOC(@2)); }
    ;

parameter_list_opt
    : /* empty */                                     { $$ = new NodeList<ast::ParamDecl>(); }
    | parameter_list                                  { $$ = $1; }
    ;

parameter_list
    : parameter_declaration                           { $$ = single($1); }
    | parameter_list TOK_COMMA parameter_declaration  { $$ = append($1, $3); }
    ;

parameter_declaration
    : type_specifier parameter_declarator_opt         { applyPendingType(typeNameAt($1->tokIndex));
                                                        $$ = makeParam($1, $2, LOC(@1)); }
    ;

parameter_declarator_opt
    : /* empty */                                     { $$ = nullptr; }
    | TOK_IDENTIFIER array_dimensions_opt             { pendName($1, 0, 0, (int)$2->size());
                                                        $$ = makeDeclarator(nameOf($1), LOC(@1), 0, false, $2, nullptr); }
    | TOK_STAR TOK_IDENTIFIER array_dimensions_opt    { pendName($2, 1, 0, (int)$3->size());
                                                        $$ = makeDeclarator(nameOf($2), LOC(@2), 1, false, $3, nullptr); }
    | TOK_BITAND TOK_IDENTIFIER array_dimensions_opt  { pendName($2, 0, 1, (int)$3->size());
                                                        $$ = makeDeclarator(nameOf($2), LOC(@2), 0, true, $3, nullptr); }
    | parenthesized_parameter_declarator              { $$ = $1; }
    ;

parenthesized_parameter_declarator
    : TOK_LPAREN TOK_STAR parameter_name_opt TOK_RPAREN
      TOK_LPAREN parameter_list_opt TOK_RPAREN        { setDeclType($3, "PROCEDURE");
                                                        $$ = makeDeclarator(nameOf($3), LOC(@1), 0, false, nullptr, nullptr);
                                                        $$->form = ast::DeclaratorForm::FunctionPointer;
                                                        $$->fnParams = take($6); }
    | TOK_LPAREN TOK_STAR parameter_name_opt TOK_RPAREN array_dimensions
                                                      { pendName($3, 1, 0, (int)$5->size());
                                                        $$ = makeDeclarator(nameOf($3), LOC(@1), 0, false, $5, nullptr);
                                                        $$->form = ast::DeclaratorForm::PointerToArray; }
    | TOK_LPAREN TOK_BITAND parameter_name_opt TOK_RPAREN array_dimensions
                                                      { pendName($3, 0, 1, (int)$5->size());
                                                        $$ = makeDeclarator(nameOf($3), LOC(@1), 0, false, $5, nullptr);
                                                        $$->form = ast::DeclaratorForm::ReferenceToArray; }
    ;

parameter_name_opt
    : /* empty */                                     { $$ = -1; }
    | TOK_IDENTIFIER                                  { $$ = $1; }
    ;

declaration
    : type_specifier init_declarator_list TOK_SEMI
                                                      { applyPendingType(typeNameAt($1->tokIndex));
                                                        $$ = makeVarDecls($1, $2); }
    | parenthesized_declarator TOK_SEMI               { $$ = single($1); }
    ;

parenthesized_declarator
    : simple_type TOK_LPAREN TOK_STAR TOK_IDENTIFIER TOK_RPAREN
      TOK_LPAREN parameter_list_opt TOK_RPAREN initializer_opt
                                                      { setDeclType($4, "PROCEDURE");
                                                        ast::Declarator *declarator =
                                                            makeDeclarator(nameOf($4), LOC(@4), 0, false, nullptr, $9);
                                                        declarator->form = ast::DeclaratorForm::FunctionPointer;
                                                        declarator->fnParams = take($7);
                                                        $$ = makeVar(own($1), declarator); }
    | simple_type TOK_LPAREN TOK_STAR TOK_IDENTIFIER TOK_RPAREN
      array_dimensions initializer_opt
                                                      { pendName($4, 1, 0, (int)$6->size());
                                                        applyPendingType(typeNameAt($1->tokIndex));
                                                        ast::Declarator *declarator =
                                                            makeDeclarator(nameOf($4), LOC(@4), 0, false, $6, $7);
                                                        declarator->form = ast::DeclaratorForm::PointerToArray;
                                                        $$ = makeVar(own($1), declarator); }
    ;

init_declarator_list
    : init_declarator                                 { $$ = single($1); }
    | init_declarator_list TOK_COMMA init_declarator  { $$ = append($1, $3); }
    ;

init_declarator
    : qualified_name array_dimensions_opt initializer_opt
                                                      { pendName($1->lastTok, 0, 0, (int)$2->size());
                                                        $$ = makeDeclarator($1, LOC(@1), 0, false, $2, $3); }
    | TOK_STAR qualified_name array_dimensions_opt initializer_opt
                                                      { pendName($2->lastTok, 1, 0, (int)$3->size());
                                                        $$ = makeDeclarator($2, LOC(@2), 1, false, $3, $4); }
    | TOK_BITAND qualified_name array_dimensions_opt initializer_opt
                                                      { pendName($2->lastTok, 0, 1, (int)$3->size());
                                                        $$ = makeDeclarator($2, LOC(@2), 0, true, $3, $4); }
    ;

array_dimensions_opt
    : /* empty */                                     { $$ = new NodeList<ast::Expr>(); }
    | array_dimensions                                { $$ = $1; }
    ;

//an unsized [] keeps a null entry
array_dimensions
    : TOK_LBRACKET expression_opt TOK_RBRACKET        { $$ = single($2); }
    | array_dimensions TOK_LBRACKET expression_opt TOK_RBRACKET
                                                      { $$ = append($1, $3); }
    ;

//both copy initialisation, int a = 1, and list initialisation, int a{1}
initializer_opt
    : /* empty */                     { $$ = nullptr; }
    | TOK_ASSIGN initializer          { $$ = new ast::Initializer{ ast::InitStyle::Copy, own($2) }; }
    | braced_init_list                { $$ = new ast::Initializer{ ast::InitStyle::Brace, own($1) }; }
    ;

initializer
    : assignment_expression           { $$ = $1; }
    | braced_init_list                { $$ = $1; }
    ;

braced_init_list
    : TOK_LBRACE initializer_list_opt TOK_RBRACE
                                      { $$ = new ast::InitListExpr(take($2), LOC(@1)); }
    ;

initializer_list_opt
    : /* empty */                     { $$ = new NodeList<ast::Expr>(); }
    | initializer_list                { $$ = $1; }
    | initializer_list TOK_COMMA      { $$ = $1; }
    ;

initializer_list
    : initializer                     { $$ = single($1); }
    | initializer_list TOK_COMMA initializer
                                      { $$ = append($1, $3); }
    ;

//local declarations
local_declaration
    : local_declaration_body TOK_SEMI { $$ = new ast::DeclStmt(take($1), LOC(@1)); }
    | TOK_STATIC local_declaration_body TOK_SEMI
                                      { markStatic($2); $$ = new ast::DeclStmt(take($2), LOC(@1)); }
    ;

local_declaration_body
    : parenthesized_declarator                        { $$ = single($1); }
    | simple_type init_declarator_list                { applyPendingType(typeNameAt($1->tokIndex));
                                                        $$ = makeVarDecls($1, $2); }
    | TOK_STRUCT TOK_IDENTIFIER init_declarator_list  { applyPendingType(typeNameAt($2));
                                                        $$ = makeVarDecls(typeSpec(ast::TypeSpecKind::Struct, $2, LOC(@1)), $3); }
    | TOK_IDENTIFIER named_type_declarator_list       { applyPendingType(typeNameAt($1));
                                                        $$ = makeVarDecls(typeSpec(ast::TypeSpecKind::Named, $1, LOC(@1)), $2); }
    ;

named_type_declarator_list
    : named_type_declarator                           { $$ = single($1); }
    | named_type_declarator_list TOK_COMMA named_type_declarator
                                                      { $$ = append($1, $3); }
    ;

//the parenthesised form is offered only here, in the declarator used for local
//declarations of a named type. a function cannot be declared inside a block, so
//Dog d(3) is unambiguous there. at file scope the same shape would collide with
//a function declaration, which is the most vexing parse, so it is not offered
named_type_declarator
    : TOK_IDENTIFIER array_dimensions_opt initializer_opt
                                                      { pendName($1, 0, 0, (int)$2->size());
                                                        $$ = makeDeclarator(nameOf($1), LOC(@1), 0, false, $2, $3); }
    | TOK_IDENTIFIER TOK_LPAREN argument_list_opt TOK_RPAREN
                                                      { pendName($1, 0, 0, 0);
                                                        $$ = makeDeclarator(nameOf($1), LOC(@1), 0, false, nullptr, nullptr);
                                                        $$->initStyle = ast::InitStyle::CtorArgs;
                                                        $$->ctorArgs = take($3); }
    | TOK_STAR TOK_IDENTIFIER array_dimensions_opt initializer_opt
                                                      { pendName($2, 1, 0, (int)$3->size());
                                                        $$ = makeDeclarator(nameOf($2), LOC(@2), 1, false, $3, $4); }
    | TOK_BITAND TOK_IDENTIFIER array_dimensions_opt initializer_opt
                                                      { pendName($2, 0, 1, (int)$3->size());
                                                        $$ = makeDeclarator(nameOf($2), LOC(@2), 0, true, $3, $4); }
    ;

//statements
compound_statement
    : TOK_LBRACE block_items_opt TOK_RBRACE
                                      { $$ = new ast::CompoundStmt(take($2), LOC(@1)); }
    ;

block_items_opt
    : /* empty */                     { $$ = new NodeList<ast::Stmt>(); }
    | block_items_opt block_item      { $$ = $1; if ($2) $$->emplace_back($2); }
    ;

block_item
    : local_declaration               { $$ = $1; }
    | statement                       { $$ = $1; }
    | error TOK_SEMI                  { yyerrok; $$ = nullptr; }
    ;

statement
    : compound_statement              { $$ = $1; }
    | expression_statement            { $$ = $1; }
    | selection_statement             { $$ = $1; }
    | iteration_statement             { $$ = $1; }
    | jump_statement                  { $$ = $1; }
    | labeled_statement               { $$ = $1; }
    ;

expression_statement
    : expression_opt TOK_SEMI         { if ($1) $$ = new ast::ExprStmt(own($1), LOC(@1));
                                        else    $$ = new ast::ExprStmt(nullptr, LOC(@2)); }
    ;

selection_statement
    : TOK_IF TOK_LPAREN expression TOK_RPAREN statement %prec LOWER_THAN_ELSE
                                      { $$ = new ast::IfStmt(own($3), own($5), nullptr, LOC(@1)); }
    | TOK_IF TOK_LPAREN expression TOK_RPAREN statement TOK_ELSE statement
                                      { $$ = new ast::IfStmt(own($3), own($5), own($7), LOC(@1)); }
    | TOK_SWITCH TOK_LPAREN expression TOK_RPAREN statement
                                      { $$ = new ast::SwitchStmt(own($3), own($5), LOC(@1)); }
    ;

iteration_statement
    : TOK_WHILE TOK_LPAREN expression TOK_RPAREN statement
                                      { $$ = new ast::WhileStmt(own($3), own($5), LOC(@1)); }
    | TOK_DO statement TOK_WHILE TOK_LPAREN expression TOK_RPAREN TOK_SEMI
                                      { $$ = new ast::DoStmt(own($2), own($5), LOC(@1)); }
    | TOK_FOR TOK_LPAREN for_initializer TOK_SEMI expression_opt TOK_SEMI
      expression_opt TOK_RPAREN statement
                                      { $$ = new ast::ForStmt(own($3), own($5), own($7), own($9), LOC(@1)); }
    ;

for_initializer
    : /* empty */                     { $$ = nullptr; }
    | expression                      { $$ = new ast::ExprStmt(own($1), LOC(@1)); }
    | local_declaration_body          { $$ = new ast::DeclStmt(take($1), LOC(@1)); }
    ;

jump_statement
    : TOK_RETURN expression_opt TOK_SEMI
                                      { $$ = new ast::ReturnStmt(own($2), LOC(@1)); }
    | TOK_BREAK TOK_SEMI              { $$ = new ast::BreakStmt(LOC(@1)); }
    | TOK_CONTINUE TOK_SEMI           { $$ = new ast::ContinueStmt(LOC(@1)); }
    | TOK_GOTO TOK_IDENTIFIER TOK_SEMI                { setDeclType($2, "LABEL");
                                                        $$ = new ast::GotoStmt(lexemeAt($2), LOC(@1)); }
    ;

labeled_statement
    : TOK_IDENTIFIER TOK_COLON statement              { setDeclType($1, "LABEL");
                                                        $$ = new ast::LabelStmt(lexemeAt($1), own($3), LOC(@1)); }
    | TOK_CASE conditional_expression TOK_COLON statement
                                      { $$ = new ast::CaseStmt(own($2), own($4), LOC(@1)); }
    | TOK_DEFAULT TOK_COLON statement { $$ = new ast::DefaultStmt(own($3), LOC(@1)); }
    ;

//expressions
expression_opt
    : /* empty */                     { $$ = nullptr; }
    | expression                      { $$ = $1; }
    ;

expression
    : assignment_expression           { $$ = $1; }
    ;

assignment_expression
    : conditional_expression          { $$ = $1; }
    | unary_expression assignment_operator assignment_expression
                                      { $$ = new ast::AssignExpr((ast::AssignOp)$2, own($1), own($3), LOC(@2)); }
    ;

assignment_operator
    : TOK_ASSIGN                      { $$ = (int)ast::AssignOp::Assign; }
    | TOK_PLUS_ASSIGN                 { $$ = (int)ast::AssignOp::AddAssign; }
    | TOK_MINUS_ASSIGN                { $$ = (int)ast::AssignOp::SubAssign; }
    | TOK_STAR_ASSIGN                 { $$ = (int)ast::AssignOp::MulAssign; }
    | TOK_SLASH_ASSIGN                { $$ = (int)ast::AssignOp::DivAssign; }
    | TOK_PERCENT_ASSIGN              { $$ = (int)ast::AssignOp::RemAssign; }
    ;

conditional_expression
    : logical_or_expression           { $$ = $1; }
    | logical_or_expression TOK_QUESTION expression TOK_COLON conditional_expression
                                      { $$ = new ast::ConditionalExpr(own($1), own($3), own($5), LOC(@2)); }
    ;

logical_or_expression
    : logical_and_expression          { $$ = $1; }
    | logical_or_expression TOK_OR logical_and_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::LogicalOr, own($1), own($3), LOC(@2)); }
    ;

logical_and_expression
    : bitwise_or_expression           { $$ = $1; }
    | logical_and_expression TOK_AND bitwise_or_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::LogicalAnd, own($1), own($3), LOC(@2)); }
    ;

bitwise_or_expression
    : bitwise_xor_expression          { $$ = $1; }
    | bitwise_or_expression TOK_BITOR bitwise_xor_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::BitOr, own($1), own($3), LOC(@2)); }
    ;

bitwise_xor_expression
    : bitwise_and_expression          { $$ = $1; }
    | bitwise_xor_expression TOK_BITXOR bitwise_and_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::BitXor, own($1), own($3), LOC(@2)); }
    ;

bitwise_and_expression
    : equality_expression             { $$ = $1; }
    | bitwise_and_expression TOK_BITAND equality_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::BitAnd, own($1), own($3), LOC(@2)); }
    ;

equality_expression
    : relational_expression           { $$ = $1; }
    | equality_expression TOK_EQ relational_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Eq, own($1), own($3), LOC(@2)); }
    | equality_expression TOK_NEQ relational_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Ne, own($1), own($3), LOC(@2)); }
    ;

relational_expression
    : shift_expression                { $$ = $1; }
    | relational_expression TOK_LT shift_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Lt, own($1), own($3), LOC(@2)); }
    | relational_expression TOK_GT shift_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Gt, own($1), own($3), LOC(@2)); }
    | relational_expression TOK_LE shift_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Le, own($1), own($3), LOC(@2)); }
    | relational_expression TOK_GE shift_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Ge, own($1), own($3), LOC(@2)); }
    ;

shift_expression
    : additive_expression             { $$ = $1; }
    | shift_expression TOK_SHL additive_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Shl, own($1), own($3), LOC(@2)); }
    | shift_expression TOK_SHR additive_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Shr, own($1), own($3), LOC(@2)); }
    ;

additive_expression
    : multiplicative_expression       { $$ = $1; }
    | additive_expression TOK_PLUS multiplicative_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Add, own($1), own($3), LOC(@2)); }
    | additive_expression TOK_MINUS multiplicative_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Sub, own($1), own($3), LOC(@2)); }
    ;

multiplicative_expression
    : unary_expression                { $$ = $1; }
    | multiplicative_expression TOK_STAR unary_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Mul, own($1), own($3), LOC(@2)); }
    | multiplicative_expression TOK_SLASH unary_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Div, own($1), own($3), LOC(@2)); }
    | multiplicative_expression TOK_PERCENT unary_expression
                                      { $$ = new ast::BinaryExpr(ast::BinaryOp::Rem, own($1), own($3), LOC(@2)); }
    ;

unary_expression
    : postfix_expression              { $$ = $1; }
    | TOK_INC unary_expression        { $$ = new ast::UnaryExpr(ast::UnaryOp::PreInc, own($2), LOC(@1)); }
    | TOK_DEC unary_expression        { $$ = new ast::UnaryExpr(ast::UnaryOp::PreDec, own($2), LOC(@1)); }
    | TOK_PLUS unary_expression       { $$ = new ast::UnaryExpr(ast::UnaryOp::Plus, own($2), LOC(@1)); }
    | TOK_MINUS unary_expression      { $$ = new ast::UnaryExpr(ast::UnaryOp::Minus, own($2), LOC(@1)); }
    | TOK_NOT unary_expression        { $$ = new ast::UnaryExpr(ast::UnaryOp::LogicalNot, own($2), LOC(@1)); }
    | TOK_BITNOT unary_expression     { $$ = new ast::UnaryExpr(ast::UnaryOp::BitNot, own($2), LOC(@1)); }
    | TOK_STAR unary_expression       { $$ = new ast::UnaryExpr(ast::UnaryOp::Deref, own($2), LOC(@1)); }
    | TOK_BITAND unary_expression     { $$ = new ast::UnaryExpr(ast::UnaryOp::AddrOf, own($2), LOC(@1)); }
    | TOK_NEW allocation_type allocation_suffix_opt
                                      { $3->type.reset($2);
                                        $3->line = @1.first_line;
                                        $3->col = @1.first_column;
                                        $$ = $3; }
    | TOK_DELETE unary_expression     { $$ = new ast::DeleteExpr(false, own($2), LOC(@1)); }
    | TOK_DELETE TOK_LBRACKET TOK_RBRACKET delete_operand
                                      { $$ = new ast::DeleteExpr(true, own($4), LOC(@1)); }
    ;

delete_operand
    : TOK_IDENTIFIER                  { $$ = new ast::DeclRefExpr(lexemeAt($1), LOC(@1)); }
    | TOK_THIS                        { $$ = new ast::ThisExpr(LOC(@1)); }
    | delete_operand TOK_DOT TOK_IDENTIFIER
                                      { $$ = new ast::MemberExpr(own($1), lexemeAt($3), false, LOC(@2)); }
    | delete_operand TOK_ARROW TOK_IDENTIFIER
                                      { $$ = new ast::MemberExpr(own($1), lexemeAt($3), true, LOC(@2)); }
    | delete_operand TOK_LBRACKET expression TOK_RBRACKET
                                      { $$ = new ast::ArraySubscriptExpr(own($1), own($3), LOC(@2)); }
    ;

allocation_type
    : simple_type                     { $$ = $1; }
    | TOK_IDENTIFIER                  { $$ = typeSpec(ast::TypeSpecKind::Named, $1, LOC(@1)); }
    | TOK_STRUCT TOK_IDENTIFIER       { $$ = typeSpec(ast::TypeSpecKind::Struct, $2, LOC(@1)); }
    ;

//builds the NewExpr without its type, the new rule above fills that in
allocation_suffix_opt
    : /* empty */                     { $$ = new ast::NewExpr(ast::NewForm::Plain, 0, 0); }
    | TOK_LPAREN argument_list_opt TOK_RPAREN
                                      { $$ = new ast::NewExpr(ast::NewForm::Args, LOC(@1));
                                        $$->args = take($2); }
    | braced_init_list                { $$ = new ast::NewExpr(ast::NewForm::InitList, LOC(@1));
                                        $$->init.reset($1); }
    | allocation_dimensions           { $$ = new ast::NewExpr(ast::NewForm::Array, LOC(@1));
                                        $$->dims = take($1); }
    | allocation_dimensions braced_init_list
                                      { $$ = new ast::NewExpr(ast::NewForm::ArrayInit, LOC(@1));
                                        $$->dims = take($1);
                                        $$->init.reset($2); }
    ;

allocation_dimensions
    : TOK_LBRACKET expression TOK_RBRACKET
                                      { $$ = single($2); }
    | allocation_dimensions TOK_LBRACKET expression TOK_RBRACKET
                                      { $$ = append($1, $3); }
    ;

postfix_expression
    : primary_expression              { $$ = $1; }
    | postfix_expression TOK_LBRACKET expression TOK_RBRACKET
                                      { $$ = new ast::ArraySubscriptExpr(own($1), own($3), LOC(@2)); }
    | postfix_expression TOK_LPAREN argument_list_opt TOK_RPAREN
                                      { $$ = new ast::CallExpr(own($1), take($3), LOC(@2)); }
    | postfix_expression TOK_DOT TOK_IDENTIFIER
                                      { $$ = new ast::MemberExpr(own($1), lexemeAt($3), false, LOC(@2)); }
    | postfix_expression TOK_ARROW TOK_IDENTIFIER
                                      { $$ = new ast::MemberExpr(own($1), lexemeAt($3), true, LOC(@2)); }
    | postfix_expression TOK_INC      { $$ = new ast::UnaryExpr(ast::UnaryOp::PostInc, own($1), LOC(@2)); }
    | postfix_expression TOK_DEC      { $$ = new ast::UnaryExpr(ast::UnaryOp::PostDec, own($1), LOC(@2)); }
    ;

argument_list_opt
    : /* empty */                     { $$ = new NodeList<ast::Expr>(); }
    | argument_list                   { $$ = $1; }
    ;

argument_list
    : assignment_expression           { $$ = single($1); }
    | argument_list TOK_COMMA assignment_expression
                                      { $$ = append($1, $3); }
    ;

primary_expression
    : TOK_IDENTIFIER %prec IDENTIFIER_EXPRESSION
                                      { $$ = new ast::DeclRefExpr(lexemeAt($1), LOC(@1)); }
    | TOK_INT_LITERAL                 { $$ = new ast::IntLiteral(lexemeAt($1), LOC(@1)); }
    | TOK_FLOAT_LITERAL               { $$ = new ast::FloatLiteral(lexemeAt($1), LOC(@1)); }
    | TOK_CHAR_LITERAL                { $$ = new ast::CharLiteral(lexemeAt($1), LOC(@1)); }
    | TOK_STRING_LITERAL              { $$ = new ast::StringLiteral(lexemeAt($1), LOC(@1)); }
    | TOK_TRUE                        { $$ = new ast::BoolLiteral(true, LOC(@1)); }
    | TOK_FALSE                       { $$ = new ast::BoolLiteral(false, LOC(@1)); }
    | TOK_NULL                        { $$ = new ast::NullLiteral(LOC(@1)); }
    | TOK_THIS                        { $$ = new ast::ThisExpr(LOC(@1)); }
    | TOK_LPAREN expression TOK_RPAREN
                                      { $$ = new ast::ParenExpr(own($2), LOC(@1)); }
    | lambda_expression               { $$ = $1; }
    ;

lambda_expression
    : TOK_LBRACKET TOK_RBRACKET TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                      { $$ = new ast::LambdaExpr(ast::CaptureDefault::None, {}, take($4),
                                                                 own($6), LOC(@1)); }
    | TOK_LBRACKET capture_default TOK_RBRACKET TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                      { $$ = new ast::LambdaExpr((ast::CaptureDefault)$2, {}, take($5),
                                                                 own($7), LOC(@1)); }
    | TOK_LBRACKET capture_list TOK_RBRACKET TOK_LPAREN parameter_list_opt TOK_RPAREN compound_statement
                                      { $$ = new ast::LambdaExpr(ast::CaptureDefault::None, takeValue($2),
                                                                 take($5), own($7), LOC(@1)); }
    ;

capture_default
    : TOK_ASSIGN                      { $$ = (int)ast::CaptureDefault::ByCopy; }
    | TOK_BITAND                      { $$ = (int)ast::CaptureDefault::ByRef; }
    ;

capture_list
    : capture                         { $$ = new std::vector<ast::LambdaCapture>();
                                        $$->push_back(takeValue($1)); }
    | capture_list TOK_COMMA capture  { $$ = $1; $$->push_back(takeValue($3)); }
    ;

capture
    : TOK_IDENTIFIER                  { $$ = new ast::LambdaCapture{ ast::LambdaCapture::Kind::ByCopy, lexemeAt($1) }; }
    | TOK_BITAND TOK_IDENTIFIER       { $$ = new ast::LambdaCapture{ ast::LambdaCapture::Kind::ByRef, lexemeAt($2) }; }
    | TOK_THIS                        { $$ = new ast::LambdaCapture{ ast::LambdaCapture::Kind::This, "" }; }
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

//writes the tree to outputs/<source name>-ast.txt
static void writeAst(const char *sourcePath) {
    std::filesystem::path outDir("outputs");
    std::error_code ec;
    std::filesystem::create_directories(outDir, ec);

    std::filesystem::path astPath = outDir / (std::filesystem::path(sourcePath).stem().string() + "-ast.txt");
    std::ofstream astFile(astPath);
    if (!astFile) {
        cerr << "error: cannot write " << astPath.string() << "\n";
        return;
    }
    ast::ASTPrinter printer(astFile);
    printer.print(g_program.get());
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
    if (g_program) {
        writeAst(argv[1]);
    }
    return 0;
}
