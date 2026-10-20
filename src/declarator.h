#ifndef DECLARATOR_H
#define DECLARATOR_H

#include "ast/ast.h"

//holders the parser uses while it reads a declaration. they never reach the
//tree: once the type name in front of the declarators is known, each one is
//joined with it into a VarDecl or ParamDecl

//the initializer as parsed, before it is attached to a declarator
struct Initializer {
    ast::InitStyle             style = ast::InitStyle::None;
    std::unique_ptr<ast::Expr> value;
};

//everything after the type name in one declarator. type holds the part of the
//type the declarator writes, *p[3] gives Array [3] -> Pointer -> (hole). its
//innermost inner is left null as the hole the type name is put into later. a
//declarator that is only a name leaves type null
struct Declarator {
    ast::QualifiedName             name;  //empty for an unnamed parameter
    int                            line = 0;
    int                            col = 0;
    std::unique_ptr<ast::TypeSpec> type;
    ast::InitStyle                 initStyle = ast::InitStyle::None;
    std::unique_ptr<ast::Expr>     init;      //for Copy and Brace
    ast::NodeList<ast::Expr>       ctorArgs;  //for CtorArgs

    void setInitializer(Initializer *initializer) {
        if (initializer == nullptr) return;
        initStyle = initializer->style;
        init = std::move(initializer->value);
        delete initializer;
    }
};

#endif
