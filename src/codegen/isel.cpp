#include "codegen/isel.h"

#include "ir/ir_types.h"

namespace codegen {

namespace {

const Reg kIntArgRegs[] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};
const size_t kIntArgCount = 6;
const size_t kFloatArgCount = 8;

Reg floatArgReg(size_t index) { return (Reg)((int)Reg::XMM0 + (int)index); }

std::string imm(long long value) { return "$" + std::to_string(value); }

//integer ops on bytes are done 32 bits wide, the low byte comes out the same
Width aluWidth(Width width) { return width == Width::Byte ? Width::Long : width; }

bool isIntCompare(ir::BinaryOpKind op) {
    return ir::isCompare(op) && op != ir::BinaryOpKind::FCmpOEQ && op != ir::BinaryOpKind::FCmpUNE &&
           op != ir::BinaryOpKind::FCmpOLT && op != ir::BinaryOpKind::FCmpOGT &&
           op != ir::BinaryOpKind::FCmpOLE && op != ir::BinaryOpKind::FCmpOGE;
}

bool isFloatOp(ir::BinaryOpKind op) {
    switch (op) {
    case ir::BinaryOpKind::FAdd:
    case ir::BinaryOpKind::FSub:
    case ir::BinaryOpKind::FMul:
    case ir::BinaryOpKind::FDiv:
    case ir::BinaryOpKind::FCmpOEQ:
    case ir::BinaryOpKind::FCmpUNE:
    case ir::BinaryOpKind::FCmpOLT:
    case ir::BinaryOpKind::FCmpOGT:
    case ir::BinaryOpKind::FCmpOLE:
    case ir::BinaryOpKind::FCmpOGE:
        return true;
    default:
        return false;
    }
}

//the condition code an integer compare sets: e, ne, l, g, ...
const char *conditionCode(ir::BinaryOpKind op) {
    switch (op) {
    case ir::BinaryOpKind::ICmpEQ:  return "e";
    case ir::BinaryOpKind::ICmpNE:  return "ne";
    case ir::BinaryOpKind::ICmpSLT: return "l";
    case ir::BinaryOpKind::ICmpSGT: return "g";
    case ir::BinaryOpKind::ICmpSLE: return "le";
    case ir::BinaryOpKind::ICmpSGE: return "ge";
    case ir::BinaryOpKind::ICmpULT: return "b";
    case ir::BinaryOpKind::ICmpUGT: return "a";
    case ir::BinaryOpKind::ICmpULE: return "be";
    case ir::BinaryOpKind::ICmpUGE: return "ae";
    default:                        return "e";
    }
}

std::string invert(const std::string &cc) {
    if (cc == "e") return "ne";
    if (cc == "ne") return "e";
    if (cc == "l") return "ge";
    if (cc == "ge") return "l";
    if (cc == "g") return "le";
    if (cc == "le") return "g";
    if (cc == "b") return "ae";
    if (cc == "ae") return "b";
    if (cc == "a") return "be";
    if (cc == "be") return "a";
    return "ne";
}

const ir::Constant *asConstant(const ir::Value *value) {
    if (value->getKind() != ir::ValueKind::Constant) return nullptr;
    return static_cast<const ir::Constant *>(value);
}

} // namespace

FunctionEmitter::FunctionEmitter(ir::Function &fn, passes::AnalysisManager &am, TargetLayout &layout,
                                 ConstantPool &pool, int id)
    : fn(fn), am(am), layout(layout), pool(pool), id(id), ra(frame, body) {}

void FunctionEmitter::fail(const std::string &message) {
    if (errorText.empty()) errorText = "@" + fn.getName() + ": " + message;
}

std::string FunctionEmitter::label(const ir::BasicBlock *block) const {
    return ".LBB" + std::to_string(id) + "_" + block->getName();
}

std::string FunctionEmitter::localLabel() { return ".Ltmp" + std::to_string(id) + "_" + std::to_string(localCount++); }

bool FunctionEmitter::isRegisterValue(const ir::Value *value) const {
    if (value->getKind() == ir::ValueKind::Argument) return true;
    if (value->getKind() != ir::ValueKind::Instruction) return false;
    return !frame.isAlloca(value);
}

//  function structure

bool FunctionEmitter::emit(AsmLines &out) {
    layoutFrame();
    AsmLines entry;
    storeIncomingArguments(entry);

    const std::vector<ir::BasicBlock *> &blocks = fn.getBlocks();
    for (size_t i = 0; i < blocks.size(); i++) {
        emitBlock(blocks[i], i + 1 < blocks.size() ? blocks[i + 1] : nullptr);
    }
    if (!errorText.empty()) return false;

    //the frame is complete only now: homes were made on demand, then the save slots
    for (Reg reg : ra.usedCalleeSaved()) frame.saveSlot(reg);
    uint64_t size = frame.size();
    std::string name = symbolName(&fn);

    out.push_back("\t.text");
    if (fn.linkage == ir::Linkage::External) out.push_back("\t.globl\t" + name);
    out.push_back("\t.p2align\t4");
    out.push_back("\t.type\t" + name + ",@function");
    out.push_back(name + ":");
    out.push_back("\tpushq\t%rbp");
    out.push_back("\tmovq\t%rsp, %rbp");
    if (size != 0) out.push_back("\tsubq\t" + imm((long long)size) + ", %rsp");
    for (Reg reg : ra.usedCalleeSaved()) {
        out.push_back("\tmovq\t" + regName(reg) + ", " + Mem::frame(frame.saveSlot(reg)).str());
    }
    out.insert(out.end(), entry.begin(), entry.end());
    out.insert(out.end(), body.begin(), body.end());
    out.push_back(".Lret" + std::to_string(id) + ":");
    for (Reg reg : ra.usedCalleeSaved()) {
        out.push_back("\tmovq\t" + Mem::frame(frame.saveSlot(reg)).str() + ", " + regName(reg));
    }
    out.push_back("\tleave");
    out.push_back("\tret");
    out.push_back("\t.size\t" + name + ", .-" + name);
    return true;
}

