
#include "ir/ir_visitor.h"

namespace ir {

namespace {

ast::Expr *withoutParens(ast::Expr *expr) {
    while (expr->getKind() == ast::ASTKind::ParenExpr) expr = static_cast<ast::ParenExpr *>(expr)->sub.get();
    return expr;
}

}

Value *IRVisitor::emitCall(ast::CallExpr *node) {
    if (node->function) return emitDirectCall(node, node->function);

    ast::Expr *callee = node->callee.get();
    const sema::Type *calleeType = callee->type;

    if (const sema::ClosureType *closure = sema::asClosure(calleeType)) {
        Function *fn = lambdaFunction(closure->lambda);
        std::vector<Value *> args;
        args.push_back(emitAddressOf(callee));
        std::vector<Value *> rest = emitArguments(closure->signature, node->args);
        args.insert(args.end(), rest.begin(), rest.end());
        return builder.createCall(fn, lambdaSignature(closure->lambda), args);
    }

    const sema::FunctionType *signature = sema::asFunction(calleeType);
    Value *target = nullptr;
    if (signature) {
        target = emitLValue(callee);
    } else {
        signature = sema::asFunction(sema::asPointer(calleeType)->pointee);
        target = emitRValue(callee);
    }
    return builder.createCall(target, signature, emitArguments(signature, node->args));
}

Value *IRVisitor::emitDirectCall(ast::CallExpr *node, sema::FunctionSymbol *fn) {
    std::vector<Value *> args;
    ast::Expr *callee = withoutParens(node->callee.get());

    if (fn->isMethod()) {
        Value *object = nullptr;
        const sema::RecordSymbol *record = nullptr;
        if (callee->getKind() == ast::ASTKind::MemberExpr) {
            ast::MemberExpr *member = static_cast<ast::MemberExpr *>(callee);
            if (member->isArrow) {
                object = emitRValue(member->base.get());
                record = sema::asRecord(sema::asPointer(member->base->type)->pointee);
            } else {
                object = emitAddressOf(member->base.get());
                record = sema::asRecord(member->base->type);
            }
        } else {

            object = ctx().thisPtr;
            record = ctx().thisRecord;
        }
        args.push_back(adjustToBase(object, record, fn->parentRecord, false));
    } else if (callee->getKind() == ast::ASTKind::MemberExpr) {

        emitIgnored(static_cast<ast::MemberExpr *>(callee)->base.get());
    }

    std::vector<Value *> rest = emitArguments(fn->signature(), node->args);
    args.insert(args.end(), rest.begin(), rest.end());
    return builder.createCall(functionFor(fn), fn->loweredType, args);
}

std::vector<Value *> IRVisitor::emitArguments(const sema::FunctionType *signature, ast::NodeList<ast::Expr> &args) {
    std::vector<Value *> values;
    for (size_t i = 0; i < args.size(); i++) {
        bool byReference = i < signature->params.size() && sema::asReference(signature->params[i]);
        values.push_back(byReference ? emitLValue(args[i].get()) : emitRValue(args[i].get()));
    }
    return values;
}

const sema::FunctionType *IRVisitor::lambdaSignature(const ast::LambdaExpr *lambda) {
    const sema::FunctionType *signature = sema::asClosure(lambda->type)->signature;
    std::vector<const sema::Type *> params;
    params.push_back(module.pointerType());
    params.insert(params.end(), signature->params.begin(), signature->params.end());
    return symTab.functionType(signature->returnType, params, false);
}

Function *IRVisitor::lambdaFunction(ast::LambdaExpr *lambda) {
    std::string name = lambdaFunctionName(lambda);
    Function *fn = module.findFunction(name);
    if (fn) return fn;
    return module.createFunction(name, lambdaSignature(lambda), Linkage::Internal);
}

void IRVisitor::emitLambdaBody(ast::LambdaExpr *lambda) {
    if (emittedLambdas.count(lambda)) return;
    emittedLambdas.insert(lambda);
    module.addClosure(lambda);

    Function *fn = lambdaFunction(lambda);
    const sema::FunctionType *signature = sema::asClosure(lambda->type)->signature;
    const sema::Type *closureType = lambda->type;
    const sema::RecordSymbol *outerRecord = contexts.empty() ? nullptr : ctx().thisRecord;

    InsertPoint saved = builder.saveInsertPoint();
    startFunction(fn, signature->returnType);
    ctx().thisRecord = outerRecord;

    Argument *closure = fn->addArgument(module.pointerType(), "__closure");
    for (size_t i = 0; i < lambda->usedCaptures.size(); i++) {
        const ast::LambdaCapture &capture = lambda->usedCaptures[i];
        Value *field = builder.createStructGEP(closureType, closure, (unsigned)i);
        if (capture.kind == ast::LambdaCapture::Kind::This) {
            ctx().thisPtr = builder.createLoad(module.pointerType(), field);
            continue;
        }
        VariableBinding binding;
        binding.slot = field;
        binding.slotHoldsAddress = capture.kind == ast::LambdaCapture::Kind::ByRef;
        ctx().locals[capture.symbol] = binding;
    }
    bindParameters(signature, &lambda->params);

    std::vector<CleanupScope> chain;
    collectLabels(lambda->body.get(), chain);
    visit(lambda->body.get());

    finishFunction();
    builder.restoreInsertPoint(saved);
}

void IRVisitor::emitLambdaInto(Value *address, ast::LambdaExpr *lambda) {
    emitLambdaBody(lambda);

    for (size_t i = 0; i < lambda->usedCaptures.size(); i++) {
        const ast::LambdaCapture &capture = lambda->usedCaptures[i];
        Value *field = builder.createStructGEP(lambda->type, address, (unsigned)i);
        if (capture.kind == ast::LambdaCapture::Kind::This) {
            builder.createStore(module.pointerType(), ctx().thisPtr, field);
            continue;
        }
        Value *object = variableAddress(capture.symbol);
        if (capture.kind == ast::LambdaCapture::Kind::ByRef) {
            builder.createStore(module.pointerType(), object, field);
        } else {
            const sema::Type *valueType = sema::stripReference(capture.symbol->type);
            builder.createStore(valueType, builder.createLoad(valueType, object), field);
        }
    }
}

}
