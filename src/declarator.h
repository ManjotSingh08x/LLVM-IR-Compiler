#ifndef DECLARATOR_H
#define DECLARATOR_H

#include "ast/ast.h"

struct Initializer {
    ast::InitStyle             style = ast::InitStyle::None;
    std::unique_ptr<ast::Expr> value;
};

struct Declarator {
    ast::QualifiedName             name;
    int                            line = 0;
    int                            col = 0;
    std::unique_ptr<ast::TypeSpec> type;
    ast::InitStyle                 initStyle = ast::InitStyle::None;
    std::unique_ptr<ast::Expr>     init;
    ast::NodeList<ast::Expr>       ctorArgs;

    void setInitializer(Initializer *initializer) {
        if (initializer == nullptr) return;
        initStyle = initializer->style;
        init = std::move(initializer->value);
        delete initializer;
    }
};

#endif
