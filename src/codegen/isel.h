#ifndef CODEGEN_ISEL_H
#define CODEGEN_ISEL_H

#include <set>
#include <string>

#include "codegen/frame.h"
#include "codegen/next_use.h"
#include "codegen/reg_alloc.h"
#include "codegen/target_layout.h"
#include "codegen/x86.h"
#include "passes/analysis_manager.h"

namespace codegen {

//turns one ir function into x86-64, one ir instruction at a time
//operands are read from their register or home, the result is computed in rax or xmm0 and
//handed to the register allocator. system v calling convention, %rbp frame
class FunctionEmitter {
public:
    FunctionEmitter(ir::Function &fn, passes::AnalysisManager &am, TargetLayout &layout, ConstantPool &pool,
                    int id);

    //writes the whole function: directives, prologue, blocks, epilogue. false on ir it cannot lower
    bool emit(AsmLines &out);

    const std::string &error() const { return errorText; }

private:
    //structure
    void        layoutFrame();
    void        storeIncomingArguments(AsmLines &lines);
    void        emitBlock(ir::BasicBlock *block, ir::BasicBlock *next);
    void        emitInstruction(ir::Instruction *inst, const InstNextUse &info);
    void        emitTerminator(ir::Instruction *inst, const InstNextUse &info, const passes::ValueSet &liveOut,
                               ir::BasicBlock *next);
    std::string label(const ir::BasicBlock *block) const;
    bool        isFusedCompare(const ir::BasicBlock *block, size_t index) const;

    //operands
    void        loadInt(const ir::Value *value, Reg dst, Width width);
    std::string intSource(const ir::Value *value, Width width, Reg scratch);
    void        loadFloat(const ir::Value *value, Reg dst);
    std::string floatSource(const ir::Value *value, Reg scratch);
    Mem         addressOf(const ir::Value *pointer, Reg scratch);
    bool        isRegisterValue(const ir::Value *value) const;

    //after an instruction: operands that died free their registers, the result goes to one
    void finishOperands(const InstNextUse &info);
    void defineResult(const ir::Instruction *inst, const InstNextUse &info, Reg src);

    //instruction kinds
    void emitLoad(ir::LoadInst *inst, const InstNextUse &info);
    void emitStore(ir::StoreInst *inst, const InstNextUse &info);
    void storeConstant(const ir::Constant *constant, const sema::Type *type, const Mem &mem);
    void copyBytes(const Mem &from, const Mem &to, uint64_t size);
    void emitGEP(ir::GEPInst *inst, const InstNextUse &info);
    void emitBinary(ir::BinaryInst *inst, const InstNextUse &info);
    void emitIntCompare(ir::BinaryInst *inst);  //sets the flags only
    void emitFloatBinary(ir::BinaryInst *inst, const InstNextUse &info);
    void emitUnary(ir::UnaryInst *inst, const InstNextUse &info);
    void emitCast(ir::CastInst *inst, const InstNextUse &info);
    void emitSelect(ir::SelectInst *inst, const InstNextUse &info);
    void emitCall(ir::CallInst *inst, const InstNextUse &info);
    void jumpUnless(const ir::BasicBlock *target, const ir::BasicBlock *next);
    void branchOn(const std::string &condCode, ir::BasicBlock *ifTrue, ir::BasicBlock *ifFalse,
                  const ir::BasicBlock *next);

    void        line(const std::string &text) { body.push_back("\t" + text); }
    void        fail(const std::string &message);
    std::string localLabel();

    ir::Function            &fn;
    passes::AnalysisManager &am;
    TargetLayout            &layout;
    ConstantPool            &pool;
    int                      id;

    Frame             frame;
    AsmLines          body;
    RegisterAllocator ra;
    NextUseMap        nextUse;  //the current next use of every value held in a register
    std::set<const ir::Instruction *> fused;  //compares emitted as part of their branch
    unsigned          localCount = 0;
    std::string       errorText;
};

} // namespace codegen

#endif
