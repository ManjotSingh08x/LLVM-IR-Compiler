#ifndef CODEGEN_GRAPH_COLOR_H
#define CODEGEN_GRAPH_COLOR_H

#include <map>
#include <set>
#include <string>

#include "codegen/frame.h"
#include "codegen/mir.h"

namespace codegen {

//counters describing one colouring run
struct ColorStats {
    std::set<Reg> calleeSavedUsed;
    unsigned      virtuals = 0;
    unsigned      coalesced = 0;
    unsigned      spilled = 0;
    unsigned      rounds = 0;
};

//assigns registers by graph colouring, spilling when needed
bool colorMachineCode(MCode &code, Frame &frame, const std::map<std::string, unsigned> &labelDepth,
                      ColorStats &stats, std::string &error);

}

#endif
