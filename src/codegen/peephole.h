#ifndef CODEGEN_PEEPHOLE_H
#define CODEGEN_PEEPHOLE_H

#include "codegen/x86.h"

namespace codegen {

//cleans up redundant instructions in emitted assembly
void peephole(AsmLines &lines);

}

#endif
