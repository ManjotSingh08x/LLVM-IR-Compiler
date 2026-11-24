#include "ir/cfg.h"

#include <set>

namespace ir {

//BasicBlock

void BasicBlock::append(std::unique_ptr<Instruction> inst) {
    inst->setParent(this);
    instructions.push_back(std::move(inst));
}

void BasicBlock::insertAlloca(std::unique_ptr<Instruction> inst) {
    inst->setParent(this);
    instructions.insert(instructions.begin() + allocaCount, std::move(inst));
    allocaCount++;
}

void BasicBlock::insertBefore(Instruction *pos, std::unique_ptr<Instruction> inst) {
    inst->setParent(this);
    for (size_t i = 0; i < instructions.size(); i++) {
        if (instructions[i].get() == pos) {
            if (i < allocaCount) allocaCount++;
            instructions.insert(instructions.begin() + i, std::move(inst));
            return;
        }
    }
    instructions.push_back(std::move(inst));
}

void BasicBlock::insertAtFront(std::unique_ptr<Instruction> inst) {
    inst->setParent(this);
    instructions.insert(instructions.begin(), std::move(inst));
}

void BasicBlock::erase(Instruction *inst) {
    for (size_t i = 0; i < instructions.size(); i++) {
        if (instructions[i].get() != inst) continue;
        if (i < allocaCount) allocaCount--;
        std::unique_ptr<Instruction> owned = std::move(instructions[i]);
        instructions.erase(instructions.begin() + i);
        owned->setParent(nullptr);
        parent->discard(std::move(owned));
        return;
    }
}

std::unique_ptr<Instruction> BasicBlock::detach(Instruction *inst) {
    for (size_t i = 0; i < instructions.size(); i++) {
        if (instructions[i].get() != inst) continue;
        if (i < allocaCount) allocaCount--;
        std::unique_ptr<Instruction> owned = std::move(instructions[i]);
        instructions.erase(instructions.begin() + i);
        owned->setParent(nullptr);
        return owned;
    }
    return nullptr;
}

BasicBlock *BasicBlock::splitAt(Instruction *inst, const std::string &label) {
    size_t at = 0;
    while (at < instructions.size() && instructions[at].get() != inst) at++;
    BasicBlock *tail = parent->createBlock(label);
    parent->placeBlockAfter(tail, this);
    for (size_t i = at; i < instructions.size(); i++) {
        instructions[i]->setParent(tail);
        tail->instructions.push_back(std::move(instructions[i]));
    }
    instructions.erase(instructions.begin() + at, instructions.end());
    if (allocaCount > instructions.size()) allocaCount = instructions.size();

    //control now reaches the old successors from the tail
    if (Instruction *term = tail->getTerminator()) {
        for (BasicBlock *next : term->getSuccessors()) {
            for (std::unique_ptr<Instruction> &phi : next->instructions) {
                if (phi->getOpcode() != Opcode::Phi) break;
                static_cast<PhiInst *>(phi.get())->replaceIncomingBlock(this, tail);
            }
        }
    }
    return tail;
}

Instruction *BasicBlock::getTerminator() const {
    if (!isTerminated()) return nullptr;
    return instructions.back().get();
}

void BasicBlock::addEdge(BasicBlock *from, BasicBlock *to) {
    from->successors.push_back(to);
    to->predecessors.push_back(from);
}

void BasicBlock::dump(std::ostream &out) const {
    out << name << ":";
    //predecessors are written as a comment, as clang does
    if (!predecessors.empty()) {
        out << "  ; preds = ";
        std::set<const BasicBlock *> written;
        bool first = true;
        for (const BasicBlock *pred : predecessors) {
            if (written.count(pred)) continue;
            written.insert(pred);
            if (!first) out << ", ";
            first = false;
            out << pred->getOperand();
        }
    }
    out << "\n";
    for (const std::unique_ptr<Instruction> &inst : instructions) inst->dump(out);
}

//Function

Argument *Function::addArgument(const sema::Type *type, const std::string &baseName) {
    Argument *arg = new Argument(type, uniqueName(baseName), this, args.size());
    args.push_back(std::unique_ptr<Argument>(arg));
    return arg;
}

BasicBlock *Function::createBlock(const std::string &label) {
    BasicBlock *block = new BasicBlock(uniqueName(label), this);
    ownedBlocks.push_back(std::unique_ptr<BasicBlock>(block));
    return block;
}

void Function::placeBlock(BasicBlock *block) {
    if (block->isPlaced) return;
    block->isPlaced = true;
    layout.push_back(block);
}

std::string Function::uniqueName(const std::string &base) {
    if (usedNames.count(base) == 0) {
        usedNames[base] = 0;
        return base;
    }
    while (true) {
        unsigned suffix = ++usedNames[base];
        std::string candidate = base + "." + std::to_string(suffix);
        if (usedNames.count(candidate) == 0) {
            usedNames[candidate] = 0;
            return candidate;
        }
    }
}

std::string Function::nextTempName() {
    return uniqueName("t" + std::to_string(tempCounter++));
}

void Function::removeBlock(BasicBlock *block) {
    for (size_t i = 0; i < layout.size(); i++) {
        if (layout[i] == block) {
            layout.erase(layout.begin() + i);
            block->isPlaced = false;
            return;
        }
    }
}

void Function::placeBlockBefore(BasicBlock *block, BasicBlock *pos) {
    if (block->isPlaced) return;
    for (size_t i = 0; i < layout.size(); i++) {
        if (layout[i] == pos) {
            layout.insert(layout.begin() + i, block);
            block->isPlaced = true;
            return;
        }
    }
    placeBlock(block);
}

void Function::placeBlockAfter(BasicBlock *block, BasicBlock *pos) {
    if (block->isPlaced) return;
    for (size_t i = 0; i < layout.size(); i++) {
        if (layout[i] == pos) {
            layout.insert(layout.begin() + i + 1, block);
            block->isPlaced = true;
            return;
        }
    }
    placeBlock(block);
}

void Function::recomputeCFG() {
    for (BasicBlock *block : layout) {
        block->predecessors.clear();
        block->successors.clear();
    }
    //only edges between placed blocks count, a removed block is no longer in the graph
    std::set<const BasicBlock *> placed(layout.begin(), layout.end());
    for (BasicBlock *block : layout) {
        Instruction *term = block->getTerminator();
        if (term == nullptr) continue;
        std::set<const BasicBlock *> seen;
        for (BasicBlock *next : term->getSuccessors()) {
            if (!placed.count(next) || seen.count(next)) continue;
            seen.insert(next);
            BasicBlock::addEdge(block, next);
        }
    }
}

void Function::discard(std::unique_ptr<Instruction> inst) {
    discarded.push_back(std::move(inst));
}

void Function::finalizeCFG(Module &module) {
    if (layout.empty()) return;

    //1. every block that can be reached from entry, following the edges
    std::set<BasicBlock *> reached;
    std::vector<BasicBlock *> work;
    work.push_back(layout.front());
    reached.insert(layout.front());
    while (!work.empty()) {
        BasicBlock *block = work.back();
        work.pop_back();
        for (BasicBlock *next : block->successors) {
            if (reached.count(next)) continue;
            reached.insert(next);
            work.push_back(next);
        }
    }

    //2. keep only reachable blocks, appending any that were never written into
    std::vector<BasicBlock *> kept;
    for (BasicBlock *block : layout) {
        if (reached.count(block)) kept.push_back(block);
    }
    for (const std::unique_ptr<BasicBlock> &block : ownedBlocks) {
        if (reached.count(block.get()) && !block->isPlaced) {
            block->isPlaced = true;
            kept.push_back(block.get());
        }
    }
    layout = kept;

    //a removed block can no longer be a predecessor
    for (BasicBlock *block : layout) {
        std::vector<BasicBlock *> preds;
        for (BasicBlock *pred : block->predecessors) {
            if (reached.count(pred)) preds.push_back(pred);
        }
        block->predecessors = preds;
    }

    //3. a block that runs off its end returns the zero value
    const sema::Type *returnType = fnType->returnType;
    for (BasicBlock *block : layout) {
        if (block->isTerminated()) continue;
        if (returnType->is(sema::TypeKind::Void)) {
            block->append(std::unique_ptr<Instruction>(new ReturnInst(returnType, nullptr)));
        } else {
            block->append(std::unique_ptr<Instruction>(new ReturnInst(returnType, module.getZero(returnType))));
        }
    }
}

void Function::dump(std::ostream &out) const {
    if (!isDefinition()) {
        out << "declare " << formatType(fnType->returnType) << " @" << name << "(";
        for (size_t i = 0; i < fnType->params.size(); i++) {
            if (i > 0) out << ", ";
            out << formatType(fnType->params[i]);
        }
        if (fnType->isVariadic) out << (fnType->params.empty() ? "..." : ", ...");
        out << ")\n";
        return;
    }

    out << "define ";
    if (linkage == Linkage::Internal) out << "internal ";
    out << formatType(fnType->returnType) << " @" << name << "(";
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) out << ", ";
        out << args[i]->getTypedOperand();
    }
    out << ") {\n";
    for (size_t i = 0; i < layout.size(); i++) {
        if (i > 0) out << "\n";
        layout[i]->dump(out);
    }
    out << "}\n";
}

