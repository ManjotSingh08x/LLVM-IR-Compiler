#ifndef IR_CONSTANT_FOLDER_H
#define IR_CONSTANT_FOLDER_H

#include "ir/cfg.h"
#include "ir/record_layout.h"

namespace ir {

//a number worked out while compiling, with the type it has at that point
struct FoldedNumber {
    bool              isFloat = false;
    long long         intValue = 0;
    double            floatValue = 0.0;
    const sema::Type *type = nullptr;
};

//works out the constant a global (or static local) starts with, so it can be
//written into the global's definition: @global_5limit = internal global i32 10.
//when the initializer needs code to run, the folder answers null and the
//caller runs the initializer at startup instead.
//
//sema::evaluateConstant is not enough here: it looks through the implicit
//casts without applying them, so bool b = 4 would give i1 4 (which llvm
//silently reads as false) and float f = 2 would give "float 2" (a parse
//error). this folder applies every cast to the value it carries
class ConstantFolder {
public:
    ConstantFolder(Module &module, RecordLayout &layout) : module(module), layout(layout) {}

    //the constant for a whole declaration, int a[4]{1} or Point p = {1, 2},
    //or null when its initializer must run as code
    Constant *foldVariable(ast::VarDecl *decl, const sema::Type *type);

    //the constant for one value converted to type, or null
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

} // namespace ir

#endif
