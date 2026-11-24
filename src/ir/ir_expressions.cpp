//expressions lower via emitLValue (the address) or emitRValue (the value)
#include "ir/ir_visitor.h"

#include <cstdlib>

#include "semantic/constants.h"
#include "semantic/type_layout.h"

namespace ir {

namespace {

bool isPointerLike(const sema::Type *type) {
    return type->is(sema::TypeKind::Pointer) || type->is(sema::TypeKind::Null);
}

} // namespace

Value *IRVisitor::emitRValue(ast::Expr *expr) {
    //an object in memory: compute where it is, then read it
    if (expr->isLValue) {
        Value *address = emitLValue(expr);
        if (sema::asArray(expr->type) || sema::asFunction(expr->type)) return address;
        return builder.createLoad(expr->type, address);
    }

    switch (expr->getKind()) {
    case ast::ASTKind::IntLiteral: {
        long long value = 0;
        const sema::Type *type = nullptr;
        sema::intLiteral(static_cast<ast::IntLiteral *>(expr)->spelling, value, type);
        return module.getInt(expr->type, value);
    }
    case ast::ASTKind::FloatLiteral: {
        const sema::Type *type = nullptr;
        return module.getFloat(expr->type, sema::floatLiteral(static_cast<ast::FloatLiteral *>(expr)->spelling, type));
    }
    case ast::ASTKind::CharLiteral:
        return module.getInt(sema::charType(), sema::charLiteralValue(static_cast<ast::CharLiteral *>(expr)->spelling));
    case ast::ASTKind::BoolLiteral:
        return module.getBool(static_cast<ast::BoolLiteral *>(expr)->value);
    case ast::ASTKind::NullLiteral:
        return module.getNull(expr->type);
    case ast::ASTKind::ThisExpr:
        return ctx().thisPtr;
    case ast::ASTKind::ParenExpr:
        return emitRValue(static_cast<ast::ParenExpr *>(expr)->sub.get());
    case ast::ASTKind::UnaryExpr:
        return emitUnary(static_cast<ast::UnaryExpr *>(expr));
    case ast::ASTKind::BinaryExpr:
        return emitBinary(static_cast<ast::BinaryExpr *>(expr));
    case ast::ASTKind::ConditionalExpr:
        return emitConditional(static_cast<ast::ConditionalExpr *>(expr), false);
    case ast::ASTKind::CommaExpr: {
        ast::CommaExpr *comma = static_cast<ast::CommaExpr *>(expr);
        emitIgnored(comma->lhs.get());
        return emitRValue(comma->rhs.get());
    }
    case ast::ASTKind::CallExpr:
        return emitCall(static_cast<ast::CallExpr *>(expr));
    case ast::ASTKind::NewExpr:
        return emitNew(static_cast<ast::NewExpr *>(expr));
    case ast::ASTKind::DeleteExpr:
        return emitDelete(static_cast<ast::DeleteExpr *>(expr));
    case ast::ASTKind::ImplicitCastExpr:
        return emitCast(static_cast<ast::ImplicitCastExpr *>(expr));
    case ast::ASTKind::CastExpr: {
        //the conversion is in the implicit casts under it; a cast to void only discards
        ast::CastExpr *cast = static_cast<ast::CastExpr *>(expr);
        if (cast->type->is(sema::TypeKind::Void)) {
            emitIgnored(cast->sub.get());
            return nullptr;
        }
        return emitRValue(cast->sub.get());
    }
    case ast::ASTKind::SizeofExpr:
        return module.getInt(expr->type, (long long)static_cast<ast::SizeofExpr *>(expr)->size);
    case ast::ASTKind::MemberExpr: {
        Value *address = emitMemberAddress(static_cast<ast::MemberExpr *>(expr));
        if (sema::asArray(expr->type)) return address;
        return builder.createLoad(expr->type, address);
    }
    case ast::ASTKind::LambdaExpr: {
        //a closure object made in a temporary slot and read as a whole
        AllocaInst *temp = builder.createEntryAlloca(expr->type, "closure.tmp");
        emitLambdaInto(temp, static_cast<ast::LambdaExpr *>(expr));
        return builder.createLoad(expr->type, temp);
    }
    default:
        break;
    }
    error(expr->line, expr->col, "this expression has no value here");
    return module.getZero(sema::intType());
}

Value *IRVisitor::emitLValue(ast::Expr *expr) {
    switch (expr->getKind()) {
    case ast::ASTKind::DeclRefExpr:
        return emitDeclRefAddress(static_cast<ast::DeclRefExpr *>(expr));
    case ast::ASTKind::ParenExpr:
        return emitLValue(static_cast<ast::ParenExpr *>(expr)->sub.get());
    case ast::ASTKind::StringLiteral: {
        std::string bytes = sema::decodeStringLiteral(static_cast<ast::StringLiteral *>(expr)->spelling);
        bytes.push_back('\0');
        return module.getStringLiteral(bytes);
    }
    case ast::ASTKind::UnaryExpr: {
        ast::UnaryExpr *unary = static_cast<ast::UnaryExpr *>(expr);
        //*p names the object p points at, so its address is the value of p
        if (unary->op == ast::UnaryOp::Deref) return emitRValue(unary->operand.get());
        if (unary->op == ast::UnaryOp::PreInc || unary->op == ast::UnaryOp::PreDec) {
            return emitIncDec(unary).address;  //++x names x itself
        }
        break;
    }
    case ast::ASTKind::ArraySubscriptExpr:
        return emitSubscriptAddress(static_cast<ast::ArraySubscriptExpr *>(expr));
    case ast::ASTKind::MemberExpr:
        return emitMemberAddress(static_cast<ast::MemberExpr *>(expr));
    case ast::ASTKind::AssignExpr:
        return emitAssign(static_cast<ast::AssignExpr *>(expr));
    case ast::ASTKind::ConditionalExpr:
        if (expr->isLValue) return emitConditional(static_cast<ast::ConditionalExpr *>(expr), true);
        break;
    case ast::ASTKind::CommaExpr:
        if (expr->isLValue) {
            ast::CommaExpr *comma = static_cast<ast::CommaExpr *>(expr);
            emitIgnored(comma->lhs.get());
            return emitAddressOf(comma->rhs.get());
        }
        break;
    case ast::ASTKind::ImplicitCastExpr: {
        //binding a Base& to a Derived object: the base part of the same object
        ast::ImplicitCastExpr *cast = static_cast<ast::ImplicitCastExpr *>(expr);
        if (cast->castKind == ast::CastKind::DerivedToBase && !sema::asPointer(cast->type)) {
            Value *object = emitAddressOf(cast->sub.get());
            return adjustToBase(object, sema::asRecord(cast->sub->type), sema::asRecord(cast->type), false);
        }
        break;
    }
    default:
        break;
    }
    //a value that is not in memory yet, but whose address is needed
    return materialize(emitRValue(expr), expr->type);
}

//&& and || in a condition branch straight to the targets without making a bool
void IRVisitor::emitBranch(ast::Expr *expr, BasicBlock *ifTrue, BasicBlock *ifFalse) {
    switch (expr->getKind()) {
    case ast::ASTKind::ParenExpr:
        return emitBranch(static_cast<ast::ParenExpr *>(expr)->sub.get(), ifTrue, ifFalse);
    case ast::ASTKind::UnaryExpr: {
        ast::UnaryExpr *unary = static_cast<ast::UnaryExpr *>(expr);
        if (unary->op == ast::UnaryOp::LogicalNot) return emitBranch(unary->operand.get(), ifFalse, ifTrue);
        break;
    }
    case ast::ASTKind::BinaryExpr: {
        ast::BinaryExpr *binary = static_cast<ast::BinaryExpr *>(expr);
        bool isAnd = binary->op == ast::BinaryOp::LogicalAnd;
        if (!isAnd && binary->op != ast::BinaryOp::LogicalOr) break;
        BasicBlock *rhsBlock = builder.createBlock(isAnd ? "land.rhs" : "lor.rhs");
        if (isAnd) {
            emitBranch(binary->lhs.get(), rhsBlock, ifFalse);
        } else {
            emitBranch(binary->lhs.get(), ifTrue, rhsBlock);
        }
        builder.setInsertBlock(rhsBlock);
        return emitBranch(binary->rhs.get(), ifTrue, ifFalse);
    }
    default:
        break;
    }
    builder.createCondBr(emitRValue(expr), ifTrue, ifFalse);
}

//an expression statement: computed for its side effects. an lvalue is not read
void IRVisitor::emitIgnored(ast::Expr *expr) {
    if (expr->isLValue) {
        emitLValue(expr);
    } else {
        emitRValue(expr);
    }
}

Value *IRVisitor::emitAddressOf(ast::Expr *expr) {
    if (expr->isLValue) return emitLValue(expr);
    return materialize(emitRValue(expr), expr->type);
}

//spills a value to a new stack slot and returns its address
Value *IRVisitor::materialize(Value *value, const sema::Type *type) {
    AllocaInst *temp = builder.createEntryAlloca(type, "tmp");
    builder.createStore(type, value, temp);
    return temp;
}

Value *IRVisitor::emitDeclRefAddress(ast::DeclRefExpr *node) {
    sema::Symbol *symbol = node->symbol;
    switch (symbol->kind) {
    case sema::SymbolKind::Function: {
        Function *fn = functionFor(static_cast<sema::FunctionSymbol *>(symbol));
        if (builder.isReachable()) fn->isUsed = true;
        return fn;
    }
    case sema::SymbolKind::Field:
        //a member named without this->, inside a method
        return fieldAddress(ctx().thisPtr, ctx().thisRecord, symbol);
    default:
        return variableAddress(symbol);
    }
}

//the address a variable names, loaded through the slot for references
Value *IRVisitor::variableAddress(const sema::Symbol *symbol) {
    VariableBinding binding;
    std::map<const sema::Symbol *, VariableBinding>::iterator local = ctx().locals.find(symbol);
    if (local != ctx().locals.end()) {
        binding = local->second;
    } else {
        std::map<const sema::Symbol *, VariableBinding>::iterator global = globals.find(symbol);
        if (global == globals.end()) {
            error(symbol->line, symbol->col, "'" + symbol->name + "' has no storage here");
            return module.getNull(module.pointerType());
        }
        binding = global->second;
    }
    if (binding.slotHoldsAddress) return builder.createLoad(module.pointerType(), binding.slot);
    return binding.slot;
}

//s.f or p->f: the object's address, moved to the declaring base, then the field
Value *IRVisitor::emitMemberAddress(ast::MemberExpr *node) {
    sema::Symbol *member = node->symbol;
    if (member->kind == sema::SymbolKind::Variable) {
        emitIgnored(node->base.get());  //obj.staticMember still evaluates obj
        return variableAddress(member);
    }

    Value *object = nullptr;
    const sema::RecordSymbol *record = nullptr;
    if (node->isArrow) {
        object = emitRValue(node->base.get());
        record = sema::asRecord(sema::asPointer(node->base->type)->pointee);
    } else {
        object = emitAddressOf(node->base.get());
        record = sema::asRecord(node->base->type);
    }
    return fieldAddress(object, record, member);
}

//a[i]: getelementptr on the decayed base pointer
Value *IRVisitor::emitSubscriptAddress(ast::ArraySubscriptExpr *node) {
    //the left operand is evaluated first, as written in the source
    Value *base = nullptr;
    Value *index = nullptr;
    if (node->reversed) {
        index = emitRValue(node->index.get());
        base = emitRValue(node->base.get());
    } else {
        base = emitRValue(node->base.get());
        index = emitRValue(node->index.get());
    }
    return builder.createElementGEP(node->type, base, index);
}

Value *IRVisitor::emitUnary(ast::UnaryExpr *node) {
    switch (node->op) {
    case ast::UnaryOp::Plus:
        return emitRValue(node->operand.get());
    case ast::UnaryOp::Minus: {
        Value *value = emitRValue(node->operand.get());
        if (sema::isFloating(node->type)) return builder.createFNeg(value);
        return builder.createBinary(BinaryOpKind::Sub, node->type, module.getInt(node->type, 0), value);
    }
    case ast::UnaryOp::BitNot:
        return builder.createBinary(BinaryOpKind::Xor, node->type, emitRValue(node->operand.get()),
                                    module.getInt(node->type, -1));
    case ast::UnaryOp::LogicalNot:
        return builder.createBinary(BinaryOpKind::Xor, sema::boolType(), emitRValue(node->operand.get()),
                                    module.getBool(true));
    case ast::UnaryOp::AddrOf:
        return emitLValue(node->operand.get());
    case ast::UnaryOp::PostInc:
    case ast::UnaryOp::PostDec:
        return emitIncDec(node).oldValue;
    case ast::UnaryOp::PreInc:
    case ast::UnaryOp::PreDec:
    case ast::UnaryOp::Deref:
        break;  //lvalues, handled by emitRValue through emitLValue
    }
    return emitLValue(node);
}

//++x, x++, --x, x--, the operand is evaluated once
IncDecResult IRVisitor::emitIncDec(ast::UnaryExpr *node) {
    ast::Expr *operand = node->operand.get();
    const sema::Type *type = operand->type;
    bool increment = node->op == ast::UnaryOp::PreInc || node->op == ast::UnaryOp::PostInc;

    IncDecResult result;
    result.address = emitLValue(operand);
    result.oldValue = builder.createLoad(type, result.address);
    if (const sema::PointerType *pointer = sema::asPointer(type)) {
        result.newValue = builder.createElementGEP(pointer->pointee, result.oldValue,
                                                   module.getInt32(increment ? 1 : -1));
    } else if (sema::isFloating(type)) {
        result.newValue = builder.createBinary(increment ? BinaryOpKind::FAdd : BinaryOpKind::FSub, type,
                                               result.oldValue, module.getFloat(type, 1.0));
    } else {
        result.newValue = builder.createBinary(increment ? BinaryOpKind::Add : BinaryOpKind::Sub, type,
                                               result.oldValue, module.getInt(type, 1));
    }
    builder.createStore(type, result.newValue, result.address);
    return result;
}

Value *IRVisitor::emitBinary(ast::BinaryExpr *node) {
    if (node->op == ast::BinaryOp::LogicalAnd || node->op == ast::BinaryOp::LogicalOr) {
        return emitShortCircuit(node);
    }

    Value *lhs = emitRValue(node->lhs.get());
    Value *rhs = emitRValue(node->rhs.get());
    const sema::Type *lhsType = node->lhs->type;
    const sema::Type *rhsType = node->rhs->type;

    switch (node->op) {
    case ast::BinaryOp::Eq:
    case ast::BinaryOp::Ne:
    case ast::BinaryOp::Lt:
    case ast::BinaryOp::Gt:
    case ast::BinaryOp::Le:
    case ast::BinaryOp::Ge:
        return emitCompare(node->op, lhsType, lhs, rhs);
    case ast::BinaryOp::Add:
        if (sema::asPointer(node->type)) return emitPointerArithmetic(node, lhs, rhs);
        break;
    case ast::BinaryOp::Sub:
        if (sema::asPointer(node->type)) return emitPointerArithmetic(node, lhs, rhs);
        if (sema::asPointer(lhsType) && sema::asPointer(rhsType)) return emitPointerDifference(node, lhs, rhs);
        break;
    default:
        break;
    }
    //both operands were converted to the type of the result
    return emitArithmetic(node->op, node->type, lhs, rhs);
}

Value *IRVisitor::emitArithmetic(ast::BinaryOp op, const sema::Type *type, Value *lhs, Value *rhs) {
    bool floating = sema::isFloating(type);
    BinaryOpKind kind = BinaryOpKind::Add;
    switch (op) {
    case ast::BinaryOp::Add:    kind = floating ? BinaryOpKind::FAdd : BinaryOpKind::Add; break;
    case ast::BinaryOp::Sub:    kind = floating ? BinaryOpKind::FSub : BinaryOpKind::Sub; break;
    case ast::BinaryOp::Mul:    kind = floating ? BinaryOpKind::FMul : BinaryOpKind::Mul; break;
    case ast::BinaryOp::Div:
        kind = floating ? BinaryOpKind::FDiv : sema::isUnsigned(type) ? BinaryOpKind::UDiv : BinaryOpKind::SDiv;
        break;
    case ast::BinaryOp::Rem:    kind = sema::isUnsigned(type) ? BinaryOpKind::URem : BinaryOpKind::SRem; break;
    case ast::BinaryOp::Shl:    kind = BinaryOpKind::Shl; break;
    case ast::BinaryOp::Shr:    kind = sema::isUnsigned(type) ? BinaryOpKind::LShr : BinaryOpKind::AShr; break;
    case ast::BinaryOp::BitAnd: kind = BinaryOpKind::And; break;
    case ast::BinaryOp::BitOr:  kind = BinaryOpKind::Or; break;
    case ast::BinaryOp::BitXor: kind = BinaryOpKind::Xor; break;
    default:                    break;
    }
    return builder.createBinary(kind, type, lhs, rhs);
}

//signed integer, ordered float (except !=) and unsigned pointer compares
Value *IRVisitor::emitCompare(ast::BinaryOp op, const sema::Type *operandType, Value *lhs, Value *rhs) {
    BinaryOpKind kind = BinaryOpKind::ICmpEQ;
    if (isPointerLike(operandType)) {
        switch (op) {
        case ast::BinaryOp::Eq: kind = BinaryOpKind::ICmpEQ; break;
        case ast::BinaryOp::Ne: kind = BinaryOpKind::ICmpNE; break;
        case ast::BinaryOp::Lt: kind = BinaryOpKind::ICmpULT; break;
        case ast::BinaryOp::Gt: kind = BinaryOpKind::ICmpUGT; break;
        case ast::BinaryOp::Le: kind = BinaryOpKind::ICmpULE; break;
        default:                kind = BinaryOpKind::ICmpUGE; break;
        }
        return builder.createBinary(kind, module.pointerType(), lhs, rhs);
    }
    if (sema::isFloating(operandType)) {
        switch (op) {
        case ast::BinaryOp::Eq: kind = BinaryOpKind::FCmpOEQ; break;
        case ast::BinaryOp::Ne: kind = BinaryOpKind::FCmpUNE; break;
        case ast::BinaryOp::Lt: kind = BinaryOpKind::FCmpOLT; break;
        case ast::BinaryOp::Gt: kind = BinaryOpKind::FCmpOGT; break;
        case ast::BinaryOp::Le: kind = BinaryOpKind::FCmpOLE; break;
        default:                kind = BinaryOpKind::FCmpOGE; break;
        }
        return builder.createBinary(kind, operandType, lhs, rhs);
    }
    bool isUnsignedOp = sema::isUnsigned(operandType);
    switch (op) {
    case ast::BinaryOp::Eq: kind = BinaryOpKind::ICmpEQ; break;
    case ast::BinaryOp::Ne: kind = BinaryOpKind::ICmpNE; break;
    case ast::BinaryOp::Lt: kind = isUnsignedOp ? BinaryOpKind::ICmpULT : BinaryOpKind::ICmpSLT; break;
    case ast::BinaryOp::Gt: kind = isUnsignedOp ? BinaryOpKind::ICmpUGT : BinaryOpKind::ICmpSGT; break;
    case ast::BinaryOp::Le: kind = isUnsignedOp ? BinaryOpKind::ICmpULE : BinaryOpKind::ICmpSLE; break;
    default:                kind = isUnsignedOp ? BinaryOpKind::ICmpUGE : BinaryOpKind::ICmpSGE; break;
    }
    return builder.createBinary(kind, operandType, lhs, rhs);
}

//p + n, n + p and p - n move p by n elements
Value *IRVisitor::emitPointerArithmetic(ast::BinaryExpr *node, Value *lhs, Value *rhs) {
    bool pointerOnLeft = sema::asPointer(node->lhs->type) != nullptr;
    Value *pointer = pointerOnLeft ? lhs : rhs;
    Value *count = pointerOnLeft ? rhs : lhs;
    if (node->op == ast::BinaryOp::Sub) {
        const sema::Type *countType = (pointerOnLeft ? node->rhs : node->lhs)->type;
        count = builder.createBinary(BinaryOpKind::Sub, countType, module.getInt(countType, 0), count);
    }
    return builder.createElementGEP(sema::asPointer(node->type)->pointee, pointer, count);
}

//q - p: byte distance divided exactly by the element size
Value *IRVisitor::emitPointerDifference(ast::BinaryExpr *node, Value *lhs, Value *rhs) {
    const sema::Type *element = sema::asPointer(node->lhs->type)->pointee;
    Value *left = builder.createCast(CastOpKind::PtrToInt, lhs, module.pointerType(), sema::longType());
    Value *right = builder.createCast(CastOpKind::PtrToInt, rhs, module.pointerType(), sema::longType());
    Value *bytes = builder.createBinary(BinaryOpKind::Sub, sema::longType(), left, right);
    Value *count = builder.createExactSDiv(sema::longType(), bytes, sizeOf(element, module.getInt64(1)));
    return builder.createCast(CastOpKind::Trunc, count, sema::longType(), sema::intType());
}

//short-circuit logic through a stack slot instead of a phi
Value *IRVisitor::emitShortCircuit(ast::BinaryExpr *node) {
    bool isAnd = node->op == ast::BinaryOp::LogicalAnd;
    AllocaInst *slot = builder.createEntryAlloca(sema::boolType(), isAnd ? "and.result" : "or.result");
    BasicBlock *rhsBlock = builder.createBlock(isAnd ? "land.rhs" : "lor.rhs");
    BasicBlock *shortBlock = builder.createBlock(isAnd ? "land.short" : "lor.short");
    BasicBlock *endBlock = builder.createBlock(isAnd ? "land.end" : "lor.end");

    Value *lhs = emitRValue(node->lhs.get());
    if (isAnd) {
        builder.createCondBr(lhs, rhsBlock, shortBlock);
    } else {
        builder.createCondBr(lhs, shortBlock, rhsBlock);
    }

    builder.setInsertBlock(shortBlock);
    builder.createStore(sema::boolType(), module.getBool(!isAnd), slot);
    builder.createBr(endBlock);

    builder.setInsertBlock(rhsBlock);
    builder.createStore(sema::boolType(), emitRValue(node->rhs.get()), slot);
    builder.createBr(endBlock);

    builder.setInsertBlock(endBlock);
    return builder.createLoad(sema::boolType(), slot);
}

//x = v and x op= v, the value is computed before the address
Value *IRVisitor::emitAssign(ast::AssignExpr *node) {
    ast::Expr *target = node->target.get();
    const sema::Type *targetType = target->type;

    if (node->op == ast::AssignOp::Assign) {
        Value *value = emitRValue(node->value.get());
        Value *address = emitLValue(target);
        builder.createStore(targetType, value, address);
        return address;
    }

    Value *value = emitRValue(node->value.get());
    Value *address = emitLValue(target);
    Value *oldValue = builder.createLoad(targetType, address);
    Value *result = nullptr;

    if (const sema::PointerType *pointer = sema::asPointer(targetType)) {
        Value *count = value;
        if (node->op == ast::AssignOp::SubAssign) {
            count = builder.createBinary(BinaryOpKind::Sub, node->value->type, module.getInt(node->value->type, 0), value);
        }
        result = builder.createElementGEP(pointer->pointee, oldValue, count);
    } else {
        const sema::Type *common = node->value->type;
        ast::BinaryOp op = ast::BinaryOp::Add;
        switch (node->op) {
        case ast::AssignOp::AddAssign: op = ast::BinaryOp::Add; break;
        case ast::AssignOp::SubAssign: op = ast::BinaryOp::Sub; break;
        case ast::AssignOp::MulAssign: op = ast::BinaryOp::Mul; break;
        case ast::AssignOp::DivAssign: op = ast::BinaryOp::Div; break;
        case ast::AssignOp::RemAssign: op = ast::BinaryOp::Rem; break;
        case ast::AssignOp::ShlAssign: op = ast::BinaryOp::Shl; break;
        case ast::AssignOp::ShrAssign: op = ast::BinaryOp::Shr; break;
        case ast::AssignOp::AndAssign: op = ast::BinaryOp::BitAnd; break;
        case ast::AssignOp::OrAssign:  op = ast::BinaryOp::BitOr; break;
        case ast::AssignOp::XorAssign: op = ast::BinaryOp::BitXor; break;
        case ast::AssignOp::Assign:    break;
        }
        Value *widened = emitArithmeticConversion(oldValue, targetType, common);
        Value *combined = emitArithmetic(op, common, widened, value);
        result = emitArithmeticConversion(combined, common, targetType);
    }
    builder.createStore(targetType, result, address);
    return address;
}

//c ? a : b with branches, an lvalue when both arms are lvalues of one type
Value *IRVisitor::emitConditional(ast::ConditionalExpr *node, bool wantAddress) {
    const sema::Type *slotType = wantAddress ? module.pointerType() : node->type;
    bool hasValue = wantAddress || !node->type->is(sema::TypeKind::Void);
    AllocaInst *slot = hasValue ? builder.createEntryAlloca(slotType, "cond.result") : nullptr;

    BasicBlock *trueBlock = builder.createBlock("cond.true");
    BasicBlock *falseBlock = builder.createBlock("cond.false");
    BasicBlock *endBlock = builder.createBlock("cond.end");
    emitBranch(node->cond.get(), trueBlock, falseBlock);

    ast::Expr *arms[2] = { node->trueExpr.get(), node->falseExpr.get() };
    BasicBlock *blocks[2] = { trueBlock, falseBlock };
    for (int i = 0; i < 2; i++) {
        builder.setInsertBlock(blocks[i]);
        if (wantAddress) {
            builder.createStore(slotType, emitLValue(arms[i]), slot);
        } else if (hasValue) {
            builder.createStore(slotType, emitRValue(arms[i]), slot);
        } else {
            emitIgnored(arms[i]);
        }
        builder.createBr(endBlock);
    }

    builder.setInsertBlock(endBlock);
    if (!hasValue) return nullptr;
    return builder.createLoad(slotType, slot);
}

Value *IRVisitor::emitCast(ast::ImplicitCastExpr *node) {
    ast::Expr *sub = node->sub.get();
    switch (node->castKind) {
    case ast::CastKind::IntegralCast:
    case ast::CastKind::IntegralToFloating:
    case ast::CastKind::FloatingToIntegral:
    case ast::CastKind::FloatingCast:
        return emitArithmeticConversion(emitRValue(sub), sub->type, node->type);

    case ast::CastKind::ToBool:
        if (isPointerLike(sub->type)) {
            return builder.createBinary(BinaryOpKind::ICmpNE, module.pointerType(), emitRValue(sub),
                                        module.getNull(module.pointerType()));
        }
        return emitArithmeticConversion(emitRValue(sub), sub->type, node->type);

    //an array decays to its first element: gep with two zero indices
    case ast::CastKind::ArrayToPointer: {
        Value *array = emitAddressOf(sub);
        std::vector<Value *> indices;
        indices.push_back(module.getInt64(0));
        indices.push_back(module.getInt64(0));
        return builder.createGEP(sub->type, array, indices);
    }

    //a function's address is the function itself, @name
    case ast::CastKind::FunctionToPointer:
        return emitLValue(sub);

    case ast::CastKind::NullToPointer:
        return module.getNull(node->type);

    //every pointer is ptr in llvm, so only the sema type changes
    case ast::CastKind::PointerToVoid:
    case ast::CastKind::PointerCast:
        return emitRValue(sub);
    case ast::CastKind::PointerToIntegral:
        return builder.createCast(CastOpKind::PtrToInt, emitRValue(sub), module.pointerType(), node->type);
    case ast::CastKind::IntegralToPointer:
        return builder.createCast(CastOpKind::IntToPtr, emitRValue(sub), sub->type, module.pointerType());

    case ast::CastKind::BaseToDerived: {
        Value *address = emitRValue(sub);
        const sema::RecordSymbol *base = sema::asRecord(sema::unqualified(sema::asPointer(sub->type)->pointee));
        const sema::RecordSymbol *derived = sema::asRecord(sema::unqualified(sema::asPointer(node->type)->pointee));
        std::vector<unsigned> path;
        if (!RecordLayout::findBasePath(derived, base, path)) return address;
        sema::TypeLayout layout(symTab);
        uint64_t offset = 0;
        const sema::RecordSymbol *current = derived;
        for (unsigned index : path) {
            offset += layout.elementOffset(current->type, index);
            current = current->bases[index].record;
        }
        if (offset == 0) return address;
        Value *moved = builder.createElementGEP(sema::charType(), address,
                                                module.getInt(sema::longType(), -(long long)offset));
        Value *isNull = builder.createBinary(BinaryOpKind::ICmpEQ, module.pointerType(), address,
                                             module.getNull(module.pointerType()));
        return builder.createSelect(isNull, module.getNull(module.pointerType()), moved, module.pointerType());
    }
    case ast::CastKind::DerivedToBase: {
        if (const sema::PointerType *target = sema::asPointer(node->type)) {
            const sema::RecordSymbol *from = sema::asRecord(sema::asPointer(sub->type)->pointee);
            return adjustToBase(emitRValue(sub), from, sema::asRecord(target->pointee), true);
        }
        //slicing: a Derived value copied into a Base keeps only its base part
        Value *object = emitAddressOf(sub);
        Value *base = adjustToBase(object, sema::asRecord(sub->type), sema::asRecord(node->type), false);
        return builder.createLoad(node->type, base);
    }
    }
    return emitRValue(sub);
}

//conversions between bool (i1), char (i8), int (i32), float and double
Value *IRVisitor::emitArithmeticConversion(Value *value, const sema::Type *from, const sema::Type *to) {
    if (from->getKind() == to->getKind()) return value;

    if (to->is(sema::TypeKind::Bool)) {
        if (sema::isFloating(from)) {
            return builder.createBinary(BinaryOpKind::FCmpUNE, from, value, module.getFloat(from, 0.0));
        }
        return builder.createBinary(BinaryOpKind::ICmpNE, from, value, module.getInt(from, 0));
    }

    if (sema::isFloating(to)) {
        if (sema::isFloating(from)) {
            CastOpKind op = to->is(sema::TypeKind::Double) ? CastOpKind::FPExt : CastOpKind::FPTrunc;
            return builder.createCast(op, value, from, to);
        }
        bool fromUnsigned = from->is(sema::TypeKind::Bool) || sema::isUnsigned(from);
        return builder.createCast(fromUnsigned ? CastOpKind::UIToFP : CastOpKind::SIToFP, value, from, to);
    }

    if (sema::isFloating(from)) {
        return builder.createCast(sema::isUnsigned(to) ? CastOpKind::FPToUI : CastOpKind::FPToSI, value, from, to);
    }

    unsigned fromBits = integerBits(from);
    unsigned toBits = integerBits(to);
    if (toBits == fromBits) return value;  //int and unsigned int: the same bits, read differently
    if (toBits < fromBits) return builder.createCast(CastOpKind::Trunc, value, from, to);
    bool fromUnsigned = from->is(sema::TypeKind::Bool) || sema::isUnsigned(from);
    return builder.createCast(fromUnsigned ? CastOpKind::ZExt : CastOpKind::SExt, value, from, to);
}

//count * sizeof(type) in bytes, via getelementptr from null and ptrtoint
Value *IRVisitor::sizeOf(const sema::Type *type, Value *count) {
    std::vector<Value *> indices;
    indices.push_back(count);
    Value *end = builder.createGEP(type, module.getNull(module.pointerType()), indices, false);
    return builder.createCast(CastOpKind::PtrToInt, end, module.pointerType(), sema::longType());
}

} // namespace ir
