#include "codegen/isel.h"

#include "ir/ir_types.h"

namespace codegen {

namespace {

const Reg kIntArgRegs[] = {Reg::RDI, Reg::RSI, Reg::RDX, Reg::RCX, Reg::R8, Reg::R9};
const size_t kIntArgCount = 6;
const size_t kFloatArgCount = 8;

//what a call overwrites (system v): every general register but rbx, rbp, rsp, r12-r15, and every xmm
const std::vector<Reg> kCallClobbers = {
    Reg::RAX, Reg::RCX, Reg::RDX, Reg::RSI, Reg::RDI, Reg::R8, Reg::R9, Reg::R10, Reg::R11,
    Reg::XMM0, Reg::XMM1, Reg::XMM2, Reg::XMM3, Reg::XMM4, Reg::XMM5, Reg::XMM6, Reg::XMM7,
    Reg::XMM8, Reg::XMM9, Reg::XMM10, Reg::XMM11, Reg::XMM12, Reg::XMM13, Reg::XMM14, Reg::XMM15};

Reg floatArgReg(size_t index) { return (Reg)((int)Reg::XMM0 + (int)index); }

std::string imm(long long value) { return "$" + std::to_string(value); }

//integer ops on bytes are done 32 bits wide, the low byte comes out the same
Width aluWidth(Width width) { return width == Width::Byte ? Width::Long : width; }

//the width a register is loaded with: bytes are loaded 32 bits wide
Width loadWidth(Width width) { return width == Width::Quad ? Width::Quad : Width::Long; }

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

std::string movFor(const sema::Type *type) { return isDoubleType(type) ? "movsd" : "movss"; }

} // namespace

FunctionEmitter::FunctionEmitter(ir::Function &fn, passes::AnalysisManager &am, TargetLayout &layout,
                                 ConstantPool &pool, int id, int optLevel)
    : fn(fn), am(am), layout(layout), pool(pool), id(id), global(optLevel >= 3), ra(frame, body) {}

void FunctionEmitter::fail(const std::string &message) {
    if (errorText.empty()) errorText = "@" + fn.getName() + ": " + message;
}

std::string FunctionEmitter::label(const ir::BasicBlock *block) const {
    return ".LBB" + std::to_string(id) + "_" + block->getName();
}

std::string FunctionEmitter::localLabel() { return ".Ltmp" + std::to_string(id) + "_" + std::to_string(localCount++); }

void FunctionEmitter::op(const std::string &mnemonic, std::vector<MOperand> operands, std::vector<Reg> implicitUses,
                         std::vector<Reg> implicitDefs) {
    MInst inst;
    inst.op = mnemonic;
    inst.operands = std::move(operands);
    inst.implicitUses = std::move(implicitUses);
    inst.implicitDefs = std::move(implicitDefs);
    body.push_back(inst);
}

void FunctionEmitter::placeLabel(const std::string &name) {
    MInst inst;
    inst.kind = MInst::Label;
    inst.op = name;
    body.push_back(inst);
}

bool FunctionEmitter::isRegisterValue(const ir::Value *value) const {
    if (value->getKind() == ir::ValueKind::Argument) return true;
    if (value->getKind() != ir::ValueKind::Instruction) return false;
    return !frame.isAlloca(value);
}

Reg FunctionEmitter::newVreg(bool isFloat) { return isFloat ? virtualFloat(floatVregs++) : virtualInt(intVregs++); }

Reg FunctionEmitter::vregFor(const ir::Value *value) {
    std::map<const ir::Value *, Reg>::iterator it = vregs.find(value);
    if (it != vregs.end()) return it->second;
    Reg reg = newVreg(isFloatType(value->getType()));
    vregs[value] = reg;
    return reg;
}

bool FunctionEmitter::held(const ir::Value *value, Reg &reg) {
    if (!global) return ra.registerOf(value, reg);
    if (!isRegisterValue(value)) return false;
    reg = vregFor(value);
    return true;
}

//  function structure

bool FunctionEmitter::emit(AsmLines &out) {
    layoutFrame();
    MCode entry;
    storeIncomingArguments(entry);

    const std::vector<ir::BasicBlock *> &blocks = fn.getBlocks();
    for (size_t i = 0; i < blocks.size(); i++) {
        emitBlock(blocks[i], i + 1 < blocks.size() ? blocks[i + 1] : nullptr);
    }
    if (!errorText.empty()) return false;

    std::set<Reg> calleeSaved;
    if (global) {
        //-O3: the whole function, entry included, is coloured at once
        MCode all = entry;
        all.insert(all.end(), body.begin(), body.end());
        std::map<std::string, unsigned> depths;
        const passes::LoopInfo &loops = am.getLoops(fn);
        for (ir::BasicBlock *block : blocks) depths[label(block)] = loops.depth(block);
        std::string problem;
        if (!colorMachineCode(all, frame, depths, stats, problem)) {
            fail(problem);
            return false;
        }
        entry.clear();
        body.swap(all);
        calleeSaved = stats.calleeSavedUsed;
    } else {
        calleeSaved = ra.usedCalleeSaved();
    }

    //the frame is complete only now: homes and spill slots were made on demand, then the save slots
    for (Reg reg : calleeSaved) frame.saveSlot(reg);
    uint64_t size = frame.size();
    std::string name = symbolName(&fn);

    out.push_back("\t.text");
    if (fn.linkage == ir::Linkage::External) out.push_back("\t.globl\t" + name);
    out.push_back("\t.p2align\t4");
    out.push_back("\t.type\t" + name + ",@function");
    out.push_back(name + ":");
    if (global) {
        out.push_back("\t# graph colouring: " + std::to_string(stats.virtuals) + " virtual registers, " +
                      std::to_string(stats.coalesced) + " copies coalesced, " + std::to_string(stats.spilled) +
                      " spilled, " + std::to_string(stats.rounds) + " rounds");
    }
    out.push_back("\tpushq\t%rbp");
    out.push_back("\tmovq\t%rsp, %rbp");
    if (size != 0) out.push_back("\tsubq\t" + imm((long long)size) + ", %rsp");
    for (Reg reg : calleeSaved) {
        out.push_back("\tmovq\t" + regName(reg) + ", " + Mem::frame(frame.saveSlot(reg)).str());
    }
    printCode(entry, out);
    printCode(body, out);
    out.push_back(".Lret" + std::to_string(id) + ":");
    for (Reg reg : calleeSaved) {
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

//arguments arrive in registers (or above the return address): -O0 to -O2 store them to their homes,
//-O3 copies them into their virtual registers
void FunctionEmitter::storeIncomingArguments(MCode &code) {
    size_t    intUsed = 0;
    size_t    floatUsed = 0;
    long long stackOffset = 16;
    MCode saved;
    saved.swap(body);
    for (const std::unique_ptr<ir::Argument> &arg : fn.args) {
        const sema::Type *type = arg->getType();
        if (isFloatType(type)) {
            if (floatUsed < kFloatArgCount) {
                Reg from = floatArgReg(floatUsed++);
                if (global) {
                    op("movaps", { use(from), def(vregFor(arg.get())) });
                } else {
                    op(movFor(type), { use(from), memOp(Mem::frame(frame.home(arg.get()))) });
                }
            } else {
                if (global) {
                    op(movFor(type), { memOp(Mem::frame(stackOffset)), def(vregFor(arg.get())) });
                } else {
                    frame.setHome(arg.get(), stackOffset);
                }
                stackOffset += 8;
            }
        } else if (isIntegerLike(type)) {
            if (intUsed < kIntArgCount) {
                Reg from = kIntArgRegs[intUsed++];
                if (global) {
                    op("movq", { use(from), def(vregFor(arg.get())) });
                } else {
                    op("movq", { use(from), memOp(Mem::frame(frame.home(arg.get()))) });
                }
            } else {
                if (global) {
                    op("movq", { memOp(Mem::frame(stackOffset)), def(vregFor(arg.get())) });
                } else {
                    frame.setHome(arg.get(), stackOffset);
                }
                stackOffset += 8;
            }
        } else {
            fail("argument %" + arg->getName() + " has an aggregate type, which is not supported");
        }
    }
    code.swap(body);
    body.swap(saved);
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

    placeLabel(label(block));
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
    const Width w = loadWidth(width);
    switch (value->getKind()) {
    case ir::ValueKind::Constant: {
        const ir::Constant *c = asConstant(value);
        if (c->constKind == ir::ConstantKind::Address) return loadInt(c->target, dst, width);
        long long v = c->constKind == ir::ConstantKind::Int ? c->intValue : 0;
        if (v == 0) {
            op("xorl", { def(dst, Width::Long), def(dst, Width::Long) });  //only writes: the old value is not read
        } else if (width != Width::Quad) {
            op("movl", { immOp(v), def(dst, Width::Long) });
        } else if (fitsInt32(v)) {
            op("movq", { immOp(v), def(dst) });
        } else {
            op("movabsq", { immOp(v), def(dst) });
        }
        return;
    }
    case ir::ValueKind::GlobalVariable:
        op("leaq", { memOp(Mem::global(symbolName(value))), def(dst) });
        return;
    case ir::ValueKind::Function: {
        const ir::Function *callee = static_cast<const ir::Function *>(value);
        if (callee->isDefinition()) {
            op("leaq", { memOp(Mem::global(symbolName(value))), def(dst) });
        } else {
            op("movq", { textOp(symbolName(value) + "@GOTPCREL(%rip)"), def(dst) });
        }
        return;
    }
    default:
        break;
    }
    if (frame.isAlloca(value)) {
        op("leaq", { memOp(Mem::frame(frame.allocaOffset(value))), def(dst) });
        return;
    }
    Reg reg;
    if (held(value, reg)) {
        if (reg == dst) return;
        op(w == Width::Quad ? "movq" : "movl", { use(reg, w), def(dst, w) });
        return;
    }
    MOperand home = memOp(Mem::frame(frame.home(value)));
    switch (width) {
    case Width::Byte: op("movzbl", { home, def(dst, Width::Long) }); break;
    case Width::Long: op("movl", { home, def(dst, Width::Long) }); break;
    case Width::Quad: op("movq", { home, def(dst) }); break;
    }
}

//an operand an alu instruction can read directly: $imm, a register or a home slot
MOperand FunctionEmitter::intSource(const ir::Value *value, Width width, Reg scratch) {
    const ir::Constant *c = asConstant(value);
    if (c && c->constKind == ir::ConstantKind::Int && fitsInt32(c->intValue)) return immOp(c->intValue);
    if (c && c->constKind == ir::ConstantKind::Null) return immOp(0);
    if (isRegisterValue(value)) {
        Reg reg;
        if (held(value, reg)) return use(reg, width);
        return memOp(Mem::frame(frame.home(value)));
    }
    loadInt(value, scratch, width);
    return use(scratch, width);
}

void FunctionEmitter::loadFloat(const ir::Value *value, Reg dst) {
    const std::string mov = movFor(value->getType());
    const ir::Constant *c = asConstant(value);
    if (c) {
        if (c->constKind != ir::ConstantKind::Float) return fail("unexpected floating constant");
        op(mov, { memOp(Mem::global(pool.label(c->floatValue, isDoubleType(value->getType())))), def(dst) });
        return;
    }
    Reg reg;
    if (held(value, reg)) {
        if (reg != dst) op("movaps", { use(reg), def(dst) });
        return;
    }
    op(mov, { memOp(Mem::frame(frame.home(value))), def(dst) });
}

MOperand FunctionEmitter::floatSource(const ir::Value *value, Reg scratch) {
    const ir::Constant *c = asConstant(value);
    if (c && c->constKind == ir::ConstantKind::Float) {
        return memOp(Mem::global(pool.label(c->floatValue, isDoubleType(value->getType()))));
    }
    if (isRegisterValue(value)) {
        Reg reg;
        if (held(value, reg)) return use(reg);
        return memOp(Mem::frame(frame.home(value)));
    }
    loadFloat(value, scratch);
    return use(scratch);
}

//the memory a pointer points at: allocas and globals directly, anything else through a register
Mem FunctionEmitter::addressOf(const ir::Value *pointer, Reg scratch) {
    if (frame.isAlloca(pointer)) return Mem::frame(frame.allocaOffset(pointer));
    if (pointer->getKind() == ir::ValueKind::GlobalVariable) return Mem::global(symbolName(pointer));
    Reg reg;
    if (held(pointer, reg)) return Mem::at(reg);
    loadInt(pointer, scratch, Width::Quad);
    return Mem::at(scratch);
}

void FunctionEmitter::finishOperands(const InstNextUse &info) {
    if (global) return;  //virtual registers are never released by hand
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
    Reg reg;
    if (global) {
        reg = vregFor(inst);
    } else {
        nextUse[inst] = info.result;
        reg = ra.allocate(inst, isXmm(src), nextUse);
    }
    op(isXmm(src) ? "movaps" : "movq", { use(src), def(reg) });
}

//  memory

void FunctionEmitter::emitLoad(ir::LoadInst *inst, const InstNextUse &info) {
    const sema::Type *type = inst->getAccessType();
    Mem mem = addressOf(inst->address, Reg::R11);
    if (isFloatType(type)) {
        op(movFor(type), { memOp(mem), def(Reg::XMM0) });
        return defineResult(inst, info, Reg::XMM0);
    }
    if (!isIntegerLike(type)) {
        //a record or array value: its bytes are copied to a frame slot of their own, as the load's snapshot
        uint64_t size = layout.sizeOf(type);
        copyBytes(mem, Mem::frame(frame.aggregateSlot(inst, size, layout.alignOf(type))), size);
        return finishOperands(info);
    }
    switch (widthOf(type)) {
    case Width::Byte: op("movzbl", { memOp(mem), def(Reg::RAX, Width::Long) }); break;
    case Width::Long: op("movl", { memOp(mem), def(Reg::RAX, Width::Long) }); break;
    case Width::Quad: op("movq", { memOp(mem), def(Reg::RAX) }); break;
    }
    defineResult(inst, info, Reg::RAX);
}

void FunctionEmitter::emitStore(ir::StoreInst *inst, const InstNextUse &info) {
    const sema::Type *type = inst->getAccessType();
    Mem mem = addressOf(inst->address, Reg::R11);
    const ir::Constant *c = asConstant(inst->value);

    if (isFloatType(type)) {
        loadFloat(inst->value, Reg::XMM0);
        op(movFor(type), { use(Reg::XMM0), memOp(mem) });
    } else if (isIntegerLike(type)) {
        Width w = widthOf(type);
        Reg   reg;
        if (held(inst->value, reg)) {
            op(std::string("mov") + suffix(w), { use(reg, w), memOp(mem) });
        } else if (c && c->constKind == ir::ConstantKind::Int && fitsInt32(c->intValue)) {
            op(std::string("mov") + suffix(w), { immOp(c->intValue), memOp(mem) });
        } else if (c && c->constKind == ir::ConstantKind::Null) {
            op("movq", { immOp(0), memOp(mem) });
        } else {
            loadInt(inst->value, Reg::RAX, w);
            op(std::string("mov") + suffix(w), { use(Reg::RAX, w), memOp(mem) });
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
            op(std::string("mov") + suffix(widthOf(type)), { immOp(constant->intValue), memOp(mem) });
        } else {
            op("movabsq", { immOp(constant->intValue), def(Reg::RAX) });
            op("movq", { use(Reg::RAX), memOp(mem) });
        }
        return;
    case ir::ConstantKind::Float: {
        long long bits = (long long)floatBits(constant->floatValue, isDoubleType(type));
        if (isDoubleType(type)) {
            op("movabsq", { immOp(bits), def(Reg::RAX) });
            op("movq", { use(Reg::RAX), memOp(mem) });
        } else {
            op("movl", { immOp((long long)(int32_t)(uint32_t)bits), memOp(mem) });
        }
        return;
    }
    case ir::ConstantKind::Null:
        op("movq", { immOp(0), memOp(mem) });
        return;
    case ir::ConstantKind::Address:
        loadInt(constant->target, Reg::RAX, Width::Quad);
        op("movq", { use(Reg::RAX), memOp(mem) });
        return;
    case ir::ConstantKind::ZeroInit: {
        uint64_t size = layout.sizeOf(type);
        uint64_t done = 0;
        for (; done + 8 <= size; done += 8) op("movq", { immOp(0), memOp(mem.plus(done)) });
        for (; done + 4 <= size; done += 4) op("movl", { immOp(0), memOp(mem.plus(done)) });
        for (; done < size; done++) op("movb", { immOp(0), memOp(mem.plus(done)) });
        return;
    }
    case ir::ConstantKind::String: {
        const std::string &bytes = constant->bytes;
        size_t done = 0;
        for (; done + 4 <= bytes.size(); done += 4) {
            uint32_t word = 0;
            for (int k = 3; k >= 0; k--) word = (word << 8) | (unsigned char)bytes[done + k];
            op("movl", { immOp((long long)(int32_t)word), memOp(mem.plus(done)) });
        }
        for (; done < bytes.size(); done++) {
            op("movb", { immOp((long long)(signed char)bytes[done]), memOp(mem.plus(done)) });
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
        op("movq", { memOp(from.plus(done)), def(Reg::RAX) });
        op("movq", { use(Reg::RAX), memOp(to.plus(done)) });
    }
    for (; done + 4 <= size; done += 4) {
        op("movl", { memOp(from.plus(done)), def(Reg::RAX, Width::Long) });
        op("movl", { use(Reg::RAX, Width::Long), memOp(to.plus(done)) });
    }
    for (; done < size; done++) {
        op("movb", { memOp(from.plus(done)), def(Reg::RAX, Width::Byte) });
        op("movb", { use(Reg::RAX, Width::Byte), memOp(to.plus(done)) });
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
            if (w == Width::Long) op("movslq", { use(Reg::RCX, Width::Long), def(Reg::RCX) });
            if (w == Width::Byte) op("movsbq", { use(Reg::RCX, Width::Byte), def(Reg::RCX) });
            if (step != 1) op("imulq", { immOp(step), useDef(Reg::RCX) });
            op("addq", { use(Reg::RCX), useDef(Reg::RAX) });
        }
        current = next;
    }
    if (disp != 0) op("leaq", { memOp(Mem::at(Reg::RAX).plus(disp)), def(Reg::RAX) });
    defineResult(inst, info, Reg::RAX);
}

//  arithmetic

void FunctionEmitter::emitIntCompare(ir::BinaryInst *inst) {
    Width w = widthOf(inst->operandType);
    loadInt(inst->lhs, Reg::RAX, w);
    MOperand rhs = intSource(inst->rhs, w, Reg::RCX);
    op(std::string("cmp") + suffix(w), { rhs, use(Reg::RAX, w) });
}

void FunctionEmitter::emitBinary(ir::BinaryInst *inst, const InstNextUse &info) {
    Width w = widthOf(inst->operandType);
    Width aw = aluWidth(w);
    Reg   result = Reg::RAX;

    if (isIntCompare(inst->op)) {
        emitIntCompare(inst);
        op(std::string("set") + conditionCode(inst->op), { def(Reg::RAX, Width::Byte) });
        op("movzbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });
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
        MOperand rhs = intSource(inst->rhs, aw, Reg::RCX);
        op(std::string(name) + suffix(aw), { rhs, useDef(Reg::RAX, aw) });
        break;
    }
    case ir::BinaryOpKind::SDiv:
    case ir::BinaryOpKind::SRem:
        loadInt(inst->lhs, Reg::RAX, aw);
        loadInt(inst->rhs, Reg::RCX, aw);
        if (w == Width::Byte) {
            op("movsbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });
            op("movsbl", { use(Reg::RCX, Width::Byte), def(Reg::RCX, Width::Long) });
        }
        //cltd/cqto sign-extends rax into rdx; idiv divides rdx:rax, quotient to rax, remainder to rdx
        op(aw == Width::Quad ? "cqto" : "cltd", {}, { Reg::RAX }, { Reg::RDX });
        op(std::string("idiv") + suffix(aw), { use(Reg::RCX, aw) }, { Reg::RAX, Reg::RDX }, { Reg::RAX, Reg::RDX });
        if (inst->op == ir::BinaryOpKind::SRem) result = Reg::RDX;
        break;
    case ir::BinaryOpKind::Shl:
    case ir::BinaryOpKind::AShr:
        loadInt(inst->lhs, Reg::RAX, aw);
        loadInt(inst->rhs, Reg::RCX, aw);
        if (w == Width::Byte && inst->op == ir::BinaryOpKind::AShr) {
            op("movsbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });
        }
        op(std::string(inst->op == ir::BinaryOpKind::Shl ? "shl" : "sar") + suffix(aw),
           { use(Reg::RCX, Width::Byte), useDef(Reg::RAX, aw) });
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
        MOperand rhs = floatSource(inst->rhs, Reg::XMM1);
        op(name + sfx, { rhs, useDef(Reg::XMM0) });
        return defineResult(inst, info, Reg::XMM0);
    }

    //ucomis a, b sets the flags as an unsigned compare of b with a, unordered sets ZF, PF and CF
    std::string ucomi = "ucomi" + sfx;
    loadFloat(inst->lhs, Reg::XMM0);
    loadFloat(inst->rhs, Reg::XMM1);
    MOperand al = def(Reg::RAX, Width::Byte);
    switch (inst->op) {
    case ir::BinaryOpKind::FCmpOGT: op(ucomi, { use(Reg::XMM1), use(Reg::XMM0) }); op("seta", { al }); break;
    case ir::BinaryOpKind::FCmpOGE: op(ucomi, { use(Reg::XMM1), use(Reg::XMM0) }); op("setae", { al }); break;
    case ir::BinaryOpKind::FCmpOLT: op(ucomi, { use(Reg::XMM0), use(Reg::XMM1) }); op("seta", { al }); break;
    case ir::BinaryOpKind::FCmpOLE: op(ucomi, { use(Reg::XMM0), use(Reg::XMM1) }); op("setae", { al }); break;
    case ir::BinaryOpKind::FCmpOEQ:
        op(ucomi, { use(Reg::XMM1), use(Reg::XMM0) });
        op("sete", { al });
        op("setnp", { def(Reg::RCX, Width::Byte) });
        op("andb", { use(Reg::RCX, Width::Byte), useDef(Reg::RAX, Width::Byte) });
        break;
    case ir::BinaryOpKind::FCmpUNE:
        op(ucomi, { use(Reg::XMM1), use(Reg::XMM0) });
        op("setne", { al });
        op("setp", { def(Reg::RCX, Width::Byte) });
        op("orb", { use(Reg::RCX, Width::Byte), useDef(Reg::RAX, Width::Byte) });
        break;
    default:
        return fail("unexpected floating compare");
    }
    op("movzbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });
    defineResult(inst, info, Reg::RAX);
}

//fneg flips the sign bit through an integer register
void FunctionEmitter::emitUnary(ir::UnaryInst *inst, const InstNextUse &info) {
    loadFloat(inst->operand, Reg::XMM0);
    if (isDoubleType(inst->getType())) {
        op("movq", { use(Reg::XMM0), def(Reg::RAX) });
        op("btcq", { immOp(63), useDef(Reg::RAX) });
        op("movq", { use(Reg::RAX), def(Reg::XMM0) });
    } else {
        op("movd", { use(Reg::XMM0), def(Reg::RAX, Width::Long) });
        op("xorl", { textOp("$0x80000000"), useDef(Reg::RAX, Width::Long) });
        op("movd", { use(Reg::RAX, Width::Long), def(Reg::XMM0) });
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
            op("andl", { immOp(1), useDef(Reg::RAX, Width::Long) });
            if (wt == Width::Quad) {
                op("negq", { useDef(Reg::RAX) });
            } else {
                op("negl", { useDef(Reg::RAX, Width::Long) });
            }
        } else if (wf == Width::Byte) {
            if (wt == Width::Quad) {
                op("movsbq", { use(Reg::RAX, Width::Byte), def(Reg::RAX) });
            } else {
                op("movsbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });
            }
        } else if (wf == Width::Long && wt == Width::Quad) {
            op("movslq", { use(Reg::RAX, Width::Long), def(Reg::RAX) });
        }
        break;
    }
    case ir::CastOpKind::ZExt: {
        Width wf = widthOf(from);
        loadInt(inst->operand, Reg::RAX, wf);
        if (wf == Width::Byte) op("movzbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });  //clears the upper half too
        if (wf == Width::Long) op("movl", { use(Reg::RAX, Width::Long), def(Reg::RAX, Width::Long) });
        break;
    }
    case ir::CastOpKind::Trunc:
        loadInt(inst->operand, Reg::RAX, widthOf(from));
        if (ir::integerBits(to) == 1) op("andl", { immOp(1), useDef(Reg::RAX, Width::Long) });
        break;
    case ir::CastOpKind::SIToFP:
    case ir::CastOpKind::UIToFP: {
        Width wf = widthOf(from);
        bool  isSigned = inst->op == ir::CastOpKind::SIToFP;
        loadInt(inst->operand, Reg::RAX, wf);
        Width convert = Width::Long;
        if (fromBool) {
            op("andl", { immOp(1), useDef(Reg::RAX, Width::Long) });
        } else if (wf == Width::Byte) {
            op(isSigned ? "movsbl" : "movzbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });
        } else if (wf == Width::Long && !isSigned) {
            //an unsigned 32-bit value fits a signed 64-bit one
            op("movl", { use(Reg::RAX, Width::Long), def(Reg::RAX, Width::Long) });
            convert = Width::Quad;
        } else if (wf == Width::Quad) {
            convert = Width::Quad;  //unsigned 64-bit values above 2^63 are not handled
        }
        op(std::string(isDoubleType(to) ? "cvtsi2sd" : "cvtsi2ss") + suffix(convert),
           { use(Reg::RAX, convert), def(Reg::XMM0) });
        return defineResult(inst, info, Reg::XMM0);
    }
    case ir::CastOpKind::FPToSI: {
        loadFloat(inst->operand, Reg::XMM0);
        Width wt = widthOf(to) == Width::Quad ? Width::Quad : Width::Long;
        op(std::string(isDoubleType(from) ? "cvttsd2si" : "cvttss2si") + suffix(wt), { use(Reg::XMM0), def(Reg::RAX, wt) });
        break;
    }
    case ir::CastOpKind::FPExt:
        loadFloat(inst->operand, Reg::XMM0);
        op("cvtss2sd", { use(Reg::XMM0), def(Reg::XMM0) });
        return defineResult(inst, info, Reg::XMM0);
    case ir::CastOpKind::FPTrunc:
        loadFloat(inst->operand, Reg::XMM0);
        op("cvtsd2ss", { use(Reg::XMM0), def(Reg::XMM0) });
        return defineResult(inst, info, Reg::XMM0);
    case ir::CastOpKind::PtrToInt:
        loadInt(inst->operand, Reg::RAX, widthOf(to));
        break;
    case ir::CastOpKind::IntToPtr:
        loadInt(inst->operand, Reg::RAX, widthOf(from));
        if (widthOf(from) == Width::Long) op("movl", { use(Reg::RAX, Width::Long), def(Reg::RAX, Width::Long) });
        if (widthOf(from) == Width::Byte) op("movzbl", { use(Reg::RAX, Width::Byte), def(Reg::RAX, Width::Long) });
        break;
    }
    defineResult(inst, info, Reg::RAX);
}

void FunctionEmitter::emitSelect(ir::SelectInst *inst, const InstNextUse &info) {
    MOperand cond = intSource(inst->condition, Width::Byte, Reg::RDX);
    bool constant = cond.kind == MOperand::Immediate;
    if (isFloatType(inst->getType())) {
        std::string skip = localLabel();
        loadFloat(inst->ifFalse, Reg::XMM0);
        if (constant) {
            if (cond.value != 0) loadFloat(inst->ifTrue, Reg::XMM0);
        } else {
            op("testb", { immOp(1), cond });
            op("je", { textOp(skip) });
            loadFloat(inst->ifTrue, Reg::XMM0);
            placeLabel(skip);
        }
        return defineResult(inst, info, Reg::XMM0);
    }
    loadInt(inst->ifFalse, Reg::RAX, Width::Quad);
    loadInt(inst->ifTrue, Reg::RCX, Width::Quad);
    if (constant) {
        if (cond.value != 0) op("movq", { use(Reg::RCX), def(Reg::RAX) });
    } else {
        op("testb", { immOp(1), cond });
        op("cmovneq", { use(Reg::RCX), useDef(Reg::RAX) });  //keeps rax when the condition is false
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
    if (stackBytes != 0) op("subq", { immOp(stackBytes), useDef(Reg::RSP) });
    for (size_t i = 0; i < onStack.size(); i++) {
        Mem slot = Mem::at(Reg::RSP).plus((long long)i * 8);
        if (isFloatType(onStack[i]->getType())) {
            loadFloat(onStack[i], Reg::XMM0);
            op(movFor(onStack[i]->getType()), { use(Reg::XMM0), memOp(slot) });
        } else {
            loadInt(onStack[i], Reg::RAX, Width::Quad);
            op("movq", { use(Reg::RAX), memOp(slot) });
        }
    }
    std::vector<Reg> argumentRegs;
    for (const std::pair<const ir::Value *, Reg> &entry : inRegs) {
        argumentRegs.push_back(entry.second);
        if (isXmm(entry.second)) {
            loadFloat(entry.first, entry.second);
            continue;
        }
        Width w = widthOf(entry.first->getType());
        loadInt(entry.first, entry.second, w);
        //chars are passed sign-extended and bools zero-extended to 32 bits, as clang does
        if (w == Width::Byte) {
            op(ir::integerBits(entry.first->getType()) == 1 ? "movzbl" : "movsbl",
               { use(entry.second, Width::Byte), def(entry.second, Width::Long) });
        }
    }

    bool direct = inst->callee->getKind() == ir::ValueKind::Function;
    if (!direct) loadInt(inst->callee, Reg::R11, Width::Quad);
    if (type->isVariadic) {
        op("movl", { immOp((long long)floatUsed), def(Reg::RAX, Width::Long) });
        argumentRegs.push_back(Reg::RAX);
    }

    //what the call clobbers: values still needed leave the caller-saved registers
    finishOperands(info);
    if (!global) ra.spillCallerSaved(nextUse);

    MInst call;
    call.op = "callq";
    call.isCall = true;
    call.implicitUses = argumentRegs;
    call.implicitDefs = kCallClobbers;
    if (direct) {
        const ir::Function *callee = static_cast<const ir::Function *>(inst->callee);
        call.operands.push_back(textOp(symbolName(callee) + (callee->isDefinition() ? "" : "@PLT")));
    } else {
        MOperand target = use(Reg::R11);
        target.indirect = true;
        call.operands.push_back(target);
    }
    body.push_back(call);
    if (stackBytes != 0) op("addq", { immOp(stackBytes), useDef(Reg::RSP) });

    if (inst->getType()->is(sema::TypeKind::Void)) return;
    InstNextUse resultOnly;
    resultOnly.result = info.result;
    defineResult(inst, resultOnly, isFloatType(inst->getType()) ? Reg::XMM0 : Reg::RAX);
}

//  phis

void FunctionEmitter::emitPhi(ir::PhiInst *phi, const InstNextUse &info) {
    if (global) return;  //defined by the copies on the edges into this block
    const sema::Type *type = phi->getType();
    MOperand slot = memOp(Mem::frame(frame.phiSlot(phi)));
    if (isFloatType(type)) {
        op(movFor(type), { slot, def(Reg::XMM0) });
        return defineResult(phi, info, Reg::XMM0);
    }
    if (!isIntegerLike(type)) return fail("a phi of an aggregate value is not supported");
    switch (widthOf(type)) {
    case Width::Byte: op("movzbl", { slot, def(Reg::RAX, Width::Long) }); break;
    case Width::Long: op("movl", { slot, def(Reg::RAX, Width::Long) }); break;
    case Width::Quad: op("movq", { slot, def(Reg::RAX) }); break;
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
        op("j" + condCode, { textOp(label(ifTrue)) });
        storePhiCopies(block, ifFalse);
        return jumpUnless(ifFalse, next);
    }
    op("j" + invert(condCode), { textOp(label(ifFalse)) });
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
        MOperand slot = memOp(Mem::frame(frame.phiSlot(phi)));
        if (isFloatType(type)) {
            loadFloat(value, Reg::XMM0);
            op(movFor(type), { use(Reg::XMM0), slot });
            continue;
        }
        Width w = widthOf(type);
        const ir::Constant *c = asConstant(value);
        Reg reg;
        if (held(value, reg)) {
            op(std::string("mov") + suffix(w), { use(reg, w), slot });
        } else if (c && c->constKind == ir::ConstantKind::Int && fitsInt32(c->intValue)) {
            op(std::string("mov") + suffix(w), { immOp(c->intValue), slot });
        } else if (c && c->constKind == ir::ConstantKind::Null) {
            op("movq", { immOp(0), slot });
        } else {
            loadInt(value, Reg::RAX, w);
            op(std::string("mov") + suffix(w), { use(Reg::RAX, w), slot });
        }
    }
}

void FunctionEmitter::copyPhis(ir::BasicBlock *block, const ir::BasicBlock *succ) {
    std::vector<std::pair<Reg, Reg>> finals;  //temporary -> phi
    for (const std::unique_ptr<ir::Instruction> &inst : succ->instructions) {
        if (inst->getOpcode() != ir::Opcode::Phi) break;
        ir::PhiInst *phi = static_cast<ir::PhiInst *>(inst.get());
        const ir::Value *value = phi->getIncomingValueFor(block);
        if (value == nullptr) continue;
        bool isFloat = isFloatType(phi->getType());
        Reg temp = newVreg(isFloat);
        Reg reg;
        if (held(value, reg)) {
            op(isFloat ? "movaps" : "movq", { use(reg), def(temp) });
        } else if (isFloat) {
            loadFloat(value, temp);
        } else {
            loadInt(value, temp, Width::Quad);
        }
        finals.push_back(std::make_pair(temp, vregFor(phi)));
    }
    for (const std::pair<Reg, Reg> &copy : finals) {
        op(isXmm(copy.first) ? "movaps" : "movq", { use(copy.first), def(copy.second) });
    }
}

void FunctionEmitter::branchSplit(const std::string &condCode, ir::BasicBlock *block, ir::BasicBlock *ifTrue,
                                  ir::BasicBlock *ifFalse, const ir::BasicBlock *next) {
    bool trueCopies = hasPhiFrom(block, ifTrue);
    bool falseCopies = hasPhiFrom(block, ifFalse);
    if (!trueCopies && !falseCopies) return branchOn(condCode, ifTrue, ifFalse, next);
    if (!trueCopies) {
        op("j" + condCode, { textOp(label(ifTrue)) });
        copyPhis(block, ifFalse);
        return jumpUnless(ifFalse, next);
    }
    if (!falseCopies) {
        op("j" + invert(condCode), { textOp(label(ifFalse)) });
        copyPhis(block, ifTrue);
        return jumpUnless(ifTrue, next);
    }
    std::string trueEdge = localLabel();
    op("j" + condCode, { textOp(trueEdge) });
    copyPhis(block, ifFalse);
    op("jmp", { textOp(label(ifFalse)) });
    placeLabel(trueEdge);
    copyPhis(block, ifTrue);
    jumpUnless(ifTrue, next);
}

//  terminators

void FunctionEmitter::jumpUnless(const ir::BasicBlock *target, const ir::BasicBlock *next) {
    if (target != next) op("jmp", { textOp(label(target)) });
}

void FunctionEmitter::branchOn(const std::string &condCode, ir::BasicBlock *ifTrue, ir::BasicBlock *ifFalse,
                               const ir::BasicBlock *next) {
    if (ifTrue == next) {
        op("j" + invert(condCode), { textOp(label(ifFalse)) });
        return;
    }
    op("j" + condCode, { textOp(label(ifTrue)) });
    jumpUnless(ifFalse, next);
}

void FunctionEmitter::emitTerminator(ir::Instruction *inst, const InstNextUse &info, const passes::ValueSet &liveOut,
                                     ir::BasicBlock *next) {
    (void)info;
    ir::BasicBlock *block = inst->getParent();
    ir::BasicBlock *deferred = nullptr;
    if (!global) {
        //the successors' phi copies come first, while the flags do not matter yet (a repeated edge
        //carries the same values). only a loop exit may wait until after the jump, see deferredEdge
        if (inst->getOpcode() == ir::Opcode::Br) deferred = deferredEdge(block, static_cast<ir::BranchInst *>(inst));
        std::set<const ir::BasicBlock *> done;
        if (deferred) done.insert(deferred);
        for (ir::BasicBlock *succ : inst->getSuccessors()) {
            if (done.insert(succ).second) storePhiCopies(block, succ);
        }
        //end of block: values live on exit go back to memory. movs leave the flags alone
        ra.flush(liveOut);
    }

    switch (inst->getOpcode()) {
    case ir::Opcode::Ret: {
        ir::ReturnInst *ret = static_cast<ir::ReturnInst *>(inst);
        std::vector<Reg> returned;
        if (ret->returnValue) {
            const sema::Type *type = ret->returnType;
            if (isFloatType(type)) {
                loadFloat(ret->returnValue, Reg::XMM0);
                returned.push_back(Reg::XMM0);
            } else if (isIntegerLike(type)) {
                loadInt(ret->returnValue, Reg::RAX, widthOf(type));
                returned.push_back(Reg::RAX);
            } else {
                return fail("returning an aggregate value is not supported");
            }
        }
        //the epilogue reads the return value: the jump to it (or a marker when it falls through) says so
        if (next != nullptr) {
            op("jmp", { textOp(".Lret" + std::to_string(id)) }, returned);
        } else {
            MInst marker;
            marker.kind = MInst::Marker;
            marker.implicitUses = returned;
            body.push_back(marker);
        }
        return;
    }
    case ir::Opcode::Br: {
        ir::BranchInst *br = static_cast<ir::BranchInst *>(inst);
        if (!br->isConditional()) {
            if (global) copyPhis(block, br->target);
            return jumpUnless(br->target, next);
        }

        const ir::Instruction *condInst = br->condition->getKind() == ir::ValueKind::Instruction
                                              ? static_cast<const ir::Instruction *>(br->condition)
                                              : nullptr;
        if (condInst && fused.count(condInst)) {
            ir::BinaryInst *cmp = static_cast<ir::BinaryInst *>(br->condition);
            emitIntCompare(cmp);
            if (global) return branchSplit(conditionCode(cmp->op), block, br->target, br->falseTarget, next);
            return branchWithCopies(conditionCode(cmp->op), block, br->target, br->falseTarget, deferred, next);
        }
        const ir::Constant *c = asConstant(br->condition);
        if (c) {
            ir::BasicBlock *taken = c->intValue ? br->target : br->falseTarget;
            if (global) {
                copyPhis(block, taken);
            } else if (taken == deferred) {
                storePhiCopies(block, taken);
            }
            return jumpUnless(taken, next);
        }
        op("testb", { immOp(1), intSource(br->condition, Width::Byte, Reg::RAX) });
        if (global) return branchSplit("ne", block, br->target, br->falseTarget, next);
        return branchWithCopies("ne", block, br->target, br->falseTarget, deferred, next);
    }
    case ir::Opcode::Switch: {
        ir::SwitchInst *sw = static_cast<ir::SwitchInst *>(inst);
        Width w = widthOf(sw->condition->getType());
        loadInt(sw->condition, Reg::RAX, w);
        //-O3: a case whose block has phis goes through a local label that does that edge's copies
        //(in order of first use, so the output does not depend on addresses)
        std::vector<std::pair<const ir::BasicBlock *, std::string>> edgeLabels;
        for (const std::pair<long long, ir::BasicBlock *> &c : sw->cases) {
            std::string target = label(c.second);
            if (global && hasPhiFrom(block, c.second)) {
                std::string edge;
                for (const std::pair<const ir::BasicBlock *, std::string> &e : edgeLabels) {
                    if (e.first == c.second) edge = e.second;
                }
                if (edge.empty()) {
                    edge = localLabel();
                    edgeLabels.push_back(std::make_pair(c.second, edge));
                }
                target = edge;
            }
            op(std::string("cmp") + suffix(w), { immOp(c.first), use(Reg::RAX, w) });
            op("je", { textOp(target) });
        }
        if (!global) return jumpUnless(sw->defaultTarget, next);
        copyPhis(block, sw->defaultTarget);
        if (edgeLabels.empty()) return jumpUnless(sw->defaultTarget, next);
        op("jmp", { textOp(label(sw->defaultTarget)) });
        for (const std::pair<const ir::BasicBlock *, std::string> &edge : edgeLabels) {
            placeLabel(edge.second);
            copyPhis(block, edge.first);
            op("jmp", { textOp(label(edge.first)) });
        }
        return;
    }
    case ir::Opcode::Unreachable:
        op("ud2");
        return;
    default:
        return fail("unknown terminator");
    }
}

} // namespace codegen
