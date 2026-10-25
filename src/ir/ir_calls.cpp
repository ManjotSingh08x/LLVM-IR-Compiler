#include "ir/ir_visitor.h"

//calls of every kind, and lambdas

namespace ir {

namespace {

ast::Expr *withoutParens(ast::Expr *expr) {
    while (expr->getKind() == ast::ASTKind::ParenExpr) expr = static_cast<ast::ParenExpr *>(expr)->sub.get();
    return expr;
}

} // namespace

// ---- calls ----

//callee order: named function, lambda, function lvalue, function pointer
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
        return builder.createCall(fn, fn->fnType, args);
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

//a method takes the object, converted to the method's class, as its first argument
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
            //a method named without an object is called on this
            object = ctx().thisPtr;
            record = ctx().thisRecord;
        }
        args.push_back(adjustToBase(object, record, fn->parentRecord, false));
    } else if (callee->getKind() == ast::ASTKind::MemberExpr) {
        //a static method through an object: no this, but the object is still evaluated
        emitIgnored(static_cast<ast::MemberExpr *>(callee)->base.get());
    }

    std::vector<Value *> rest = emitArguments(fn->signature(), node->args);
    args.insert(args.end(), rest.begin(), rest.end());
    Function *function = functionFor(fn);
    return builder.createCall(function, function->fnType, args);
}

//a reference argument passes the address of the object
std::vector<Value *> IRVisitor::emitArguments(const sema::FunctionType *signature, ast::NodeList<ast::Expr> &args) {
    std::vector<Value *> values;
    for (size_t i = 0; i < args.size(); i++) {
        bool byReference = i < signature->params.size() && sema::asReference(signature->params[i]);
        values.push_back(byReference ? emitLValue(args[i].get()) : emitRValue(args[i].get()));
    }
    return values;
}

// ---- lambdas: a closure struct of captures plus a function taking it ----

Function *IRVisitor::lambdaFunction(ast::LambdaExpr *lambda) {
    std::string name = lambdaFunctionName(lambda);
    Function *fn = module.findFunction(name);
    if (fn) return fn;

    const sema::FunctionType *signature = sema::asClosure(lambda->type)->signature;
    std::vector<const sema::Type *> params;
    params.push_back(module.pointerType());
    params.insert(params.end(), signature->params.begin(), signature->params.end());
    return module.createFunction(name, symTab.functionType(signature->returnType, params, false), Linkage::Internal);
}

//the lambda body is lowered as its own function, saving and restoring the current context
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

    //each capture is a closure field, by-reference captures hold the address
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

//fills a closure with captured values, addresses, or this
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

} // namespace ir
