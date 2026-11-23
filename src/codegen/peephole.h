#ifndef CODEGEN_PEEPHOLE_H
#define CODEGEN_PEEPHOLE_H

#include "codegen/x86.h"

namespace codegen {

//peephole optimization (dragon book 8.7) over one function's emitted instructions, repeated until
//nothing changes. no pattern looks across a label, another block may jump in there
//  redundant loads and stores (8.7.1): mov R, M then mov M, R drops the reload; mov M, R then
//    mov R, M drops the store back; mov A, B then mov B, A drops the copy back; mov R, R goes
//  unreachable code (8.7.2): instructions after jmp, ret or ud2 up to the next label
//  flow of control (8.7.3): a jump to a jmp goes straight to its target, a jmp to the next label goes
//  algebraic simplification (8.7.4): add/sub $0 and imul $1 go
//  machine idioms (8.7.5): mov $0, R -> xor R, R; add/sub $1 -> inc/dec; cmp $0, R -> test R, R
//the rewrites that change the flags (xor, inc, dec, dropping add $0) are made only when the next
//instruction that cares about the flags sets them again first
void peephole(AsmLines &lines);

} // namespace codegen

#endif
