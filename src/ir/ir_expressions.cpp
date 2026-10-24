#include "ir/ir_visitor.h"

#include <cstdlib>

#include "semantic/constants.h"

//expressions. every expression is lowered in one of two ways:
//  emitLValue: the address of the object it names (a ptr), for x in x = 1
//  emitRValue: its value, for x in y = x. for an lvalue that is a load from
//              the address emitLValue gives
//the type checker marks each expression with isLValue and its type, and has
//already wrapped every implicit conversion in an ImplicitCastExpr

namespace ir {

namespace {

bool isPointerLike(const sema::Type *type) {
    return type->is(sema::TypeKind::Pointer) || type->is(sema::TypeKind::Null);
}

} // namespace

// ---------------------------------------------------------------------------
// the two ways to lower an expression

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
        sema::intLiteralValue(static_cast<ast::IntLiteral *>(expr)->spelling, value);
        return module.getInt32(value);
    }
    case ast::ASTKind::FloatLiteral:
        return module.getFloat(sema::doubleType(),
                               std::strtod(static_cast<ast::FloatLiteral *>(expr)->spelling.c_str(), nullptr));
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
    case ast::ASTKind::CallExpr:
        return emitCall(static_cast<ast::CallExpr *>(expr));
    case ast::ASTKind::NewExpr:
        return emitNew(static_cast<ast::NewExpr *>(expr));
    case ast::ASTKind::DeleteExpr:
        return emitDelete(static_cast<ast::DeleteExpr *>(expr));
    case ast::ASTKind::ImplicitCastExpr:
        return emitCast(static_cast<ast::ImplicitCastExpr *>(expr));
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

//a bool for a branch. the type checker already turned every condition into a
//bool with a ToBool cast, so this is the value itself
Value *IRVisitor::emitCondition(ast::Expr *expr) { return emitRValue(expr); }

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

//stores a value into a new stack slot and gives the slot's address. used for
//an object that only exists as a value, such as a record returned by a call,
//when a member of it or a method on it needs its address
Value *IRVisitor::materialize(Value *value, const sema::Type *type) {
    AllocaInst *temp = builder.createEntryAlloca(type, "tmp");
    builder.createStore(type, value, temp);
    return temp;
}

// ---------------------------------------------------------------------------
// names, members and subscripts

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

//the address of the object a variable names. for a reference the slot holds
//that address, so it is loaded: int &r = x; r = 5 stores through load(%r)
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

//s.f or p->f: the address of the object, moved to the base class that declares
//the field when it is inherited, then the field inside it
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

//a[i]: the base was already turned into a pointer to the first element, so this
//is getelementptr <element>, ptr %a, i32 %i. a two dimensional grid[r][c] is two
//of these, with the inner row turned into a pointer by an ArrayToPointer cast
Value *IRVisitor::emitSubscriptAddress(ast::ArraySubscriptExpr *node) {
    Value *base = emitRValue(node->base.get());
    Value *index = emitRValue(node->index.get());
    return builder.createElementGEP(node->type, base, index);
}

// ---------------------------------------------------------------------------
// operators

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
        //the operand is already a bool: flip it
        return builder.createBinary(BinaryOpKind::Xor, sema::boolType(), emitRValue(node->operand.get()),
                                    module.getBool(true));
    case ast::UnaryOp::AddrOf:
        //the address of x is the value of &x
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

//++x, x++, --x, x--: read, add or subtract one, write back. a pointer moves by
//one element. x++ gives the old value, ++x names x itself. the operand is
//evaluated once, so ++a[i++] moves i only once
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
    case ast::BinaryOp::Div:    kind = floating ? BinaryOpKind::FDiv : BinaryOpKind::SDiv; break;
    case ast::BinaryOp::Rem:    kind = BinaryOpKind::SRem; break;
    case ast::BinaryOp::Shl:    kind = BinaryOpKind::Shl; break;
    case ast::BinaryOp::Shr:    kind = BinaryOpKind::AShr; break;   //int is signed: arithmetic shift
    case ast::BinaryOp::BitAnd: kind = BinaryOpKind::And; break;
    case ast::BinaryOp::BitOr:  kind = BinaryOpKind::Or; break;
    case ast::BinaryOp::BitXor: kind = BinaryOpKind::Xor; break;
    default:                    break;
    }
    return builder.createBinary(kind, type, lhs, rhs);
}

//integers compare signed, floats with the ordered predicates (false when a side
//is NaN) except != which is true for NaN, pointers as unsigned addresses
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
    switch (op) {
    case ast::BinaryOp::Eq: kind = BinaryOpKind::ICmpEQ; break;
    case ast::BinaryOp::Ne: kind = BinaryOpKind::ICmpNE; break;
    case ast::BinaryOp::Lt: kind = BinaryOpKind::ICmpSLT; break;
    case ast::BinaryOp::Gt: kind = BinaryOpKind::ICmpSGT; break;
    case ast::BinaryOp::Le: kind = BinaryOpKind::ICmpSLE; break;
    default:                kind = BinaryOpKind::ICmpSGE; break;
    }
    return builder.createBinary(kind, operandType, lhs, rhs);
}