void FunctionEmitter::layoutFrame() {
    for (ir::BasicBlock *block : fn.getBlocks()) {
        for (const std::unique_ptr<ir::Instruction> &inst : block->instructions) {
            if (inst->getOpcode() != ir::Opcode::Alloca) continue;
            ir::AllocaInst *alloca = static_cast<ir::AllocaInst *>(inst.get());
            const sema::Type *type = alloca->getAccessType();
            frame.addAlloca(alloca, layout.sizeOf(type), layout.alignOf(type));
        }
    }
}

//arguments arrive in registers (or above the return address) and are stored to their homes
void FunctionEmitter::storeIncomingArguments(AsmLines &lines) {
    size_t    intUsed = 0;
    size_t    floatUsed = 0;
    long long stackOffset = 16;
    for (const std::unique_ptr<ir::Argument> &arg : fn.args) {
        const sema::Type *type = arg->getType();
        if (isFloatType(type)) {
            if (floatUsed < kFloatArgCount) {
                std::string home = Mem::frame(frame.home(arg.get())).str();
                lines.push_back(std::string("\t") + (isDoubleType(type) ? "movsd" : "movss") + "\t" +
                                regName(floatArgReg(floatUsed++)) + ", " + home);
            } else {
                frame.setHome(arg.get(), stackOffset);
                stackOffset += 8;
            }
        } else if (isIntegerLike(type)) {
            if (intUsed < kIntArgCount) {
                lines.push_back("\tmovq\t" + regName(kIntArgRegs[intUsed++]) + ", " +
                                Mem::frame(frame.home(arg.get())).str());
            } else {
                frame.setHome(arg.get(), stackOffset);
                stackOffset += 8;
            }
        } else {
            fail("argument %" + arg->getName() + " has an aggregate type, which is not supported");
        }
    }
}

bool FunctionEmitter::isFusedCompare(const ir::BasicBlock *block, size_t index) const {
    const ir::Instruction *inst = block->instructions[index].get();
    if (inst->getOpcode() != ir::Opcode::Binary) return false;
    if (!isIntCompare(static_cast<const ir::BinaryInst *>(inst)->op)) return false;
    if (index + 2 != block->instructions.size()) return false;
    const ir::Instruction *term = block->instructions[index + 1].get();
    if (term->getOpcode() != ir::Opcode::Br) return false;
    const ir::BranchInst *br = static_cast<const ir::BranchInst *>(term);
    if (br->condition != inst) return false;
    //the branch must be its only reader, then the i1 never needs to exist
    return am.getDefUse(fn).users(inst).size() == 1;
}

void FunctionEmitter::emitBlock(ir::BasicBlock *block, ir::BasicBlock *next) {
    ra.beginBlock();
    nextUse.clear();
    const passes::ValueSet &liveOut = am.getLiveness(fn).liveOut(block);
    NextUse info(*block, liveOut);

    body.push_back(label(block) + ":");
    for (size_t i = 0; i < block->instructions.size(); i++) {
        ir::Instruction *inst = block->instructions[i].get();
        if (inst->isTerminator()) {
            emitTerminator(inst, info.at(i), liveOut, next);
        } else if (isFusedCompare(block, i)) {
            fused.insert(inst);
        } else {
            emitInstruction(inst, info.at(i));
        }
    }
}

void FunctionEmitter::emitInstruction(ir::Instruction *inst, const InstNextUse &info) {
    switch (inst->getOpcode()) {
    case ir::Opcode::Alloca:
        return;  //its address is fixed in the frame
    case ir::Opcode::Load:
        return emitLoad(static_cast<ir::LoadInst *>(inst), info);
    case ir::Opcode::Store:
        return emitStore(static_cast<ir::StoreInst *>(inst), info);
    case ir::Opcode::GEP:
        return emitGEP(static_cast<ir::GEPInst *>(inst), info);
    case ir::Opcode::Call:
        return emitCall(static_cast<ir::CallInst *>(inst), info);
    case ir::Opcode::Binary: {
        ir::BinaryInst *binary = static_cast<ir::BinaryInst *>(inst);
        if (isFloatOp(binary->op)) return emitFloatBinary(binary, info);
        return emitBinary(binary, info);
    }
    case ir::Opcode::Unary:
        return emitUnary(static_cast<ir::UnaryInst *>(inst), info);
    case ir::Opcode::Cast:
        return emitCast(static_cast<ir::CastInst *>(inst), info);
    case ir::Opcode::Select:
        return emitSelect(static_cast<ir::SelectInst *>(inst), info);
    case ir::Opcode::Phi:
        return emitPhi(static_cast<ir::PhiInst *>(inst), info);
    default:
        return fail("unexpected terminator in the middle of a block");
    }
}

//  operands

