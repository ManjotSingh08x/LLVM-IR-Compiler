#ifndef CODEGEN_PEEPHOLE_H
#define CODEGEN_PEEPHOLE_H

#include "codegen/x86.h"

namespace codegen {

//peephole optimization over one function's emitted instructions, repeated until nothing changes
void peephole(AsmLines &lines);

} // namespace codegen

#endif