//Module

const sema::Type *Module::pointerType() {
    if (genericPointer == nullptr) genericPointer = symTab.pointerTo(sema::voidType());
    return genericPointer;
}

Constant *Module::keep(Constant *constant) {
    constants.push_back(std::unique_ptr<Constant>(constant));
    return constant;
}

Constant *Module::getInt(const sema::Type *type, long long value) {
    Constant *constant = new Constant(ConstantKind::Int, type);
    constant->intValue = normalizeInt(value, type);
    return keep(constant);
}

Constant *Module::getFloat(const sema::Type *type, double value) {
    Constant *constant = new Constant(ConstantKind::Float, type);
    constant->floatValue = type->is(sema::TypeKind::Float) ? (double)(float)value : value;
    return keep(constant);
}

Constant *Module::getNull(const sema::Type *type) {
    return keep(new Constant(ConstantKind::Null, type));
}

Constant *Module::getZero(const sema::Type *type) {
    if (integerBits(type) != 0) return getInt(type, 0);
    if (sema::isFloating(type)) return getFloat(type, 0.0);
    switch (type->getKind()) {
    case sema::TypeKind::Pointer:
    case sema::TypeKind::Reference:
    case sema::TypeKind::Null:
        return getNull(type);
    default:
        return keep(new Constant(ConstantKind::ZeroInit, type));
    }
}

