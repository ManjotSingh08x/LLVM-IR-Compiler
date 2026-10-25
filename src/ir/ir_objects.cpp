#include "ir/ir_visitor.h"

#include "semantic/constants.h"

//object lifetime: initialization, construction, destruction, new, delete and base conversions

namespace ir {

namespace {

//the expression a reference is bound to: int &r = x, int &r{x}, int &r(x)
ast::Expr *referenceSource(ast::VarDecl *decl) {
    switch (decl->initStyle) {
    case ast::InitStyle::Copy:
        return decl->init.get();
    case ast::InitStyle::Brace: {
        ast::InitListExpr *list = static_cast<ast::InitListExpr *>(decl->init.get());
        return list->inits.empty() ? nullptr : list->inits[0].get();
    }
    case ast::InitStyle::CtorArgs:
        return decl->ctorArgs.empty() ? nullptr : decl->ctorArgs[0].get();
    case ast::InitStyle::None:
        break;
    }
    return nullptr;
}

} // namespace

//  initialization 

//gives the object at address its initial value
void IRVisitor::emitObjectInit(Value *address, const sema::Type *type, ast::VarDecl *decl) {
    //a reference stores the address of the object it is bound to
    if (sema::asReference(type)) {
        ast::Expr *source = referenceSource(decl);
        if (source) builder.createStore(type, emitLValue(source), address);
        return;
    }

    switch (decl->initStyle) {
    case ast::InitStyle::None:
        //Dog d; runs the default constructor. int x; leaves x as it is
        emitDefaultConstruct(address, type, decl->constructor);
        return;

    case ast::InitStyle::CtorArgs:
        if (sema::asRecord(type)) {
            if (decl->constructor) {
                emitConstructorCall(decl->constructor, address, decl->ctorArgs);
            } else if (decl->ctorArgs.size() == 1) {
                builder.createStore(type, emitRValue(decl->ctorArgs[0].get()), address);  //Point b(a): a copy
            } else {
                emitDefaultConstruct(address, type, nullptr);
            }
        } else if (!decl->ctorArgs.empty()) {
            builder.createStore(type, emitRValue(decl->ctorArgs[0].get()), address);    //int x(5)
        }
        return;

    case ast::InitStyle::Copy:
    case ast::InitStyle::Brace:
        emitInitValue(address, type, decl->init.get());
        return;
    }
}

//one initializer value: a brace list, a string for a char array, or a lambda
void IRVisitor::emitInitValue(Value *address, const sema::Type *type, ast::Expr *expr) {
    if (expr->getKind() == ast::ASTKind::InitListExpr) {
        emitInitList(address, type, static_cast<ast::InitListExpr *>(expr));
        return;
    }
    const sema::ArrayType *array = sema::asArray(type);
    if (array && expr->getKind() == ast::ASTKind::StringLiteral) {
        //char s[8] = "hi": the characters, the \0, then zeros up to the size
        std::string bytes = sema::decodeStringLiteral(static_cast<ast::StringLiteral *>(expr)->spelling);
        bytes.resize(array->size, '\0');
        builder.createStore(type, module.getString(type, bytes), address);
        return;
    }
    if (expr->getKind() == ast::ASTKind::LambdaExpr) {
        emitLambdaInto(address, static_cast<ast::LambdaExpr *>(expr));
        return;
    }
    if (sema::asReference(type)) {
        builder.createStore(type, emitLValue(expr), address);
        return;
    }
    builder.createStore(type, emitRValue(expr), address);
}

//{ ... }: constructor arguments, or an element or field-wise fill, the rest zero or default constructed
void IRVisitor::emitInitList(Value *address, const sema::Type *type, ast::InitListExpr *list) {
    if (list->constructor) {
        emitConstructorCall(list->constructor, address, list->inits);
        return;
    }

    if (const sema::ArrayType *array = sema::asArray(type)) {
        uint64_t given = list->inits.size();
        bool elementsNeedCode = layout.needsConstruction(array->element);
        if (given < array->size && !elementsNeedCode) {
            builder.createStore(type, module.getZero(type), address);
        }
        for (uint64_t i = 0; i < given && i < array->size; i++) {
            std::vector<Value *> indices;
            indices.push_back(module.getInt64(0));
            indices.push_back(module.getInt64((long long)i));
            Value *element = builder.createGEP(type, address, indices);
            emitInitValue(element, array->element, list->inits[i].get());
        }
        if (given < array->size && elementsNeedCode) {
            emitElementLoop(address, array->element, module.getInt64((long long)given),
                            module.getInt64((long long)array->size), ElementAction::DefaultConstruct, nullptr);
        }
        return;
    }

    if (sema::RecordSymbol *record = sema::asRecord(type)) {
        if (list->inits.size() < record->fields.size()) {
            builder.createStore(type, module.getZero(type), address);
        }
        for (size_t i = 0; i < record->fields.size(); i++) {
            const sema::Symbol *field = record->fields[i];
            if (i < list->inits.size()) {
                Value *fieldAddr = builder.createStructGEP(type, address, RecordLayout::fieldIndex(record, field));
                emitInitValue(fieldAddr, field->type, list->inits[i].get());
            } else {
                emitFieldDefaultInit(address, record, field);
            }
        }
        return;
    }

    //int a{5} or int b{}
    if (list->inits.empty()) {
        builder.createStore(type, module.getZero(type), address);
    } else {
        emitInitValue(address, type, list->inits[0].get());
    }
}

//a field without an initializer: its in-class initializer or default construction
void IRVisitor::emitFieldDefaultInit(Value *object, const sema::RecordSymbol *record, const sema::Symbol *field) {
    ast::VarDecl *decl = RecordLayout::fieldDeclaration(field);
    if (decl == nullptr && !layout.needsConstruction(field->type)) return;  //int plain; needs no code

    Value *fieldAddr = builder.createStructGEP(record->type, object, RecordLayout::fieldIndex(record, field));
    if (decl == nullptr) {
        emitDefaultConstruct(fieldAddr, field->type, nullptr);
        return;
    }
    Value *savedThis = ctx().thisPtr;
    const sema::RecordSymbol *savedRecord = ctx().thisRecord;
    ctx().thisPtr = object;
    ctx().thisRecord = record;
    emitObjectInit(fieldAddr, field->type, decl);
    ctx().thisPtr = savedThis;
    ctx().thisRecord = savedRecord;
}

//  construction 

//default-constructs an object with no initializer, element-wise for arrays
void IRVisitor::emitDefaultConstruct(Value *address, const sema::Type *type, const sema::FunctionSymbol *chosen) {
    if (const sema::ArrayType *array = sema::asArray(type)) {
        if (!layout.needsConstruction(type)) return;
        emitElementLoop(address, array->element, module.getInt64(0), module.getInt64((long long)array->size),
                        ElementAction::DefaultConstruct, chosen);
        return;
    }

    sema::RecordSymbol *record = sema::asRecord(type);
    if (record == nullptr) return;

    const sema::FunctionSymbol *ctor = chosen;
    if (ctor == nullptr && !record->constructors.empty()) {
        ctor = RecordLayout::defaultConstructor(record);
        if (ctor == nullptr) {
            error(record->line, record->col, "'" + record->qualifiedName() +
                  "' has no constructor without arguments, but one of its objects is made without arguments");
            return;
        }
    }
    if (ctor) {
        ast::NodeList<ast::Expr> noArgs;
        emitConstructorCall(ctor, address, noArgs);
    } else if (layout.needsConstruction(record)) {
        std::vector<Value *> args;
        args.push_back(address);
        Function *fn = implicitStructor(record, true);
        builder.createCall(fn, fn->fnType, args);
    }
}

void IRVisitor::emitConstructorCall(const sema::FunctionSymbol *ctor, Value *address, ast::NodeList<ast::Expr> &args) {
    std::vector<Value *> values;
    values.push_back(address);
    std::vector<Value *> rest = emitArguments(ctor->signature(), args);
    values.insert(values.end(), rest.begin(), rest.end());
    Function *fn = functionFor(ctor);
    builder.createCall(fn, fn->fnType, values);
}

//the start of every constructor: bases in order, then the fields in order
void IRVisitor::emitMemberInitialization(const sema::RecordSymbol *record, Value *thisPtr) {
    for (size_t i = 0; i < record->bases.size(); i++) {
        Value *base = builder.createStructGEP(record->type, thisPtr, (unsigned)i);
        emitDefaultConstruct(base, record->bases[i].record->type, nullptr);
    }
    for (const sema::Symbol *field : record->fields) emitFieldDefaultInit(thisPtr, record, field);
}

//  destruction 

//the end of every destructor: fields in reverse order, then bases in reverse
void IRVisitor::emitMemberDestruction(const sema::RecordSymbol *record, Value *thisPtr) {
    for (size_t i = record->fields.size(); i > 0; i--) {
        const sema::Symbol *field = record->fields[i - 1];
        if (!layout.needsDestruction(field->type)) continue;
        emitDestroy(builder.createStructGEP(record->type, thisPtr, RecordLayout::fieldIndex(record, field)),
                    field->type);
    }
    for (size_t i = record->bases.size(); i > 0; i--) {
        Value *base = builder.createStructGEP(record->type, thisPtr, (unsigned)(i - 1));
        emitDestroy(base, record->bases[i - 1].record->type);
    }
}

void IRVisitor::emitDestroy(Value *address, const sema::Type *type) {
    if (!layout.needsDestruction(type)) return;
    if (const sema::ArrayType *array = sema::asArray(type)) {
        emitElementLoop(address, array->element, module.getInt64(0), module.getInt64((long long)array->size),
                        ElementAction::Destroy, nullptr);
        return;
    }
    std::vector<Value *> args;
    args.push_back(address);
    Function *fn = destructorFunction(sema::asRecord(type));
    builder.createCall(fn, fn->fnType, args);
}

Function *IRVisitor::destructorFunction(const sema::RecordSymbol *record) {
    if (record->destructor) return functionFor(record->destructor);
    return implicitStructor(record, false);
}

//the implicit constructor or destructor for a record that needs one, body emitted later
Function *IRVisitor::implicitStructor(const sema::RecordSymbol *record, bool isConstructor) {
    std::string name = "class_" + RecordLayout::recordPath(record) + (isConstructor ? "_C_v" : "_D_v");
    Function *fn = module.findFunction(name);
    if (fn) return fn;

    std::vector<const sema::Type *> params;
    params.push_back(symTab.pointerTo(record->type));
    fn = module.createFunction(name, symTab.functionType(sema::voidType(), params, false), Linkage::External);
    PendingStructor pending;
    pending.function = fn;
    pending.record = record;
    pending.isConstructor = isConstructor;
    pendingStructors.push_back(pending);
    return fn;
}

//  array element loops, destruction runs backwards 

void IRVisitor::emitElementLoop(Value *first, const sema::Type *elementType, Value *begin, Value *end,
                                ElementAction action, const sema::FunctionSymbol *ctor) {
    bool backwards = action == ElementAction::Destroy;
    AllocaInst *counter = builder.createEntryAlloca(int64Type(), "arr.i");
    builder.createStore(int64Type(), backwards ? end : begin, counter);

    BasicBlock *condBlock = builder.createBlock("arr.cond");
    BasicBlock *bodyBlock = builder.createBlock("arr.body");
    BasicBlock *endBlock = builder.createBlock("arr.end");
    builder.createBr(condBlock);

    builder.setInsertBlock(condBlock);
    Value *index = builder.createLoad(int64Type(), counter);
    Value *done = backwards ? builder.createBinary(BinaryOpKind::ICmpULE, int64Type(), index, begin)
                            : builder.createBinary(BinaryOpKind::ICmpUGE, int64Type(), index, end);
    builder.createCondBr(done, endBlock, bodyBlock);

    builder.setInsertBlock(bodyBlock);
    Value *current = index;
    if (backwards) {
        current = builder.createBinary(BinaryOpKind::Sub, int64Type(), index, module.getInt64(1));
        builder.createStore(int64Type(), current, counter);
    }
    Value *element = builder.createElementGEP(elementType, first, current);
    emitElementAction(element, elementType, action, ctor);
    if (!backwards) {
        Value *next = builder.createBinary(BinaryOpKind::Add, int64Type(), current, module.getInt64(1));
        builder.createStore(int64Type(), next, counter);
    }
    builder.createBr(condBlock);

    builder.setInsertBlock(endBlock);
}

void IRVisitor::emitElementAction(Value *element, const sema::Type *elementType, ElementAction action,
                                  const sema::FunctionSymbol *ctor) {
    switch (action) {
    case ElementAction::DefaultConstruct:
        emitDefaultConstruct(element, elementType, ctor);
        break;
    case ElementAction::ZeroFill:
        builder.createStore(elementType, module.getZero(elementType), element);
        break;
    case ElementAction::Destroy:
        emitDestroy(element, elementType);
        break;
    }
}

//  fields and base classes 

//the address of a field, moved into the base subobject when inherited
Value *IRVisitor::fieldAddress(Value *object, const sema::RecordSymbol *objectRecord, const sema::Symbol *field) {
    const sema::RecordSymbol *owner = field->parentRecord;
    Value *base = adjustToBase(object, objectRecord, owner, false);
    return builder.createStructGEP(owner->type, base, RecordLayout::fieldIndex(owner, field));
}

//the address of a base subobject, a null pointer stays null via select
Value *IRVisitor::adjustToBase(Value *address, const sema::RecordSymbol *from, const sema::RecordSymbol *to,
                               bool keepNull) {
    if (from == to || from == nullptr || to == nullptr) return address;

    std::vector<unsigned> path;
    if (!RecordLayout::findBasePath(from, to, path)) return address;

    Value *result = address;
    const sema::RecordSymbol *current = from;
    bool moves = false;
    for (unsigned index : path) {
        result = builder.createStructGEP(current->type, result, index);
        if (index != 0) moves = true;
        current = current->bases[index].record;
    }
    if (!keepNull || !moves) return result;

    Value *isNull = builder.createBinary(BinaryOpKind::ICmpEQ, module.pointerType(), address,
                                         module.getNull(module.pointerType()));
    return builder.createSelect(isNull, module.getNull(module.pointerType()), result, module.pointerType());
}

//  new and delete 

Value *IRVisitor::callMalloc(Value *bytes) {
    Function *fn = module.getRuntimeMalloc();
    std::vector<Value *> args;
    args.push_back(bytes);
    return builder.createCall(fn, fn->fnType, args);
}

void IRVisitor::callFree(Value *pointer) {
    Function *fn = module.getRuntimeFree();
    std::vector<Value *> args;
    args.push_back(pointer);
    builder.createCall(fn, fn->fnType, args);
}

//new T and new T[n], with an 8 byte count cookie in front when elements have destructors
Value *IRVisitor::emitNew(ast::NewExpr *node) {
    //the expression type is a pointer to the element, NewExpr::type is as written
    const sema::Type *resultType = static_cast<ast::Expr *>(node)->type;
    const sema::Type *element = sema::asPointer(resultType)->pointee;

    if (node->form == ast::NewForm::Plain || node->form == ast::NewForm::Args || node->form == ast::NewForm::InitList) {
        Value *object = callMalloc(sizeOf(element, module.getInt64(1)));
        if (node->form == ast::NewForm::Plain) {
            emitDefaultConstruct(object, element, node->constructor);
        } else if (node->form == ast::NewForm::InitList) {
            emitInitList(object, element, static_cast<ast::InitListExpr *>(node->init.get()));
        } else if (sema::asRecord(element)) {
            if (node->constructor) {
                emitConstructorCall(node->constructor, object, node->args);
            } else if (node->args.size() == 1) {
                builder.createStore(element, emitRValue(node->args[0].get()), object);
            } else {
                emitDefaultConstruct(object, element, nullptr);
            }
        } else if (node->args.empty()) {
            builder.createStore(element, module.getZero(element), object);  //new int() is 0
        } else {
            builder.createStore(element, emitRValue(node->args[0].get()), object);
        }
        return object;
    }

    //new T[n] and new T[n]{...}
    Value *count = builder.createCast(CastOpKind::SExt, emitRValue(node->dims[0].get()), sema::intType(), int64Type());
    Value *bytes = sizeOf(element, count);
    Value *elements = nullptr;
    if (layout.needsDestruction(element)) {
        Value *withCookie = builder.createBinary(BinaryOpKind::Add, int64Type(), bytes, module.getInt64(8));
        Value *raw = callMalloc(withCookie);
        builder.createStore(int64Type(), count, raw);
        elements = builder.createElementGEP(sema::charType(), raw, module.getInt64(8));
    } else {
        elements = callMalloc(bytes);
    }

    Value *initialised = module.getInt64(0);
    if (node->form == ast::NewForm::ArrayInit) {
        ast::InitListExpr *list = static_cast<ast::InitListExpr *>(node->init.get());
        for (size_t i = 0; i < list->inits.size(); i++) {
            Value *slot = builder.createElementGEP(element, elements, module.getInt64((long long)i));
            emitInitValue(slot, element, list->inits[i].get());
        }
        initialised = module.getInt64((long long)list->inits.size());
        //the elements the list leaves out are zero, or default constructed
        ElementAction rest = layout.needsConstruction(element) ? ElementAction::DefaultConstruct : ElementAction::ZeroFill;
        emitElementLoop(elements, element, initialised, count, rest, nullptr);
    } else if (layout.needsConstruction(element)) {
        emitElementLoop(elements, element, initialised, count, ElementAction::DefaultConstruct, node->constructor);
    }
    return elements;
}

//delete and delete[], null skips the destructor, delete[] reads the cookie
Value *IRVisitor::emitDelete(ast::DeleteExpr *node) {
    Value *pointer = emitRValue(node->operand.get());
    const sema::Type *element = sema::asPointer(node->operand->type)->pointee;

    if (!layout.needsDestruction(element)) {
        callFree(pointer);
        return nullptr;
    }

    BasicBlock *destroyBlock = builder.createBlock("delete.notnull");
    BasicBlock *endBlock = builder.createBlock("delete.end");
    Value *isNull = builder.createBinary(BinaryOpKind::ICmpEQ, module.pointerType(), pointer,
                                         module.getNull(module.pointerType()));
    builder.createCondBr(isNull, endBlock, destroyBlock);

    builder.setInsertBlock(destroyBlock);
    if (node->isArray) {
        Value *raw = builder.createElementGEP(sema::charType(), pointer, module.getInt64(-8));
        Value *count = builder.createLoad(int64Type(), raw);
        emitElementLoop(pointer, element, module.getInt64(0), count, ElementAction::Destroy, nullptr);
        callFree(raw);
    } else {
        emitDestroy(pointer, element);
        callFree(pointer);
    }
    builder.createBr(endBlock);

    builder.setInsertBlock(endBlock);
    return nullptr;
}

} // namespace ir
