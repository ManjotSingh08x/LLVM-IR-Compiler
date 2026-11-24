#ifndef CODEGEN_ASM_WRITER_H
#define CODEGEN_ASM_WRITER_H

#include <ostream>
#include <string>

#include "semantic/type_layout.h"
#include "codegen/x86.h"
#include "pipeline/analysis_manager.h"

namespace codegen {

//writes a module as x86-64 gnu assembly (at&t syntax)
class AsmWriter {
public:
    AsmWriter(ir::Module &module, passes::AnalysisManager &am, int optLevel);

    //false when the module uses ir the backend cannot lower, see error()
    bool write(std::ostream &out);

    const std::string &error() const { return errorText; }

private:
    void writeGlobal(std::ostream &out, const ir::GlobalVariable *global);
    void writeConstant(std::ostream &out, const ir::Constant *constant, const sema::Type *type);

    ir::Module              &module;
    passes::AnalysisManager &am;
    sema::TypeLayout             layout;
    ConstantPool             pool;
    int                      optLevel;
    std::string              errorText;
};

} // namespace codegen

#endif
