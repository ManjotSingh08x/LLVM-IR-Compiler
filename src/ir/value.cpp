#include "ir/value.h"

namespace ir {

std::string Constant::getOperand() const {
    switch (constKind) {
    case ConstantKind::Int:
        if (integerBits(type) == 1) return intValue != 0 ? "true" : "false";
        return std::to_string(intValue);

    case ConstantKind::Float:
        return formatFloatLiteral(floatValue, type->is(sema::TypeKind::Float));

    case ConstantKind::Null:
        return "null";

    case ConstantKind::ZeroInit:
        return "zeroinitializer";

    case ConstantKind::String:
        return escapeLLVMString(bytes);

    case ConstantKind::Aggregate: {
        //arrays are written [ i32 1, i32 2 ], records { i32 1, i32 2 }
        bool isArray = type->is(sema::TypeKind::Array);
        std::string out = isArray ? "[" : "{ ";
        for (size_t i = 0; i < elements.size(); i++) {
            if (i > 0) out += ", ";
            out += elements[i]->getTypedOperand();
        }
        out += isArray ? "]" : " }";
        return out;
    }

    case ConstantKind::Address:
        return target->getOperand();
    }
    return "";
}

void GlobalVariable::dump(std::ostream &out) const {
    out << "@" << name << " = ";
    switch (linkage) {
    case Linkage::External:
        break;
    case Linkage::Internal:
        out << "internal ";
        break;
    case Linkage::Private:
        out << "private unnamed_addr ";
        break;
    }
    out << (isConstant ? "constant " : "global ") << formatType(valueType) << " "
        << initializer->getOperand() << "\n";
}

} // namespace ir
