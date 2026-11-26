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
#include "pipeline/analysis_manager.h"

namespace codegen {

//turns one IR function into machine code
class FunctionEmitter {
public:
    FunctionEmitter(ir::Function &fn, passes::AnalysisManager &am, sema::TypeLayout &layout, ConstantPool &pool,
                    int id, int optLevel);

    bool emit(AsmLines &out);

    const std::string &error() const { return errorText; }
    const ColorStats  &colorStats() const { return stats; }

private:
    void        layoutFrame();
    //moves incoming argument registers into their homes
    void        storeIncomingArguments(MCode &code);
    void        emitBlock(ir::BasicBlock *block, ir::BasicBlock *next);
    void        emitInstruction(ir::Instruction *inst, const InstNextUse &info);
    void        emitTerminator(ir::Instruction *inst, const InstNextUse &info, const passes::ValueSet &liveOut,
                               ir::BasicBlock *next);
    std::string label(const ir::BasicBlock *block) const;
    //true when a compare merges into its branch
    bool        isFusedCompare(const ir::BasicBlock *block, size_t index) const;

    void     loadInt(const ir::Value *value, Reg dst, Width width);
    MOperand intSource(const ir::Value *value, Width width, Reg scratch);
    void     loadFloat(const ir::Value *value, Reg dst);
    MOperand floatSource(const ir::Value *value, Reg scratch);
    Mem      addressOf(const ir::Value *pointer, Reg scratch);
    bool     isRegisterValue(const ir::Value *value) const;

    bool held(const ir::Value *value, Reg &reg);
    Reg  vregFor(const ir::Value *value);
    Reg  newVreg(bool isFloat);

    void finishOperands(const InstNextUse &info);
    void defineResult(const ir::Instruction *inst, const InstNextUse &info, Reg src);

    void emitLoad(ir::LoadInst *inst, const InstNextUse &info);
    void emitStore(ir::StoreInst *inst, const InstNextUse &info);
    void storeConstant(const ir::Constant *constant, const sema::Type *type, const Mem &mem);
    //copies a memory block between two addresses
    void copyBytes(const Mem &from, const Mem &to, uint64_t size);
    void emitGEP(ir::GEPInst *inst, const InstNextUse &info);
    void emitBinary(ir::BinaryInst *inst, const InstNextUse &info);
    //divides by a power of two using shifts
    Reg  divideByPowerOfTwo(bool remainder, Width width, unsigned shift);
    void widen(Reg reg, Width width, bool isSigned);
    void emitIntCompare(ir::BinaryInst *inst);
    void emitFloatBinary(ir::BinaryInst *inst, const InstNextUse &info);
    void emitUnary(ir::UnaryInst *inst, const InstNextUse &info);
    void emitCast(ir::CastInst *inst, const InstNextUse &info);
    void emitSelect(ir::SelectInst *inst, const InstNextUse &info);
    //sets up arguments per the System V ABI
    void emitCall(ir::CallInst *inst, const InstNextUse &info);

    void emitPhi(ir::PhiInst *phi, const InstNextUse &info);
    //writes phi inputs into slots before leaving a block
    void storePhiCopies(ir::BasicBlock *block, const ir::BasicBlock *succ);
    bool hasPhiFrom(const ir::BasicBlock *block, const ir::BasicBlock *succ) const;
    //finds an edge whose phi copies must be split
    ir::BasicBlock *deferredEdge(ir::BasicBlock *block, ir::BranchInst *br);
    void branchWithCopies(const std::string &condCode, ir::BasicBlock *block, ir::BasicBlock *ifTrue,
                          ir::BasicBlock *ifFalse, ir::BasicBlock *deferred, const ir::BasicBlock *next);

    void copyPhis(ir::BasicBlock *block, const ir::BasicBlock *succ);
    void branchSplit(const std::string &condCode, ir::BasicBlock *block, ir::BasicBlock *ifTrue,
                     ir::BasicBlock *ifFalse, const ir::BasicBlock *next);

    void jumpUnless(const ir::BasicBlock *target, const ir::BasicBlock *next);
    void branchOn(const std::string &condCode, ir::BasicBlock *ifTrue, ir::BasicBlock *ifFalse,
                  const ir::BasicBlock *next);

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
    bool                     global;

    Frame             frame;
    MCode             body;
    RegisterAllocator ra;
    NextUseMap        nextUse;
    std::set<const ir::Instruction *> fused;
    unsigned          localCount = 0;
    std::string       errorText;

    std::map<const ir::Value *, Reg> vregs;
    unsigned                         intVregs = 0;
    unsigned                         floatVregs = 0;
    ColorStats                       stats;
};

}

#endif
