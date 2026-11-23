#ifndef PASSES_INLINE_H
#define PASSES_INLINE_H

#include "passes/pass.h"

namespace passes {

//procedure inlining (dragon book 12.1, 1.5.1): a call is replaced by a copy of the callee's body.
//functions are visited bottom-up over the call graph, so each body copied has already had its own
//calls inlined. a call site is inlined when
//  the call is direct and the callee is defined here, not variadic, not recursive (not in a cycle
//  of the call graph) and not the caller itself
//  the callee is small (kSmallCallee instructions) or this is its only call site (kSingleCallee)
//  the caller has not grown past kCallerLimit
//the copy: the caller's block is split at the call, the callee's blocks are cloned between the
//two halves with its parameters mapped to the arguments, its allocas move to the caller's entry
//block, and each ret jumps to the second half. with one ret its value replaces the call; with
//several it goes through a stack slot (mem2reg turns it into a phi later)
class Inliner : public GlobalPass {
public:
    const char *name() const override { return "inline"; }
    bool        run(ir::Module &module, AnalysisManager &am) override;

    static const size_t kSmallCallee = 40;
    static const size_t kSingleCallee = 400;
    static const size_t kCallerLimit = 4000;
};

} // namespace passes

#endif
