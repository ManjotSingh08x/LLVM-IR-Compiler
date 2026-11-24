#ifndef CODEGEN_ISEL_H
#define CODEGEN_ISEL_H

#include <map>
#include <set>
#include <string>

#include "codegen/frame.h"
#include "codegen/graph_color.h"
#include "codegen/mir.h"
#include "codegen/next_use.h"
#include "codegen/reg_alloc.h"
#include "semantic/type_layout.h"
#include "codegen/x86.h"
#include "passes/analysis_manager.h"

namespace codegen {

//turns one ir function into x86-64, one ir instruction at a time, as machine ir (mir.h) whose every
//operand says whether it is read or written. each result is computed in rax or xmm0, then:
//  -O0 to -O2: handed to the 8.6 register allocator (registers cached within a block, values live
//    across blocks in their homes, phis through slots in the frame)
//  -O3: copied into a virtual register of its own. phis become copies on the edges into their block.
//    graph colouring (graph_color.h) then gives every virtual register a physical one
//system v calling convention, %rbp frame
class FunctionEmitter {
public:
    FunctionEmitter(ir::Function &fn, passes::AnalysisManager &am, sema::TypeLayout &layout, ConstantPool &pool,
                    int id, int optLevel);

    //writes the whole function: directives, prologue, blocks, epilogue. false on ir it cannot lower
    bool emit(AsmLines &out);

    const std::string &error() const { return errorText; }
    const ColorStats  &colorStats() const { return stats; }

private:
    //structure
    void        layoutFrame();
    void        storeIncomingArguments(MCode &code);
    void        emitBlock(ir::BasicBlock *block, ir::BasicBlock *next);
    void        emitInstruction(ir::Instruction *inst, const InstNextUse &info);
    void        emitTerminator(ir::Instruction *inst, const InstNextUse &info, const passes::ValueSet &liveOut,
                               ir::BasicBlock *next);
    std::string label(const ir::BasicBlock *block) const;
    bool        isFusedCompare(const ir::BasicBlock *block, size_t index) const;

    //operands
    void     loadInt(const ir::Value *value, Reg dst, Width width);
    MOperand intSource(const ir::Value *value, Width width, Reg scratch);
    void     loadFloat(const ir::Value *value, Reg dst);
    MOperand floatSource(const ir::Value *value, Reg scratch);
    Mem      addressOf(const ir::Value *pointer, Reg scratch);
    bool     isRegisterValue(const ir::Value *value) const;

    //the register value is in: at -O0 to -O2 a register caching it, at -O3 its virtual register
    bool held(const ir::Value *value, Reg &reg);
    Reg  vregFor(const ir::Value *value);
    Reg  newVreg(bool isFloat);

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
    Reg  divideByPowerOfTwo(bool remainder, Width width, unsigned shift);
    void emitIntCompare(ir::BinaryInst *inst);  //sets the flags only
    void emitFloatBinary(ir::BinaryInst *inst, const InstNextUse &info);
    void emitUnary(ir::UnaryInst *inst, const InstNextUse &info);
    void emitCast(ir::CastInst *inst, const InstNextUse &info);
    void emitSelect(ir::SelectInst *inst, const InstNextUse &info);
    void emitCall(ir::CallInst *inst, const InstNextUse &info);

    //phis at -O0 to -O2: each has a slot in the frame. every predecessor stores its incoming value
    //there before it jumps, and the phi is defined at the top of its block by loading the slot. the
    //copies read values, never another phi's slot, so no copy can clobber one still to be read (the
    //swap and lost-copy problems of leaving ssa) and no edge needs splitting
    void emitPhi(ir::PhiInst *phi, const InstNextUse &info);
    void storePhiCopies(ir::BasicBlock *block, const ir::BasicBlock *succ);  //the copies for one edge
    bool hasPhiFrom(const ir::BasicBlock *block, const ir::BasicBlock *succ) const;
    //copies go before the compare, on both edges (a store on the edge not taken is cheaper than a
    //jump). the exception: when both edges need copies and one is a loop's back edge, the other
    //(the loop exit) does its copies after the jump instruction, so the loop does not pay for them.
    //returns that exit edge's target, or null
    ir::BasicBlock *deferredEdge(ir::BasicBlock *block, ir::BranchInst *br);
    void branchWithCopies(const std::string &condCode, ir::BasicBlock *block, ir::BasicBlock *ifTrue,
                          ir::BasicBlock *ifFalse, ir::BasicBlock *deferred, const ir::BasicBlock *next);

    //phis at -O3: the copies of one edge, each incoming value first into a fresh register and then
    //into the phi's register, so the copies act in parallel (a, b = b, a works). they are placed on
    //the edge itself: after the jump instruction of a conditional branch, behind a local label when
    //both sides need them, which splits critical edges and so avoids the lost copy
    void copyPhis(ir::BasicBlock *block, const ir::BasicBlock *succ);
    void branchSplit(const std::string &condCode, ir::BasicBlock *block, ir::BasicBlock *ifTrue,
                     ir::BasicBlock *ifFalse, const ir::BasicBlock *next);

    void jumpUnless(const ir::BasicBlock *target, const ir::BasicBlock *next);
    void branchOn(const std::string &condCode, ir::BasicBlock *ifTrue, ir::BasicBlock *ifFalse,
                  const ir::BasicBlock *next);

    //one instruction into the body
    void        op(const std::string &mnemonic, std::vector<MOperand> operands = {}, std::vector<Reg> implicitUses = {},
                   std::vector<Reg> implicitDefs = {});
    void        placeLabel(const std::string &name);
    void        fail(const std::string &message);
    std::string localLabel();

    ir::Function            &fn;
    passes::AnalysisManager &am;
    sema::TypeLayout            &layout;
    ConstantPool            &pool;
    int                      id;
    bool                     global;  //-O3: virtual registers and graph colouring

    Frame             frame;
    MCode             body;
    RegisterAllocator ra;
    NextUseMap        nextUse;  //the current next use of every value held in a register
    std::set<const ir::Instruction *> fused;  //compares emitted as part of their branch
    unsigned          localCount = 0;
    std::string       errorText;

    std::map<const ir::Value *, Reg> vregs;
    unsigned                         intVregs = 0;
    unsigned                         floatVregs = 0;
    ColorStats                       stats;
};

} // namespace codegen

#endif
