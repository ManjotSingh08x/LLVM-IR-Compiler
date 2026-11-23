#include "codegen/codegen.h"

#include "codegen/asm_writer.h"

namespace codegen {

bool emitAssembly(ir::Module &module, passes::AnalysisManager &am, int optLevel, std::ostream &out,
                  std::string &error) {
    (void)optLevel;  //-O2 adds the peephole pass here
    AsmWriter writer(module, am);
    if (writer.write(out)) return true;
    error = writer.error();
    return false;
}

} // namespace codegen