//any integer or pointer value into dst: constants, addresses of globals and allocas, cached or home values
void FunctionEmitter::loadInt(const ir::Value *value, Reg dst, Width width) {
    const std::string d = regName(dst, width == Width::Quad ? Width::Quad : Width::Long);
    switch (value->getKind()) {
    case ir::ValueKind::Constant: {
        const ir::Constant *c = asConstant(value);
        if (c->constKind == ir::ConstantKind::Address) return loadInt(c->target, dst, width);
        long long v = c->constKind == ir::ConstantKind::Int ? c->intValue : 0;
        if (v == 0) {
            line("xorl\t" + regName(dst, Width::Long) + ", " + regName(dst, Width::Long));
        } else if (width != Width::Quad) {
            line("movl\t" + imm(v) + ", " + d);
        } else if (fitsInt32(v)) {
            line("movq\t" + imm(v) + ", " + d);
        } else {
            line("movabsq\t" + imm(v) + ", " + d);
        }
        return;
    }
    case ir::ValueKind::GlobalVariable:
        line("leaq\t" + Mem::global(symbolName(value)).str() + ", " + regName(dst));
        return;
    case ir::ValueKind::Function: {
        const ir::Function *callee = static_cast<const ir::Function *>(value);
        if (callee->isDefinition()) {
            line("leaq\t" + Mem::global(symbolName(value)).str() + ", " + regName(dst));
        } else {
            line("movq\t" + symbolName(value) + "@GOTPCREL(%rip), " + regName(dst));
        }
        return;
    }
    default:
        break;
    }
    if (frame.isAlloca(value)) {
        line("leaq\t" + Mem::frame(frame.allocaOffset(value)).str() + ", " + regName(dst));
        return;
    }
    Reg held;
    if (ra.registerOf(value, held)) {
        if (held == dst) return;
        line(std::string(width == Width::Quad ? "movq\t" : "movl\t") + regName(held, width == Width::Quad ? Width::Quad : Width::Long) +
             ", " + d);
        return;
    }
    std::string home = Mem::frame(frame.home(value)).str();
    switch (width) {
    case Width::Byte: line("movzbl\t" + home + ", " + d); break;
    case Width::Long: line("movl\t" + home + ", " + d); break;
    case Width::Quad: line("movq\t" + home + ", " + d); break;
    }
}

//an operand an alu instruction can read directly: $imm, a register or a home slot
std::string FunctionEmitter::intSource(const ir::Value *value, Width width, Reg scratch) {
    const ir::Constant *c = asConstant(value);
    if (c && c->constKind == ir::ConstantKind::Int && fitsInt32(c->intValue)) return imm(c->intValue);
    if (c && c->constKind == ir::ConstantKind::Null) return "$0";
    if (isRegisterValue(value)) {
        Reg held;
        if (ra.registerOf(value, held)) return regName(held, width);
        return Mem::frame(frame.home(value)).str();
    }
    loadInt(value, scratch, width);
    return regName(scratch, width);
}

void FunctionEmitter::loadFloat(const ir::Value *value, Reg dst) {
    const char *mov = isDoubleType(value->getType()) ? "movsd\t" : "movss\t";
    const ir::Constant *c = asConstant(value);
    if (c) {
        if (c->constKind != ir::ConstantKind::Float) return fail("unexpected floating constant");
        line(mov + Mem::global(pool.label(c->floatValue, isDoubleType(value->getType()))).str() + ", " +
             regName(dst));
        return;
    }
    Reg held;
    if (ra.registerOf(value, held)) {
        if (held != dst) line("movaps\t" + regName(held) + ", " + regName(dst));
        return;
    }
    line(mov + Mem::frame(frame.home(value)).str() + ", " + regName(dst));
}

std::string FunctionEmitter::floatSource(const ir::Value *value, Reg scratch) {
    const ir::Constant *c = asConstant(value);
    if (c && c->constKind == ir::ConstantKind::Float) {
        return Mem::global(pool.label(c->floatValue, isDoubleType(value->getType()))).str();
    }
    if (isRegisterValue(value)) {
        Reg held;
        if (ra.registerOf(value, held)) return regName(held);
        return Mem::frame(frame.home(value)).str();
    }
    loadFloat(value, scratch);
    return regName(scratch);
}

//the memory a pointer points at: allocas and globals directly, anything else through a register
Mem FunctionEmitter::addressOf(const ir::Value *pointer, Reg scratch) {
    if (frame.isAlloca(pointer)) return Mem::frame(frame.allocaOffset(pointer));
    if (pointer->getKind() == ir::ValueKind::GlobalVariable) return Mem::global(symbolName(pointer));
    Reg held;
    if (ra.registerOf(pointer, held)) return Mem::at(held);
    loadInt(pointer, scratch, Width::Quad);
    return Mem::at(scratch);
}

void FunctionEmitter::finishOperands(const InstNextUse &info) {
    for (const std::pair<const ir::Value *const, long> &entry : info.operands) {
        if (entry.second == kDead) {
            ra.release(entry.first);
            nextUse.erase(entry.first);
        } else {
            nextUse[entry.first] = entry.second;
        }
    }
}

void FunctionEmitter::defineResult(const ir::Instruction *inst, const InstNextUse &info, Reg src) {
    finishOperands(info);
    if (info.result == kDead) return;  //never read: no register, no home
    nextUse[inst] = info.result;
    Reg reg = ra.allocate(inst, isXmm(src), nextUse);
    line(std::string(isXmm(src) ? "movaps\t" : "movq\t") + regName(src) + ", " + regName(reg));
}

//  memory

void FunctionEmitter::emitLoad(ir::LoadInst *inst, const InstNextUse &info) {
    const sema::Type *type = inst->getAccessType();
    Mem mem = addressOf(inst->address, Reg::R11);
    if (isFloatType(type)) {
        line(std::string(isDoubleType(type) ? "movsd\t" : "movss\t") + mem.str() + ", %xmm0");
        return defineResult(inst, info, Reg::XMM0);
    }
    if (!isIntegerLike(type)) {
        //a record or array value: its bytes are copied to a frame slot of their own, as the load's snapshot
        uint64_t size = layout.sizeOf(type);
        copyBytes(mem, Mem::frame(frame.aggregateSlot(inst, size, layout.alignOf(type))), size);
        return finishOperands(info);
    }
    switch (widthOf(type)) {
    case Width::Byte: line("movzbl\t" + mem.str() + ", %eax"); break;
    case Width::Long: line("movl\t" + mem.str() + ", %eax"); break;
    case Width::Quad: line("movq\t" + mem.str() + ", %rax"); break;
    }
    defineResult(inst, info, Reg::RAX);
}

