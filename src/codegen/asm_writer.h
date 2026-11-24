#ifndef CODEGEN_ASM_WRITER_H
#define CODEGEN_ASM_WRITER_H

#include <ostream>
#include <string>

#include "semantic/type_layout.h"
#include "codegen/x86.h"
#include "passes/analysis_manager.h"

namespace codegen {

//writes a whole module as gnu assembler text (at&t syntax) for x86-64 linux
//  .text    every defined function
//  .data    globals with an initializer, .bss when it is all zeros, .rodata for constants
//  .init_array  the function that runs global constructors before main
class AsmWriter {
public:
    //optLevel 1 and up runs the peephole pass over each function
    AsmWriter(ir::Module &module, passes::AnalysisManager &am, int optLevel);

    //false when a function uses ir the backend cannot lower, see error()
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
