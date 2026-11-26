
#ifndef IR_IR_TYPES_H
#define IR_IR_TYPES_H

#include <string>

#include "semantic/symbols.h"
#include "semantic/types.h"

namespace ir {

std::string formatType(const sema::Type *type);

std::string formatRecordName(const sema::RecordSymbol *record);

std::string formatRecordBody(const sema::RecordSymbol *record);

std::string closureTypeName(const ast::LambdaExpr *lambda);
std::string formatClosureBody(const ast::LambdaExpr *lambda);

std::string lambdaFunctionName(const ast::LambdaExpr *lambda);

std::string formatFloatLiteral(double value, bool isFloat32);

std::string escapeLLVMString(const std::string &bytes);

unsigned integerBits(const sema::Type *type);

bool isPassedByAddress(const sema::Type *type);

long long normalizeInt(long long value, const sema::Type *type);

}

#endif