void FunctionEmitter::emitStore(ir::StoreInst *inst, const InstNextUse &info) {
    const sema::Type *type = inst->getAccessType();
    Mem mem = addressOf(inst->address, Reg::R11);
    const ir::Constant *c = asConstant(inst->value);

    if (isFloatType(type)) {
        loadFloat(inst->value, Reg::XMM0);
        line(std::string(isDoubleType(type) ? "movsd\t" : "movss\t") + "%xmm0, " + mem.str());
    } else if (isIntegerLike(type)) {
        Width w = widthOf(type);
        Reg   held;
        if (ra.registerOf(inst->value, held)) {
            line(std::string("mov") + suffix(w) + "\t" + regName(held, w) + ", " + mem.str());
        } else if (c && c->constKind == ir::ConstantKind::Int && fitsInt32(c->intValue)) {
            line(std::string("mov") + suffix(w) + "\t" + imm(c->intValue) + ", " + mem.str());
        } else if (c && c->constKind == ir::ConstantKind::Null) {
            line("movq\t$0, " + mem.str());
        } else {
            loadInt(inst->value, Reg::RAX, w);
            line(std::string("mov") + suffix(w) + "\t" + regName(Reg::RAX, w) + ", " + mem.str());
        }
    } else if (c) {
        storeConstant(c, type, mem);
    } else if (frame.isAggregate(inst->value)) {
        uint64_t size = layout.sizeOf(type);
        copyBytes(Mem::frame(frame.aggregateSlot(inst->value, size, layout.alignOf(type))), mem, size);
    } else {
        fail("storing a whole aggregate value that was not loaded is not supported");
    }
    finishOperands(info);
}

//writes a constant of any type at mem: zeroinitializer, strings and nested aggregates field by field
void FunctionEmitter::storeConstant(const ir::Constant *constant, const sema::Type *type, const Mem &mem) {
    switch (constant->constKind) {
    case ir::ConstantKind::Int:
        if (fitsInt32(constant->intValue)) {
            line(std::string("mov") + suffix(widthOf(type)) + "\t" + imm(constant->intValue) + ", " + mem.str());
        } else {
            line("movabsq\t" + imm(constant->intValue) + ", %rax");
            line("movq\t%rax, " + mem.str());
        }
        return;
    case ir::ConstantKind::Float: {
        long long bits = (long long)floatBits(constant->floatValue, isDoubleType(type));
        if (isDoubleType(type)) {
            line("movabsq\t" + imm(bits) + ", %rax");
            line("movq\t%rax, " + mem.str());
        } else {
            line("movl\t" + imm((long long)(int32_t)(uint32_t)bits) + ", " + mem.str());
        }
        return;
    }
    case ir::ConstantKind::Null:
        line("movq\t$0, " + mem.str());
        return;
    case ir::ConstantKind::Address:
        loadInt(constant->target, Reg::RAX, Width::Quad);
        line("movq\t%rax, " + mem.str());
        return;
    case ir::ConstantKind::ZeroInit: {
        uint64_t size = layout.sizeOf(type);
        uint64_t done = 0;
        for (; done + 8 <= size; done += 8) line("movq\t$0, " + mem.plus(done).str());
        for (; done + 4 <= size; done += 4) line("movl\t$0, " + mem.plus(done).str());
        for (; done < size; done++) line("movb\t$0, " + mem.plus(done).str());
        return;
    }
    case ir::ConstantKind::String: {
        const std::string &bytes = constant->bytes;
        size_t done = 0;
        for (; done + 4 <= bytes.size(); done += 4) {
            uint32_t word = 0;
            for (int k = 3; k >= 0; k--) word = (word << 8) | (unsigned char)bytes[done + k];
            line("movl\t" + imm((long long)(int32_t)word) + ", " + mem.plus(done).str());
        }
        for (; done < bytes.size(); done++) {
            line("movb\t" + imm((long long)(signed char)bytes[done]) + ", " + mem.plus(done).str());
        }
        return;
    }
    case ir::ConstantKind::Aggregate: {
        const sema::ArrayType *array = sema::asArray(type);
        for (size_t i = 0; i < constant->elements.size(); i++) {
            if (array) {
                storeConstant(constant->elements[i], array->element, mem.plus(i * layout.sizeOf(array->element)));
            } else {
                storeConstant(constant->elements[i], layout.elementType(type, i),
                              mem.plus(layout.elementOffset(type, i)));
            }
        }
        return;
    }
    }
}

//a block copy through %rax, 8 bytes at a time, then 4, then 1
void FunctionEmitter::copyBytes(const Mem &from, const Mem &to, uint64_t size) {
    uint64_t done = 0;
    for (; done + 8 <= size; done += 8) {
        line("movq\t" + from.plus(done).str() + ", %rax");
        line("movq\t%rax, " + to.plus(done).str());
    }
    for (; done + 4 <= size; done += 4) {
        line("movl\t" + from.plus(done).str() + ", %eax");
        line("movl\t%eax, " + to.plus(done).str());
    }
    for (; done < size; done++) {
        line("movb\t" + from.plus(done).str() + ", %al");
        line("movb\t%al, " + to.plus(done).str());
    }
}

//base + constant offsets folded into one displacement, + index * size for each variable index
void FunctionEmitter::emitGEP(ir::GEPInst *inst, const InstNextUse &info) {
    loadInt(inst->base, Reg::RAX, Width::Quad);
    const sema::Type *current = inst->getAccessType();
    long long         disp = 0;

    for (size_t i = 0; i < inst->indices.size(); i++) {
        const ir::Value  *index = inst->indices[i];
        const ir::Constant *c = asConstant(index);
        long long         step = 0;
        const sema::Type *next = current;

        if (i == 0) {
            step = (long long)layout.sizeOf(current);
        } else if (current->is(sema::TypeKind::Array)) {
            next = sema::asArray(current)->element;
            step = (long long)layout.sizeOf(next);
        } else {
            //a record or closure field: the index is always a constant
            if (!c) return fail("variable index into a record");
            disp += (long long)layout.elementOffset(current, (unsigned)c->intValue);
            current = layout.elementType(current, (unsigned)c->intValue);
            continue;
        }

        if (c) {
            disp += c->intValue * step;
        } else {
            Width w = widthOf(index->getType());
            loadInt(index, Reg::RCX, w);
            if (w == Width::Long) line("movslq\t%ecx, %rcx");
            if (w == Width::Byte) line("movsbq\t%cl, %rcx");
            if (step != 1) line("imulq\t" + imm(step) + ", %rcx");
            line("addq\t%rcx, %rax");
        }
        current = next;
    }
    if (disp != 0) line("leaq\t" + Mem::at(Reg::RAX).plus(disp).str() + ", %rax");
    defineResult(inst, info, Reg::RAX);
}

