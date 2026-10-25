#ifndef DECLARATOR_H
#define DECLARATOR_H

#include "ast/ast.h"

//parser-only holders, joined with the type name into a VarDecl or ParamDecl

//the initializer as parsed, before it is attached to a declarator
struct Initializer {
    ast::InitStyle             style = ast::InitStyle::None;
    std::unique_ptr<ast::Expr> value;
};

//everything after the type name, type's innermost null inner is the hole for the type name
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
