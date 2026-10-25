#include "ir/constant_folder.h"

#include <cstdlib>

#include "semantic/constants.h"

namespace ir {

namespace {

ast::Expr *withoutParens(ast::Expr *expr) {
    while (expr->getKind() == ast::ASTKind::ParenExpr) expr = static_cast<ast::ParenExpr *>(expr)->sub.get();
    return expr;
}

bool isArithmeticCast(ast::CastKind kind) {
    switch (kind) {
    case ast::CastKind::IntegralCast:
    case ast::CastKind::IntegralToFloating:
    case ast::CastKind::FloatingToIntegral:
    case ast::CastKind::FloatingCast:
    case ast::CastKind::ToBool:
        return true;
    default:
        return false;
    }
}

FoldedNumber makeInt(long long value, const sema::Type *type) {
    FoldedNumber number;
    number.intValue = normalizeInt(value, type);
    number.type = type;
    return number;
}

FoldedNumber makeFloat(double value, const sema::Type *type) {
    FoldedNumber number;
    number.isFloat = true;
    number.floatValue = type->is(sema::TypeKind::Float) ? (double)(float)value : value;
    number.type = type;
    return number;
}

} // namespace

// ---- whole declarations and values ----

Constant *ConstantFolder::foldVariable(ast::VarDecl *decl, const sema::Type *type) {
    //a constructor, or a member that needs one, always runs as code
    if (decl->constructor != nullptr || layout.needsConstruction(type)) return nullptr;

    switch (decl->initStyle) {
    case ast::InitStyle::None:
        return module.getZero(type);  //globals start as zero in c++
    case ast::InitStyle::Copy:
    case ast::InitStyle::Brace:
        return fold(decl->init.get(), type);
    case ast::InitStyle::CtorArgs:
        if (decl->ctorArgs.size() != 1) return nullptr;
        return fold(decl->ctorArgs[0].get(), type);
    }
    return nullptr;
}

Constant *ConstantFolder::fold(ast::Expr *expr, const sema::Type *type) {
    expr = withoutParens(expr);

    //a reference holds the address of what it is bound to
    if (sema::asReference(type)) return foldAddress(expr);

    if (expr->getKind() == ast::ASTKind::InitListExpr) {
        return foldList(static_cast<ast::InitListExpr *>(expr), type);
    }

    switch (type->getKind()) {
    case sema::TypeKind::Array:
        if (expr->getKind() == ast::ASTKind::StringLiteral) {
            return foldStringArray(static_cast<ast::StringLiteral *>(expr), sema::asArray(type));
        }
        return nullptr;

    //copying another object is not a constant
    case sema::TypeKind::Record:
    case sema::TypeKind::Closure:
        return nullptr;

    case sema::TypeKind::Pointer:
        return foldPointer(expr);

    default: {
        FoldedNumber number;
        if (!foldNumber(expr, number)) return nullptr;
        return numberConstant(convertNumber(number, type));
    }
    }
}

//{ ... } for an array, a plain record or a single value, missing elements are zero
Constant *ConstantFolder::foldList(ast::InitListExpr *list, const sema::Type *type) {
    if (list->constructor != nullptr) return nullptr;

    if (const sema::ArrayType *array = sema::asArray(type)) {
        if (list->inits.empty()) return module.getZero(type);
        std::vector<const Constant *> elements;
        for (uint64_t i = 0; i < array->size; i++) {
            Constant *element = nullptr;
            if (i < list->inits.size()) {
                element = fold(list->inits[i].get(), array->element);
            } else if (!layout.needsConstruction(array->element)) {
                element = module.getZero(array->element);
            }
            if (element == nullptr) return nullptr;
            elements.push_back(element);
        }
        return module.getAggregate(type, elements);
    }

    if (sema::RecordSymbol *record = sema::asRecord(type)) {
        if (!record->bases.empty()) return nullptr;
        std::vector<const Constant *> elements;
        for (size_t i = 0; i < record->fields.size(); i++) {
            const sema::Symbol *field = record->fields[i];
            Constant *element = nullptr;
            if (i < list->inits.size()) {
                element = fold(list->inits[i].get(), field->type);
            } else if (ast::VarDecl *fieldDecl = RecordLayout::fieldDeclaration(field)) {
                element = foldVariable(fieldDecl, field->type);
            } else if (!layout.needsConstruction(field->type)) {
                element = module.getZero(field->type);
            }
            if (element == nullptr) return nullptr;
            elements.push_back(element);
        }
        if (elements.empty()) return module.getZero(type);
        return module.getAggregate(type, elements);
    }

    //int a{5} or int b{}
    if (list->inits.empty()) return module.getZero(type);
    return fold(list->inits[0].get(), type);
}

//char s[8] = "hi" is the characters, the \0, and zeros up to the size
Constant *ConstantFolder::foldStringArray(ast::StringLiteral *literal, const sema::ArrayType *array) {
    std::string bytes = sema::decodeStringLiteral(literal->spelling);
    bytes.resize(array->size, '\0');
    return module.getString(array, bytes);
}

Constant *ConstantFolder::foldPointer(ast::Expr *expr) {
    expr = withoutParens(expr);
    switch (expr->getKind()) {
    case ast::ASTKind::NullLiteral:
        return module.getNull(module.pointerType());

    case ast::ASTKind::UnaryExpr: {
        ast::UnaryExpr *unary = static_cast<ast::UnaryExpr *>(expr);
        if (unary->op == ast::UnaryOp::AddrOf) return foldAddress(unary->operand.get());
        return nullptr;
    }

    case ast::ASTKind::ImplicitCastExpr: {
        ast::ImplicitCastExpr *cast = static_cast<ast::ImplicitCastExpr *>(expr);
        switch (cast->castKind) {
        case ast::CastKind::NullToPointer:
            return module.getNull(module.pointerType());
        case ast::CastKind::ArrayToPointer:     //the first element is at the array's address
        case ast::CastKind::FunctionToPointer:
            return foldAddress(cast->sub.get());
        case ast::CastKind::PointerToVoid:      //the same address, only the type changes
            return foldPointer(cast->sub.get());
        default:
            return nullptr;  //a base class conversion may move the address
        }
    }

    default:
        return nullptr;
    }
}

//the address of a string literal, a global variable or a function
Constant *ConstantFolder::foldAddress(ast::Expr *expr) {
    expr = withoutParens(expr);

    if (expr->getKind() == ast::ASTKind::StringLiteral) {
        std::string bytes = sema::decodeStringLiteral(static_cast<ast::StringLiteral *>(expr)->spelling);
        bytes.push_back('\0');
        return module.getAddress(module.getStringLiteral(bytes));
    }

    if (expr->getKind() != ast::ASTKind::DeclRefExpr) return nullptr;
    sema::Symbol *symbol = static_cast<ast::DeclRefExpr *>(expr)->symbol;
    if (symbol == nullptr || symbol->mangledName.empty()) return nullptr;  //locals have no global name

    if (symbol->kind == sema::SymbolKind::Function) {
        Function *fn = module.findFunction(symbol->mangledName);
        if (fn == nullptr) return nullptr;
        fn->isUsed = true;
        return module.getAddress(fn);
    }
    //a global reference would have to be read first, which is not a constant
    if (symbol->kind != sema::SymbolKind::Variable || sema::asReference(symbol->type)) return nullptr;
    GlobalVariable *global = module.findGlobal(symbol->mangledName);
    if (global == nullptr) return nullptr;
    return module.getAddress(global);
}

// ---- numbers ----

bool ConstantFolder::foldNumber(ast::Expr *expr, FoldedNumber &out) {
    switch (expr->getKind()) {
    case ast::ASTKind::IntLiteral: {
        long long value = 0;
        if (!sema::intLiteralValue(static_cast<ast::IntLiteral *>(expr)->spelling, value)) return false;
        out = makeInt(value, sema::intType());
        return true;
    }
    case ast::ASTKind::CharLiteral:
        out = makeInt(sema::charLiteralValue(static_cast<ast::CharLiteral *>(expr)->spelling), sema::charType());
        return true;
    case ast::ASTKind::BoolLiteral:
        out = makeInt(static_cast<ast::BoolLiteral *>(expr)->value ? 1 : 0, sema::boolType());
        return true;
    case ast::ASTKind::FloatLiteral:
        out = makeFloat(std::strtod(static_cast<ast::FloatLiteral *>(expr)->spelling.c_str(), nullptr),
                        sema::doubleType());
        return true;
    case ast::ASTKind::ParenExpr:
        return foldNumber(static_cast<ast::ParenExpr *>(expr)->sub.get(), out);

    case ast::ASTKind::ImplicitCastExpr: {
        ast::ImplicitCastExpr *cast = static_cast<ast::ImplicitCastExpr *>(expr);
        if (!isArithmeticCast(cast->castKind)) return false;
        FoldedNumber inner;
        if (!foldNumber(cast->sub.get(), inner)) return false;
        out = convertNumber(inner, cast->type);
        return true;
    }

    case ast::ASTKind::UnaryExpr:
        return foldUnary(static_cast<ast::UnaryExpr *>(expr), out);
    case ast::ASTKind::BinaryExpr:
        return foldBinary(static_cast<ast::BinaryExpr *>(expr), out);

    case ast::ASTKind::ConditionalExpr: {
        ast::ConditionalExpr *cond = static_cast<ast::ConditionalExpr *>(expr);
        FoldedNumber test;
        if (!foldNumber(cond->cond.get(), test)) return false;
        bool chooseTrue = test.isFloat ? test.floatValue != 0.0 : test.intValue != 0;
        return foldNumber(chooseTrue ? cond->trueExpr.get() : cond->falseExpr.get(), out);
    }

    default:
        return false;
    }
}

bool ConstantFolder::foldUnary(ast::UnaryExpr *expr, FoldedNumber &out) {
    FoldedNumber operand;
    if (!foldNumber(expr->operand.get(), operand)) return false;

    switch (expr->op) {
    case ast::UnaryOp::Plus:
        out = operand;
        return true;
    case ast::UnaryOp::Minus:
        out = operand.isFloat ? makeFloat(-operand.floatValue, expr->type) : makeInt(-operand.intValue, expr->type);
        return true;
    case ast::UnaryOp::BitNot:
        out = makeInt(~operand.intValue, expr->type);
        return true;
    case ast::UnaryOp::LogicalNot:  //the operand was already converted to bool
        out = makeInt(operand.intValue == 0 ? 1 : 0, sema::boolType());
        return true;
    default:
        return false;
    }
}

//both operands were already converted to the same type by the type checker
bool ConstantFolder::foldBinary(ast::BinaryExpr *expr, FoldedNumber &out) {
    FoldedNumber a;
    FoldedNumber b;
    if (!foldNumber(expr->lhs.get(), a) || !foldNumber(expr->rhs.get(), b)) return false;
    const sema::Type *type = expr->type;

    if (a.isFloat || b.isFloat) {
        double x = a.isFloat ? a.floatValue : (double)a.intValue;
        double y = b.isFloat ? b.floatValue : (double)b.intValue;
        switch (expr->op) {
        case ast::BinaryOp::Add: out = makeFloat(x + y, type); return true;
        case ast::BinaryOp::Sub: out = makeFloat(x - y, type); return true;
        case ast::BinaryOp::Mul: out = makeFloat(x * y, type); return true;
        case ast::BinaryOp::Div: out = makeFloat(x / y, type); return true;
        case ast::BinaryOp::Eq:  out = makeInt(x == y, type); return true;
        case ast::BinaryOp::Ne:  out = makeInt(x != y, type); return true;
        case ast::BinaryOp::Lt:  out = makeInt(x < y, type); return true;
        case ast::BinaryOp::Gt:  out = makeInt(x > y, type); return true;
        case ast::BinaryOp::Le:  out = makeInt(x <= y, type); return true;
        case ast::BinaryOp::Ge:  out = makeInt(x >= y, type); return true;
        default:                 return false;
        }
    }

    long long x = a.intValue;
    long long y = b.intValue;
    switch (expr->op) {
    case ast::BinaryOp::Add: out = makeInt(x + y, type); return true;
    case ast::BinaryOp::Sub: out = makeInt(x - y, type); return true;
    case ast::BinaryOp::Mul: out = makeInt(x * y, type); return true;
    case ast::BinaryOp::Div:
    case ast::BinaryOp::Rem:
        //division by zero and INT_MIN / -1 are undefined: leave them to run
        if (y == 0 || (x == -2147483648LL && y == -1)) return false;
        out = makeInt(expr->op == ast::BinaryOp::Div ? x / y : x % y, type);
        return true;
    case ast::BinaryOp::Shl:
    case ast::BinaryOp::Shr:
        if (y < 0 || y >= 32) return false;
        out = makeInt(expr->op == ast::BinaryOp::Shl ? (long long)((unsigned long long)x << y) : x >> y, type);
        return true;
    case ast::BinaryOp::BitAnd:     out = makeInt(x & y, type); return true;
    case ast::BinaryOp::BitOr:      out = makeInt(x | y, type); return true;
    case ast::BinaryOp::BitXor:     out = makeInt(x ^ y, type); return true;
    case ast::BinaryOp::LogicalAnd: out = makeInt(x != 0 && y != 0, type); return true;
    case ast::BinaryOp::LogicalOr:  out = makeInt(x != 0 || y != 0, type); return true;
    case ast::BinaryOp::Eq:         out = makeInt(x == y, type); return true;
    case ast::BinaryOp::Ne:         out = makeInt(x != y, type); return true;
    case ast::BinaryOp::Lt:         out = makeInt(x < y, type); return true;
    case ast::BinaryOp::Gt:         out = makeInt(x > y, type); return true;
    case ast::BinaryOp::Le:         out = makeInt(x <= y, type); return true;
    case ast::BinaryOp::Ge:         out = makeInt(x >= y, type); return true;
    }
    return false;
}

//what an implicit conversion does to a value
FoldedNumber ConstantFolder::convertNumber(const FoldedNumber &number, const sema::Type *to) {
    if (to->is(sema::TypeKind::Bool)) {
        bool truth = number.isFloat ? number.floatValue != 0.0 : number.intValue != 0;
        return makeInt(truth ? 1 : 0, to);
    }
    if (sema::isFloating(to)) {
        return makeFloat(number.isFloat ? number.floatValue : (double)number.intValue, to);
    }
    if (number.isFloat) return makeInt((long long)number.floatValue, to);  //truncates toward zero
    return makeInt(number.intValue, to);
}

Constant *ConstantFolder::numberConstant(const FoldedNumber &number) {
    if (number.isFloat) return module.getFloat(number.type, number.floatValue);
    return module.getInt(number.type, number.intValue);
}

} // namespace ir