//  arithmetic

void FunctionEmitter::emitIntCompare(ir::BinaryInst *inst) {
    Width w = widthOf(inst->operandType);
    loadInt(inst->lhs, Reg::RAX, w);
    std::string rhs = intSource(inst->rhs, w, Reg::RCX);
    line(std::string("cmp") + suffix(w) + "\t" + rhs + ", " + regName(Reg::RAX, w));
}

void FunctionEmitter::emitBinary(ir::BinaryInst *inst, const InstNextUse &info) {
    Width w = widthOf(inst->operandType);
    Width aw = aluWidth(w);
    Reg   result = Reg::RAX;

    if (isIntCompare(inst->op)) {
        emitIntCompare(inst);
        line(std::string("set") + conditionCode(inst->op) + "\t%al");
        line("movzbl\t%al, %eax");
        return defineResult(inst, info, Reg::RAX);
    }

    switch (inst->op) {
    case ir::BinaryOpKind::Add:
    case ir::BinaryOpKind::Sub:
    case ir::BinaryOpKind::Mul:
    case ir::BinaryOpKind::And:
    case ir::BinaryOpKind::Or:
    case ir::BinaryOpKind::Xor: {
        const char *name = inst->op == ir::BinaryOpKind::Add   ? "add"
                           : inst->op == ir::BinaryOpKind::Sub ? "sub"
                           : inst->op == ir::BinaryOpKind::Mul ? "imul"
                           : inst->op == ir::BinaryOpKind::And ? "and"
                           : inst->op == ir::BinaryOpKind::Or  ? "or"
                                                               : "xor";
        loadInt(inst->lhs, Reg::RAX, aw);
        std::string rhs = intSource(inst->rhs, aw, Reg::RCX);
        line(std::string(name) + suffix(aw) + "\t" + rhs + ", " + regName(Reg::RAX, aw));
        break;
    }
    case ir::BinaryOpKind::SDiv:
    case ir::BinaryOpKind::SRem:
        loadInt(inst->lhs, Reg::RAX, aw);
        loadInt(inst->rhs, Reg::RCX, aw);
        if (w == Width::Byte) {
            line("movsbl\t%al, %eax");
            line("movsbl\t%cl, %ecx");
        }
        line(aw == Width::Quad ? "cqto" : "cltd");
        line(std::string("idiv") + suffix(aw) + "\t" + regName(Reg::RCX, aw));
        if (inst->op == ir::BinaryOpKind::SRem) result = Reg::RDX;
        break;
    case ir::BinaryOpKind::Shl:
    case ir::BinaryOpKind::AShr:
        loadInt(inst->lhs, Reg::RAX, aw);
        loadInt(inst->rhs, Reg::RCX, aw);
        if (w == Width::Byte && inst->op == ir::BinaryOpKind::AShr) line("movsbl\t%al, %eax");
        line(std::string(inst->op == ir::BinaryOpKind::Shl ? "shl" : "sar") + suffix(aw) + "\t%cl, " +
             regName(Reg::RAX, aw));
        break;
    default:
        return fail("unexpected integer operation");
    }
    defineResult(inst, info, result);
}

void FunctionEmitter::emitFloatBinary(ir::BinaryInst *inst, const InstNextUse &info) {
    bool        dbl = isDoubleType(inst->operandType);
    std::string sfx = dbl ? "sd" : "ss";

    if (!ir::isCompare(inst->op)) {
        const char *name = inst->op == ir::BinaryOpKind::FAdd   ? "add"
                           : inst->op == ir::BinaryOpKind::FSub ? "sub"
                           : inst->op == ir::BinaryOpKind::FMul ? "mul"
                                                                : "div";
        loadFloat(inst->lhs, Reg::XMM0);
        std::string rhs = floatSource(inst->rhs, Reg::XMM1);
        line(name + sfx + "\t" + rhs + ", %xmm0");
        return defineResult(inst, info, Reg::XMM0);
    }

    //ucomis a, b sets the flags as an unsigned compare of b with a, unordered sets ZF, PF and CF
    std::string ucomi = "ucomi" + sfx + "\t";
    loadFloat(inst->lhs, Reg::XMM0);
    loadFloat(inst->rhs, Reg::XMM1);
    switch (inst->op) {
    case ir::BinaryOpKind::FCmpOGT: line(ucomi + "%xmm1, %xmm0"); line("seta\t%al"); break;
    case ir::BinaryOpKind::FCmpOGE: line(ucomi + "%xmm1, %xmm0"); line("setae\t%al"); break;
    case ir::BinaryOpKind::FCmpOLT: line(ucomi + "%xmm0, %xmm1"); line("seta\t%al"); break;
    case ir::BinaryOpKind::FCmpOLE: line(ucomi + "%xmm0, %xmm1"); line("setae\t%al"); break;
    case ir::BinaryOpKind::FCmpOEQ:
        line(ucomi + "%xmm1, %xmm0");
        line("sete\t%al");
        line("setnp\t%cl");
        line("andb\t%cl, %al");
        break;
    case ir::BinaryOpKind::FCmpUNE:
        line(ucomi + "%xmm1, %xmm0");
        line("setne\t%al");
        line("setp\t%cl");
        line("orb\t%cl, %al");
        break;
    default:
        return fail("unexpected floating compare");
    }
    line("movzbl\t%al, %eax");
    defineResult(inst, info, Reg::RAX);
}