Constant *Module::getString(const sema::Type *arrayType, const std::string &bytes) {
    Constant *constant = new Constant(ConstantKind::String, arrayType);
    constant->bytes = bytes;
    return keep(constant);
}

Constant *Module::getAggregate(const sema::Type *type, const std::vector<const Constant *> &elements) {
    Constant *constant = new Constant(ConstantKind::Aggregate, type);
    constant->elements = elements;
    return keep(constant);
}

Constant *Module::getAddress(const Value *target) {
    Constant *constant = new Constant(ConstantKind::Address, pointerType());
    constant->target = target;
    return keep(constant);
}

GlobalVariable *Module::createGlobal(const std::string &name, const sema::Type *valueType,
                                     const Constant *initializer, Linkage linkage, bool isConstant) {
    GlobalVariable *global = new GlobalVariable(name, valueType, pointerType(), initializer, linkage, isConstant);
    globals.push_back(std::unique_ptr<GlobalVariable>(global));
    globalMap[name] = global;
    return global;
}

GlobalVariable *Module::findGlobal(const std::string &name) const {
    std::map<std::string, GlobalVariable *>::const_iterator it = globalMap.find(name);
    return it == globalMap.end() ? nullptr : it->second;
}

GlobalVariable *Module::getStringLiteral(const std::string &bytesWithNull) {
    std::map<std::string, GlobalVariable *>::iterator it = stringLiterals.find(bytesWithNull);
    if (it != stringLiterals.end()) return it->second;

    std::string name = ".str";
    if (!stringLiterals.empty()) name += "." + std::to_string(stringLiterals.size());
    const sema::Type *arrayType = symTab.arrayOf(sema::charType(), bytesWithNull.size());
    GlobalVariable *global = createGlobal(name, arrayType, getString(arrayType, bytesWithNull),
                                          Linkage::Private, true);
    stringLiterals[bytesWithNull] = global;
    return global;
}

