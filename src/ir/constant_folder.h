#ifndef IR_CONSTANT_FOLDER_H
#define IR_CONSTANT_FOLDER_H

#include "ir/cfg.h"
#include "ir/record_layout.h"

namespace ir {

struct FoldedNumber {
    bool              isFloat = false;
    long long         intValue = 0;
    double            floatValue = 0.0;
    const sema::Type *type = nullptr;
};

//evaluates initializers at compile time
class ConstantFolder {
public:
    ConstantFolder(Module &module, RecordLayout &layout) : module(module), layout(layout) {}

    //folds a global initializer into a constant
    Constant *foldVariable(ast::VarDecl *decl, const sema::Type *type);

    Constant *fold(ast::Expr *expr, const sema::Type *type);

private:
    Constant *foldList(ast::InitListExpr *list, const sema::Type *type);
    Constant *foldStringArray(ast::StringLiteral *literal, const sema::ArrayType *array);
    Constant *foldPointer(ast::Expr *expr);
    Constant *foldAddress(ast::Expr *expr);

    bool foldNumber(ast::Expr *expr, FoldedNumber &out);
    bool foldUnary(ast::UnaryExpr *expr, FoldedNumber &out);
    bool foldBinary(ast::BinaryExpr *expr, FoldedNumber &out);
    FoldedNumber convertNumber(const FoldedNumber &number, const sema::Type *to);
    Constant    *numberConstant(const FoldedNumber &number);

    Module       &module;
    RecordLayout &layout;
};

}

#endif