//fneg flips the sign bit through an integer register
void FunctionEmitter::emitUnary(ir::UnaryInst *inst, const InstNextUse &info) {
    loadFloat(inst->operand, Reg::XMM0);
    if (isDoubleType(inst->getType())) {
        line("movq\t%xmm0, %rax");
        line("btcq\t$63, %rax");
        line("movq\t%rax, %xmm0");
    } else {
        line("movd\t%xmm0, %eax");
        line("xorl\t$0x80000000, %eax");
        line("movd\t%eax, %xmm0");
    }
    defineResult(inst, info, Reg::XMM0);
}

void FunctionEmitter::emitCast(ir::CastInst *inst, const InstNextUse &info) {
    const sema::Type *from = inst->fromType;
    const sema::Type *to = inst->toType;
    bool fromBool = ir::integerBits(from) == 1;

    switch (inst->op) {
    case ir::CastOpKind::SExt: {
        Width wf = widthOf(from);
        Width wt = widthOf(to);
        loadInt(inst->operand, Reg::RAX, wf);
        if (fromBool) {
            line("andl\t$1, %eax");
            line(wt == Width::Quad ? "negq\t%rax" : "negl\t%eax");
        } else if (wf == Width::Byte) {
            line(wt == Width::Quad ? "movsbq\t%al, %rax" : "movsbl\t%al, %eax");
        } else if (wf == Width::Long && wt == Width::Quad) {
            line("movslq\t%eax, %rax");
        }
        break;
    }
    case ir::CastOpKind::ZExt: {
        Width wf = widthOf(from);
        loadInt(inst->operand, Reg::RAX, wf);
        if (wf == Width::Byte) line("movzbl\t%al, %eax");  //clears the upper half too
        if (wf == Width::Long) line("movl\t%eax, %eax");
        break;
    }
    case ir::CastOpKind::Trunc:
        loadInt(inst->operand, Reg::RAX, widthOf(from));
        if (ir::integerBits(to) == 1) line("andl\t$1, %eax");
        break;
    case ir::CastOpKind::SIToFP:
    case ir::CastOpKind::UIToFP: {
        Width wf = widthOf(from);
        bool  isSigned = inst->op == ir::CastOpKind::SIToFP;
        loadInt(inst->operand, Reg::RAX, wf);
        Width convert = Width::Long;
        if (fromBool) {
            line("andl\t$1, %eax");
        } else if (wf == Width::Byte) {
            line(isSigned ? "movsbl\t%al, %eax" : "movzbl\t%al, %eax");
        } else if (wf == Width::Long && !isSigned) {
            line("movl\t%eax, %eax");  //an unsigned 32-bit value fits a signed 64-bit one
            convert = Width::Quad;
        } else if (wf == Width::Quad) {
            convert = Width::Quad;  //unsigned 64-bit values above 2^63 are not handled
        }
        line(std::string(isDoubleType(to) ? "cvtsi2sd" : "cvtsi2ss") + suffix(convert) + "\t" +
             regName(Reg::RAX, convert) + ", %xmm0");
        return defineResult(inst, info, Reg::XMM0);
    }
    case ir::CastOpKind::FPToSI: {
        loadFloat(inst->operand, Reg::XMM0);
        Width wt = widthOf(to) == Width::Quad ? Width::Quad : Width::Long;
        line(std::string(isDoubleType(from) ? "cvttsd2si" : "cvttss2si") + suffix(wt) + "\t%xmm0, " +
             regName(Reg::RAX, wt));
        break;
    }
    case ir::CastOpKind::FPExt:
        loadFloat(inst->operand, Reg::XMM0);
        line("cvtss2sd\t%xmm0, %xmm0");
        return defineResult(inst, info, Reg::XMM0);
    case ir::CastOpKind::FPTrunc:
        loadFloat(inst->operand, Reg::XMM0);
        line("cvtsd2ss\t%xmm0, %xmm0");
        return defineResult(inst, info, Reg::XMM0);
    case ir::CastOpKind::PtrToInt:
        loadInt(inst->operand, Reg::RAX, widthOf(to));
        break;
    case ir::CastOpKind::IntToPtr:
        loadInt(inst->operand, Reg::RAX, widthOf(from));
        if (widthOf(from) == Width::Long) line("movl\t%eax, %eax");
        if (widthOf(from) == Width::Byte) line("movzbl\t%al, %eax");
        break;
    }
    defineResult(inst, info, Reg::RAX);
}

void FunctionEmitter::emitSelect(ir::SelectInst *inst, const InstNextUse &info) {
    std::string cond = intSource(inst->condition, Width::Byte, Reg::RDX);
    if (isFloatType(inst->getType())) {
        std::string skip = localLabel();
        loadFloat(inst->ifFalse, Reg::XMM0);
        if (cond[0] == '$') {
            if (cond != "$0") loadFloat(inst->ifTrue, Reg::XMM0);
        } else {
            line("testb\t$1, " + cond);
            line("je\t" + skip);
            loadFloat(inst->ifTrue, Reg::XMM0);
            body.push_back(skip + ":");
        }
        return defineResult(inst, info, Reg::XMM0);
    }
    loadInt(inst->ifFalse, Reg::RAX, Width::Quad);
    loadInt(inst->ifTrue, Reg::RCX, Width::Quad);
    if (cond[0] == '$') {
        if (cond != "$0") line("movq\t%rcx, %rax");
    } else {
        line("testb\t$1, " + cond);
        line("cmovneq\t%rcx, %rax");
    }
    defineResult(inst, info, Reg::RAX);
}

//  calls