Function *Module::createFunction(const std::string &name, const sema::FunctionType *sourceType, Linkage linkage) {
    Function *fn = new Function(name, abiSignature(sourceType), pointerType(), linkage);
    functions.push_back(std::unique_ptr<Function>(fn));
    functionMap[name] = fn;
    return fn;
}

const sema::FunctionType *Module::abiSignature(const sema::FunctionType *sourceType) {
    std::vector<const sema::Type *> params;
    const sema::Type *returnType = sourceType->returnType;
    if (isPassedByAddress(returnType)) {
        params.push_back(pointerType());
        returnType = sema::voidType();
    }
    for (const sema::Type *param : sourceType->params) {
        params.push_back(isPassedByAddress(param) ? pointerType() : param);
    }
    return symTab.functionType(returnType, params, sourceType->isVariadic);
}

Function *Module::findFunction(const std::string &name) const {
    std::map<std::string, Function *>::const_iterator it = functionMap.find(name);
    return it == functionMap.end() ? nullptr : it->second;
}

Function *Module::getRuntimeMalloc() {
    Function *fn = findFunction("malloc");
    if (fn) return fn;
    std::vector<const sema::Type *> params;
    params.push_back(sema::longType());
    return createFunction("malloc", symTab.functionType(pointerType(), params, false), Linkage::External);
}

Function *Module::getRuntimeFree() {
    Function *fn = findFunction("free");
    if (fn) return fn;
    std::vector<const sema::Type *> params;
    params.push_back(pointerType());
    return createFunction("free", symTab.functionType(sema::voidType(), params, false), Linkage::External);
}

Function *Module::getRuntimeAtexit() {
    Function *fn = findFunction("atexit");
    if (fn) return fn;
    std::vector<const sema::Type *> params;
    params.push_back(pointerType());
    return createFunction("atexit", symTab.functionType(sema::intType(), params, false), Linkage::External);
}

void Module::dump(std::ostream &out) const {
    out << "; ModuleID = '" << sourceName << "'\n";
    out << "source_filename = \"" << sourceName << "\"\n";

    if (!records.empty() || !closures.empty()) out << "\n";
    for (const sema::RecordSymbol *record : records) {
        out << formatRecordName(record) << " = type " << formatRecordBody(record) << "\n";
    }
    for (const ast::LambdaExpr *lambda : closures) {
        out << closureTypeName(lambda) << " = type " << formatClosureBody(lambda) << "\n";
    }

    if (!globals.empty()) out << "\n";
    for (const std::unique_ptr<GlobalVariable> &global : globals) global->dump(out);

    //the function listed here runs before main starts
    if (globalInit) {
        out << "@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] "
            << "[{ i32, ptr, ptr } { i32 65535, ptr @" << globalInit->getName() << ", ptr null }]\n";
    }

    //a declaration is only written for a function the program uses
    for (const std::unique_ptr<Function> &fn : functions) {
        if (!fn->isDefinition() && !fn->isUsed) continue;
        out << "\n";
        fn->dump(out);
    }
}

} // namespace ir