//p + n and n + p move p by n elements: getelementptr inbounds <pointee>, ptr %p,
//i32 %n. p - n moves by -n
Value *IRVisitor::emitPointerArithmetic(ast::BinaryExpr *node, Value *lhs, Value *rhs) {
    bool pointerOnLeft = sema::asPointer(node->lhs->type) != nullptr;
    Value *pointer = pointerOnLeft ? lhs : rhs;
    Value *count = pointerOnLeft ? rhs : lhs;
    if (node->op == ast::BinaryOp::Sub) {
        count = builder.createBinary(BinaryOpKind::Sub, sema::intType(), module.getInt32(0), count);
    }
    return builder.createElementGEP(sema::asPointer(node->type)->pointee, pointer, count);
}

//q - p counts the elements between two pointers: the byte distance divided by
//the element size. the division is exact because both point into one array
Value *IRVisitor::emitPointerDifference(ast::BinaryExpr *node, Value *lhs, Value *rhs) {
    const sema::Type *element = sema::asPointer(node->lhs->type)->pointee;
    Value *left = builder.createCast(CastOpKind::PtrToInt, lhs, module.pointerType(), int64Type());
    Value *right = builder.createCast(CastOpKind::PtrToInt, rhs, module.pointerType(), int64Type());
    Value *bytes = builder.createBinary(BinaryOpKind::Sub, int64Type(), left, right);
    Value *count = builder.createExactSDiv(int64Type(), bytes, sizeOf(element, module.getInt64(1)));
    return builder.createCast(CastOpKind::Trunc, count, int64Type(), sema::intType());
}

//a && b must not evaluate b when a is false (p != null && p->x > 0). the
//result goes through a stack slot instead of a phi node:
//           br i1 %a, label %land.rhs, label %land.short
//  land.short: store i1 false, ptr %slot     land.rhs: store i1 %b, ptr %slot
//  land.end:   %r = load i1, ptr %slot
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

//x = v stores v and gives x. x op= v reads x once, combines it with v in their
//common type (the type checker converted v to it) and converts the result back
//to the type of x before the store. the value is computed before the address,
//as c++17 orders assignments
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
        //p += n, p -= n
        Value *count = value;
        if (node->op == ast::AssignOp::SubAssign) {
            count = builder.createBinary(BinaryOpKind::Sub, sema::intType(), module.getInt32(0), value);
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
        case ast::AssignOp::Assign:    break;
        }
        Value *widened = emitArithmeticConversion(oldValue, targetType, common);
        Value *combined = emitArithmetic(op, common, widened, value);
        result = emitArithmeticConversion(combined, common, targetType);
    }
    builder.createStore(targetType, result, address);
    return address;
}

//c ? a : b with branches. as a value it goes through a stack slot like &&.
//when both arms are lvalues of one type the whole expression is an lvalue
//((a > b) ? a : b) = 0 and the slot holds the address of the chosen arm.
//select is never used: it would evaluate both arms
Value *IRVisitor::emitConditional(ast::ConditionalExpr *node, bool wantAddress) {
    const sema::Type *slotType = wantAddress ? module.pointerType() : node->type;
    bool hasValue = wantAddress || !node->type->is(sema::TypeKind::Void);
    AllocaInst *slot = hasValue ? builder.createEntryAlloca(slotType, "cond.result") : nullptr;

    BasicBlock *trueBlock = builder.createBlock("cond.true");
    BasicBlock *falseBlock = builder.createBlock("cond.false");
    BasicBlock *endBlock = builder.createBlock("cond.end");
    builder.createCondBr(emitCondition(node->cond.get()), trueBlock, falseBlock);

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

// ---------------------------------------------------------------------------
// implicit conversions, one CastKind at a time

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

    //the address of the array is the address of its first element. the gep
    //with two zero indices says so in the types: from [N x T] to its first T
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
        return emitRValue(sub);

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

    //to bool: compare with zero
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
        //bool is 0 or 1, unsigned. char and int are signed
        CastOpKind op = from->is(sema::TypeKind::Bool) ? CastOpKind::UIToFP : CastOpKind::SIToFP;
        return builder.createCast(op, value, from, to);
    }

    if (sema::isFloating(from)) return builder.createCast(CastOpKind::FPToSI, value, from, to);

    unsigned fromBits = integerBits(from);
    unsigned toBits = integerBits(to);
    if (toBits < fromBits) return builder.createCast(CastOpKind::Trunc, value, from, to);
    CastOpKind op = from->is(sema::TypeKind::Bool) ? CastOpKind::ZExt : CastOpKind::SExt;
    return builder.createCast(op, value, from, to);
}

//the size of count objects of a type, in bytes, without knowing the target:
//the address of element count of an array that starts at address 0
//    %end = getelementptr %T, ptr null, i64 %count
//    %bytes = ptrtoint ptr %end to i64
Value *IRVisitor::sizeOf(const sema::Type *type, Value *count) {
    std::vector<Value *> indices;
    indices.push_back(count);
    Value *end = builder.createGEP(type, module.getNull(module.pointerType()), indices, false);
    return builder.createCast(CastOpKind::PtrToInt, end, module.pointerType(), int64Type());
}

} // namespace ir