//system v: integers in rdi, rsi, rdx, rcx, r8, r9, floats in xmm0-xmm7, the rest on the stack
//right to left, %al holds the number of vector registers for a variadic callee
void FunctionEmitter::emitCall(ir::CallInst *inst, const InstNextUse &info) {
    const sema::FunctionType *type = inst->calleeType;
    std::vector<std::pair<const ir::Value *, Reg>> inRegs;
    std::vector<const ir::Value *>                 onStack;
    size_t intUsed = 0;
    size_t floatUsed = 0;
    for (size_t i = 0; i < inst->args.size(); i++) {
        const ir::Value  *arg = inst->args[i];
        const sema::Type *argType = i < type->params.size() ? type->params[i] : arg->getType();
        if (isFloatType(argType)) {
            if (floatUsed < kFloatArgCount) {
                inRegs.push_back(std::make_pair(arg, floatArgReg(floatUsed++)));
            } else {
                onStack.push_back(arg);
            }
        } else if (isIntegerLike(argType)) {
            if (intUsed < kIntArgCount) {
                inRegs.push_back(std::make_pair(arg, kIntArgRegs[intUsed++]));
            } else {
                onStack.push_back(arg);
            }
        } else {
            return fail("passing an aggregate by value is not supported");
        }
    }

    //stack arguments first: rax and xmm0 are free to use only before the argument registers are filled
    long long stackBytes = (long long)alignTo(onStack.size() * 8, 16);
    if (stackBytes != 0) line("subq\t" + imm(stackBytes) + ", %rsp");
    for (size_t i = 0; i < onStack.size(); i++) {
        std::string slot = Mem::at(Reg::RSP).plus((long long)i * 8).str();
        if (isFloatType(onStack[i]->getType())) {
            loadFloat(onStack[i], Reg::XMM0);
            line(std::string(isDoubleType(onStack[i]->getType()) ? "movsd" : "movss") + "\t%xmm0, " + slot);
        } else {
            loadInt(onStack[i], Reg::RAX, Width::Quad);
            line("movq\t%rax, " + slot);
        }
    }
    for (const std::pair<const ir::Value *, Reg> &entry : inRegs) {
        if (isXmm(entry.second)) {
            loadFloat(entry.first, entry.second);
            continue;
        }
        Width w = widthOf(entry.first->getType());
        loadInt(entry.first, entry.second, w);
        //chars are passed sign-extended and bools zero-extended to 32 bits, as clang does
        if (w == Width::Byte) {
            const char *ext = ir::integerBits(entry.first->getType()) == 1 ? "movzbl\t" : "movsbl\t";
            line(ext + regName(entry.second, Width::Byte) + ", " + regName(entry.second, Width::Long));
        }
    }

    bool direct = inst->callee->getKind() == ir::ValueKind::Function;
    if (!direct) loadInt(inst->callee, Reg::R11, Width::Quad);
    if (type->isVariadic) line("movl\t" + imm((long long)floatUsed) + ", %eax");

    //what the call clobbers: values still needed leave the caller-saved registers
    finishOperands(info);
    ra.spillCallerSaved(nextUse);

    if (direct) {
        const ir::Function *callee = static_cast<const ir::Function *>(inst->callee);
        line("callq\t" + symbolName(callee) + (callee->isDefinition() ? "" : "@PLT"));
    } else {
        line("callq\t*%r11");
    }
    if (stackBytes != 0) line("addq\t" + imm(stackBytes) + ", %rsp");

    if (inst->getType()->is(sema::TypeKind::Void)) return;
    InstNextUse resultOnly;
    resultOnly.result = info.result;
    defineResult(inst, resultOnly, isFloatType(inst->getType()) ? Reg::XMM0 : Reg::RAX);
}

//  phis

void FunctionEmitter::emitPhi(ir::PhiInst *phi, const InstNextUse &info) {
    const sema::Type *type = phi->getType();
    Mem slot = Mem::frame(frame.phiSlot(phi));
    if (isFloatType(type)) {
        line(std::string(isDoubleType(type) ? "movsd\t" : "movss\t") + slot.str() + ", %xmm0");
        return defineResult(phi, info, Reg::XMM0);
    }
    if (!isIntegerLike(type)) return fail("a phi of an aggregate value is not supported");
    switch (widthOf(type)) {
    case Width::Byte: line("movzbl\t" + slot.str() + ", %eax"); break;
    case Width::Long: line("movl\t" + slot.str() + ", %eax"); break;
    case Width::Quad: line("movq\t" + slot.str() + ", %rax"); break;
    }
    defineResult(phi, info, Reg::RAX);
}

bool FunctionEmitter::hasPhiFrom(const ir::BasicBlock *block, const ir::BasicBlock *succ) const {
    for (const std::unique_ptr<ir::Instruction> &inst : succ->instructions) {
        if (inst->getOpcode() != ir::Opcode::Phi) return false;
        if (static_cast<const ir::PhiInst *>(inst.get())->getIncomingValueFor(block)) return true;
    }
    return false;
}

ir::BasicBlock *FunctionEmitter::deferredEdge(ir::BasicBlock *block, ir::BranchInst *br) {
    if (!br->isConditional() || br->target == br->falseTarget) return nullptr;
    if (!hasPhiFrom(block, br->target) || !hasPhiFrom(block, br->falseTarget)) return nullptr;
    const passes::Dominators &doms = am.getDominators(fn);
    bool trueBack = doms.dominates(br->target, block);
    bool falseBack = doms.dominates(br->falseTarget, block);
    if (trueBack == falseBack) return nullptr;
    return trueBack ? br->falseTarget : br->target;
}

void FunctionEmitter::branchWithCopies(const std::string &condCode, ir::BasicBlock *block, ir::BasicBlock *ifTrue,
                                       ir::BasicBlock *ifFalse, ir::BasicBlock *deferred, const ir::BasicBlock *next) {
    if (deferred == nullptr) return branchOn(condCode, ifTrue, ifFalse, next);
    if (deferred == ifFalse) {
        line("j" + condCode + "\t" + label(ifTrue));
        storePhiCopies(block, ifFalse);
        return jumpUnless(ifFalse, next);
    }
    line("j" + invert(condCode) + "\t" + label(ifFalse));
    storePhiCopies(block, ifTrue);
    jumpUnless(ifTrue, next);
}

