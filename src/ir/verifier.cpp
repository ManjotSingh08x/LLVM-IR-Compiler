#include "ir/verifier.h"

#include <map>
#include <set>
#include <sstream>

namespace ir {

namespace {

//one line of llvm text for an instruction, used in messages
std::string describe(const Instruction *inst) {
    std::ostringstream out;
    inst->dump(out);
    std::string text = out.str();
    while (!text.empty() && (text.back() == '\n' || text.back() == ' ')) text.pop_back();
    size_t start = text.find_first_not_of(" \t");
    return start == std::string::npos ? text : text.substr(start);
}

bool fail(std::string &error, const BasicBlock *block, const std::string &message) {
    error = "in block %" + block->getName() + ": " + message;
    return false;
}

} // namespace

bool verifyFunction(const Function &fn, std::string &error) {
    const std::vector<BasicBlock *> &blocks = fn.getBlocks();
    std::set<const BasicBlock *> placed(blocks.begin(), blocks.end());

    //where each placed instruction lives, and its position in its block
    std::map<const Instruction *, size_t> position;
    for (const BasicBlock *block : blocks) {
        for (size_t i = 0; i < block->instructions.size(); i++) {
            position[block->instructions[i].get()] = i;
        }
    }

    //how many edges run from each block to each other block, repeats counted
    std::map<const BasicBlock *, std::map<const BasicBlock *, size_t>> edgesInto;
    for (const BasicBlock *block : blocks) {
        const Instruction *term = block->getTerminator();
        if (term == nullptr) continue;
        for (const BasicBlock *next : term->getSuccessors()) edgesInto[next][block]++;
    }

    std::set<std::string> names;
    for (const std::unique_ptr<Argument> &arg : fn.args) {
        if (!arg->getName().empty() && !names.insert(arg->getName()).second) {
            error = "argument name %" + arg->getName() + " is used twice";
            return false;
        }
    }

    for (const BasicBlock *block : blocks) {
        if (block->parent != &fn) return fail(error, block, "block belongs to another function");
        if (!names.insert(block->getName()).second) {
            return fail(error, block, "the label is used twice in the function");
        }
        if (block->instructions.empty()) return fail(error, block, "the block is empty");

        bool pastPhis = false;
        for (size_t i = 0; i < block->instructions.size(); i++) {
            const Instruction *inst = block->instructions[i].get();
            const bool last = i + 1 == block->instructions.size();

            if (inst->getParent() != block) {
                return fail(error, block, "'" + describe(inst) + "' does not name this block as its parent");
            }
            if (inst->isTerminator() != last) {
                return fail(error, block, last ? "the block does not end in a terminator"
                                               : "terminator '" + describe(inst) + "' is not last");
            }
            if (!inst->getName().empty() && !names.insert(inst->getName()).second) {
                return fail(error, block, "the name %" + inst->getName() + " is defined twice");
            }

            //phis: at the top, one entry per incoming edge
            if (inst->getOpcode() == Opcode::Phi) {
                if (pastPhis) return fail(error, block, "phi '" + describe(inst) + "' is not at the top");
                const PhiInst *phi = static_cast<const PhiInst *>(inst);
                std::map<const BasicBlock *, size_t> entries;
                for (const std::pair<Value *, BasicBlock *> &in : phi->incoming) entries[in.second]++;
                if (entries != edgesInto[block]) {
                    return fail(error, block, "phi '" + describe(inst) +
                                                  "' does not have exactly one entry per incoming edge");
                }
            } else {
                pastPhis = true;
            }

            //operands
            for (const Value *operand : inst->getOperands()) {
                if (operand == nullptr) {
                    return fail(error, block, "'" + describe(inst) + "' has a missing operand");
                }
                switch (operand->getKind()) {
                case ValueKind::Constant:
                case ValueKind::GlobalVariable:
                case ValueKind::Function:
                    break;
                case ValueKind::Argument:
                    if (static_cast<const Argument *>(operand)->parent != &fn) {
                        return fail(error, block, "'" + describe(inst) + "' uses another function's argument");
                    }
                    break;
                case ValueKind::BasicBlock:
                    return fail(error, block, "'" + describe(inst) + "' uses a block as a value");
                case ValueKind::Instruction: {
                    const Instruction *def = static_cast<const Instruction *>(operand);
                    if (position.count(def) == 0) {
                        return fail(error, block, "'" + describe(inst) + "' uses %" + def->getName() +
                                                      ", which is erased or not in this function");
                    }
                    //phi operands are read at the end of their predecessor, others in order
                    if (inst->getOpcode() != Opcode::Phi && def->getParent() == block && position[def] >= i) {
                        return fail(error, block, "'" + describe(inst) + "' uses %" + def->getName() +
                                                      " before it is defined");
                    }
                    break;
                }
                }
            }

            //jump targets
            for (const BasicBlock *next : inst->getSuccessors()) {
                if (next == nullptr || placed.count(next) == 0) {
                    return fail(error, block, "'" + describe(inst) + "' jumps to a block that is not placed");
                }
            }
            if (inst->getOpcode() == Opcode::Phi) {
                for (const std::pair<Value *, BasicBlock *> &in : static_cast<const PhiInst *>(inst)->incoming) {
                    if (placed.count(in.second) == 0) {
                        return fail(error, block, "phi '" + describe(inst) + "' names a block that is not placed");
                    }
                }
            }
        }

        //the edge lists must be what recomputeCFG would build: no repeats, terminator order
        std::vector<const BasicBlock *> expectedSuccs;
        std::set<const BasicBlock *> seen;
        for (const BasicBlock *next : block->getTerminator()->getSuccessors()) {
            if (seen.insert(next).second) expectedSuccs.push_back(next);
        }
        std::set<const BasicBlock *> haveSuccs(block->successors.begin(), block->successors.end());
        if (haveSuccs != seen || block->successors.size() != expectedSuccs.size()) {
            return fail(error, block, "the successor list does not match the terminator");
        }
        std::set<const BasicBlock *> expectedPreds;
        for (const std::pair<const BasicBlock *const, size_t> &edge : edgesInto[block]) {
            expectedPreds.insert(edge.first);
        }
        std::set<const BasicBlock *> havePreds(block->predecessors.begin(), block->predecessors.end());
        if (havePreds != expectedPreds || block->predecessors.size() != expectedPreds.size()) {
            return fail(error, block, "the predecessor list does not match the terminators");
        }
    }
    return true;
}

} // namespace ir
