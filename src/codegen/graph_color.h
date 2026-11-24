#ifndef CODEGEN_GRAPH_COLOR_H
#define CODEGEN_GRAPH_COLOR_H

#include <map>
#include <set>
#include <string>

#include "codegen/frame.h"
#include "codegen/mir.h"

namespace codegen {

//what one function's colouring did, for the report and the prologue
struct ColorStats {
    std::set<Reg> calleeSavedUsed;  //to save in the prologue and restore in the epilogue
    unsigned      virtuals = 0;     //virtual registers at the start
    unsigned      coalesced = 0;    //copies removed by merging their two registers
    unsigned      spilled = 0;      //virtual registers sent to the stack
    unsigned      rounds = 0;       //colouring attempts (one more after each round of spills)
};

//register allocation by graph colouring (dragon book 8.8.4, chaitin; with briggs' and george's
//conservative coalescing and optimistic colouring) over one function's machine ir at -O3:
//  1. the machine cfg (labels, jumps, fall-through) and liveness of every register, physical and
//     virtual, from the reads and writes each instruction states (readsAndWrites)
//  2. the interference graph: a register written at a point interferes with every register live
//     after it, except the source of a copy that writes it. physical registers are nodes too, with
//     their colour fixed (pre-coloured): a call's implicit writes make every value live across it
//     interfere with all caller-saved registers, idiv's with rax and rdx, a shift's with rcx
//  3. coalescing: a copy between two virtual registers that do not interfere merges them when the
//     merged node has fewer than k neighbours of degree k or more (briggs); a copy between a
//     virtual and a physical register merges when every neighbour of the virtual one is
//     pre-coloured, of low degree, or already interferes with the physical one (george)
//  4. simplify: a virtual node with fewer than k neighbours can always be coloured, so it is removed
//     and pushed; when none is left, the one with the lowest spill cost per neighbour is pushed
//     optimistically (cost: reads and writes, times 10 per level of loop nesting)
//  5. select: nodes are popped and given a register no neighbour has. one that finds none is spilled:
//     a stack slot, a load into a new short-lived register before each read and a store after each
//     write; then everything is done again
//k: 14 for integers (every general register but rsp and rbp), 16 for floats (xmm0-xmm15).
//labelDepth gives the loop depth of each ir block label, for the spill costs
bool colorMachineCode(MCode &code, Frame &frame, const std::map<std::string, unsigned> &labelDepth,
                      ColorStats &stats, std::string &error);

} // namespace codegen

#endif