void FunctionEmitter::storePhiCopies(ir::BasicBlock *block, const ir::BasicBlock *succ) {
    for (const std::unique_ptr<ir::Instruction> &inst : succ->instructions) {
        if (inst->getOpcode() != ir::Opcode::Phi) break;
        ir::PhiInst *phi = static_cast<ir::PhiInst *>(inst.get());
        const ir::Value *value = phi->getIncomingValueFor(block);
        if (value == nullptr) continue;
        const sema::Type *type = phi->getType();
        Mem slot = Mem::frame(frame.phiSlot(phi));
        if (isFloatType(type)) {
            loadFloat(value, Reg::XMM0);
            line(std::string(isDoubleType(type) ? "movsd\t" : "movss\t") + "%xmm0, " + slot.str());
            continue;
        }
        Width w = widthOf(type);
        const ir::Constant *c = asConstant(value);
        Reg held;
        if (ra.registerOf(value, held)) {
            line(std::string("mov") + suffix(w) + "\t" + regName(held, w) + ", " + slot.str());
        } else if (c && c->constKind == ir::ConstantKind::Int && fitsInt32(c->intValue)) {
            line(std::string("mov") + suffix(w) + "\t" + imm(c->intValue) + ", " + slot.str());
        } else if (c && c->constKind == ir::ConstantKind::Null) {
            line("movq\t$0, " + slot.str());
        } else {
            loadInt(value, Reg::RAX, w);
            line(std::string("mov") + suffix(w) + "\t" + regName(Reg::RAX, w) + ", " + slot.str());
        }
    }
}

//  terminators

void FunctionEmitter::jumpUnless(const ir::BasicBlock *target, const ir::BasicBlock *next) {
    if (target != next) line("jmp\t" + label(target));
}

void FunctionEmitter::branchOn(const std::string &condCode, ir::BasicBlock *ifTrue, ir::BasicBlock *ifFalse,
                               const ir::BasicBlock *next) {
    if (ifTrue == next) {
        line("j" + invert(condCode) + "\t" + label(ifFalse));
        return;
    }
    line("j" + condCode + "\t" + label(ifTrue));
    jumpUnless(ifFalse, next);
}

void FunctionEmitter::emitTerminator(ir::Instruction *inst, const InstNextUse &info, const passes::ValueSet &liveOut,
                                     ir::BasicBlock *next) {
    (void)info;
    ir::BasicBlock *block = inst->getParent();
    //the successors' phi copies come first, while the flags do not matter yet (a repeated edge
    //carries the same values). only a loop exit may wait until after the jump, see deferredEdge
    ir::BasicBlock *deferred = nullptr;
    if (inst->getOpcode() == ir::Opcode::Br) deferred = deferredEdge(block, static_cast<ir::BranchInst *>(inst));
    std::set<const ir::BasicBlock *> done;
    if (deferred) done.insert(deferred);
    for (ir::BasicBlock *succ : inst->getSuccessors()) {
        if (done.insert(succ).second) storePhiCopies(block, succ);
    }
    //end of block: values live on exit go back to memory. movs leave the flags alone
    ra.flush(liveOut);

    switch (inst->getOpcode()) {
    case ir::Opcode::Ret: {
        ir::ReturnInst *ret = static_cast<ir::ReturnInst *>(inst);
        if (ret->returnValue) {
            const sema::Type *type = ret->returnType;
            if (isFloatType(type)) {
                loadFloat(ret->returnValue, Reg::XMM0);
            } else if (isIntegerLike(type)) {
                loadInt(ret->returnValue, Reg::RAX, widthOf(type));
            } else {
                return fail("returning an aggregate value is not supported");
            }
        }
        if (next != nullptr) line("jmp\t.Lret" + std::to_string(id));
        return;
    }
    case ir::Opcode::Br: {
        ir::BranchInst *br = static_cast<ir::BranchInst *>(inst);
        if (!br->isConditional()) return jumpUnless(br->target, next);

        const ir::Instruction *condInst = br->condition->getKind() == ir::ValueKind::Instruction
                                              ? static_cast<const ir::Instruction *>(br->condition)
                                              : nullptr;
        if (condInst && fused.count(condInst)) {
            ir::BinaryInst *cmp = static_cast<ir::BinaryInst *>(br->condition);
            emitIntCompare(cmp);
            return branchWithCopies(conditionCode(cmp->op), block, br->target, br->falseTarget, deferred, next);
        }
        const ir::Constant *c = asConstant(br->condition);
        if (c) {
            ir::BasicBlock *taken = c->intValue ? br->target : br->falseTarget;
            if (taken == deferred) storePhiCopies(block, taken);
            return jumpUnless(taken, next);
        }
        line("testb\t$1, " + intSource(br->condition, Width::Byte, Reg::RAX));
        return branchWithCopies("ne", block, br->target, br->falseTarget, deferred, next);
    }
    case ir::Opcode::Switch: {
        ir::SwitchInst *sw = static_cast<ir::SwitchInst *>(inst);
        Width w = widthOf(sw->condition->getType());
        loadInt(sw->condition, Reg::RAX, w);
        for (const std::pair<long long, ir::BasicBlock *> &c : sw->cases) {
            line(std::string("cmp") + suffix(w) + "\t" + imm(c.first) + ", " + regName(Reg::RAX, w));
            line("je\t" + label(c.second));
        }
        return jumpUnless(sw->defaultTarget, next);
    }
    case ir::Opcode::Unreachable:
        line("ud2");
        return;
    default:
        return fail("unknown terminator");
    }
}

} // namespace codegen
