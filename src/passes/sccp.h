#ifndef PASSES_SCCP_H
#define PASSES_SCCP_H

#include "passes/pass.h"

namespace passes {

//sparse conditional constant propagation (wegman and zadeck; not in the dragon book, whose 9.4 is
//the dense, unconditional version). on ssa:
//  sparse: one lattice cell per value (TOP, a constant, BOTTOM), lowered along def-use edges
//  conditional: cfg edges start not executable; a branch on a constant marks only the edge it
//    takes, a phi meets only the entries of executable edges, and a block's instructions are
//    looked at only once an edge into it is executable
//two worklists, cfg edges and ssa values, run until both are empty. then every value that is a
//constant is replaced by it, branches on constants become plain jumps, and blocks that never
//became executable are deleted (with the phi entries they fed). a branch whose condition is
//still TOP at the end is treated as going both ways, and solving resumes
class SCCP : public FunctionPass {
public:
    const char *name() const override { return "sccp"; }
    bool        run(ir::Function &fn, ir::Module &module, AnalysisManager &am) override;
};

} // namespace passes

#endif
