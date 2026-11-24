#ifndef CODEGEN_GRAPH_COLOR_H
#define CODEGEN_GRAPH_COLOR_H

#include <map>
#include <set>
#include <string>

#include "codegen/frame.h"
#include "codegen/mir.h"

namespace codegen {

struct ColorStats {
    std::set<Reg> calleeSavedUsed;
    unsigned      virtuals = 0;
    unsigned      coalesced = 0;
    unsigned      spilled = 0;
    unsigned      rounds = 0;
};

//register allocation by graph colouring with coalescing and spilling, used at -O3
bool colorMachineCode(MCode &code, Frame &frame, const std::map<std::string, unsigned> &labelDepth,
                      ColorStats &stats, std::string &error);

} // namespace codegen

#endif
