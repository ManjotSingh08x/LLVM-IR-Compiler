#include "codegen/asm_writer.h"

#include "codegen/isel.h"
#include "codegen/peephole.h"
#include "ir/ir_types.h"

namespace codegen {

namespace {

unsigned log2Of(uint64_t align) {
    unsigned shift = 0;
    while ((1ULL << shift) < align) shift++;
    return shift;
}

bool isAllZero(const ir::Constant *constant) {
    switch (constant->constKind) {
    case ir::ConstantKind::Int:      return constant->intValue == 0;
    case ir::ConstantKind::Float:    return floatBits(constant->floatValue, true) == 0;
    case ir::ConstantKind::Null:
    case ir::ConstantKind::ZeroInit: return true;
    case ir::ConstantKind::String:   return constant->bytes.find_first_not_of('\0') == std::string::npos;
    case ir::ConstantKind::Address:  return false;
    case ir::ConstantKind::Aggregate:
        for (const ir::Constant *element : constant->elements) {
            if (!isAllZero(element)) return false;
        }
        return true;
    }
    return false;
}

} // namespace

AsmWriter::AsmWriter(ir::Module &module, passes::AnalysisManager &am, int optLevel)
    : module(module), am(am), layout(module.symTab), optLevel(optLevel) {}

bool AsmWriter::write(std::ostream &out) {
    int id = 0;
    for (const std::unique_ptr<ir::Function> &fn : module.getFunctions()) {
        if (!fn->isDefinition()) continue;
        FunctionEmitter emitter(*fn, am, layout, pool, id++, optLevel);
        AsmLines lines;
        if (!emitter.emit(lines)) {
            errorText = emitter.error();
            return false;
        }
        if (optLevel >= 1) peephole(lines);
        out << "\n";
        for (const std::string &text : lines) out << text << "\n";
    }

    for (const std::unique_ptr<ir::GlobalVariable> &global : module.getGlobals()) writeGlobal(out, global.get());

    pool.emit(out);

    //the function irgen lists in @llvm.global_ctors runs before main
    if (module.globalInit) {
        out << "\n\t.section\t.init_array,\"aw\",@init_array\n";
        out << "\t.p2align\t3\n";
        out << "\t.quad\t" << symbolName(module.globalInit) << "\n";
    }

    //no executable stack
    out << "\n\t.section\t.note.GNU-stack,\"\",@progbits\n";
    return true;
}

void AsmWriter::writeGlobal(std::ostream &out, const ir::GlobalVariable *global) {
    const sema::Type *type = global->valueType;
    std::string       name = symbolName(global);
    uint64_t          size = layout.sizeOf(type);
    bool              zero = global->initializer == nullptr || isAllZero(global->initializer);

    if (global->isConstant) {
        out << "\n\t.section\t.rodata\n";
    } else if (zero) {
        out << "\n\t.bss\n";
    } else {
        out << "\n\t.data\n";
    }
    if (global->linkage == ir::Linkage::External) out << "\t.globl\t" << name << "\n";
    out << "\t.p2align\t" << log2Of(layout.alignOf(type)) << "\n";
    out << "\t.type\t" << name << ",@object\n";
    out << name << ":\n";
    if (zero) {
        out << "\t.zero\t" << (size == 0 ? 1 : size) << "\n";
    } else {
        writeConstant(out, global->initializer, type);
    }
    out << "\t.size\t" << name << ", " << size << "\n";
}

//one initializer as data directives, records padded between fields as the layout says
void AsmWriter::writeConstant(std::ostream &out, const ir::Constant *constant, const sema::Type *type) {
    switch (constant->constKind) {
    case ir::ConstantKind::Int:
        switch (widthOf(type)) {
        case Width::Byte: out << "\t.byte\t" << (constant->intValue & 0xFF) << "\n"; break;
        case Width::Word: out << "\t.short\t" << (constant->intValue & 0xFFFF) << "\n"; break;
        case Width::Long: out << "\t.long\t" << constant->intValue << "\n"; break;
        case Width::Quad: out << "\t.quad\t" << constant->intValue << "\n"; break;
        }
        return;
    case ir::ConstantKind::Float:
        out << (isDoubleType(type) ? "\t.quad\t" : "\t.long\t") << floatBits(constant->floatValue, isDoubleType(type))
            << "\n";
        return;
    case ir::ConstantKind::Null:
        out << "\t.quad\t0\n";
        return;
    case ir::ConstantKind::Address:
        out << "\t.quad\t" << symbolName(constant->target) << "\n";
        return;
    case ir::ConstantKind::ZeroInit: {
        uint64_t size = layout.sizeOf(type);
        if (size != 0) out << "\t.zero\t" << size << "\n";
        return;
    }
    case ir::ConstantKind::String:
        out << "\t.ascii\t\"" << escapeAscii(constant->bytes) << "\"\n";
        return;
    case ir::ConstantKind::Aggregate: {
        const sema::ArrayType *array = sema::asArray(type);
        uint64_t               offset = 0;
        for (size_t i = 0; i < constant->elements.size(); i++) {
            const sema::Type *elementType = array ? array->element : layout.elementType(type, i);
            uint64_t          start = array ? i * layout.sizeOf(array->element) : layout.elementOffset(type, i);
            if (start > offset) out << "\t.zero\t" << start - offset << "\n";
            writeConstant(out, constant->elements[i], elementType);
            offset = start + layout.sizeOf(elementType);
        }
        uint64_t size = layout.sizeOf(type);
        if (size > offset) out << "\t.zero\t" << size - offset << "\n";
        return;
    }
    }
}

} // namespace codegen
