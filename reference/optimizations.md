# Optimizations: the -O0 … -O3 pipeline of this compiler

This document describes the final optimization setup level by level. For each pass it covers:
- what the pass does;
- a small example in our IR style;
- the Dragon Book section it comes from, or "not in the book";
- the analyses it needs;
- what keeps it correct;
- why it sits where it does.

All section numbers refer to *Compilers: Principles, Techniques, and Tools*, 2nd ed. (the PDF in this folder), and were checked against it.

**Out of scope:** partial-redundancy elimination / lazy code motion (§9.5), and loop fusion and fission (§11.7.8).

---

## 0. Overview

```
-O0   simplify_cfg → loop_rotate → codegen
-O1   O0 → inline → local_dag → simplify_cfg
      → codegen + peephole                     (linear register allocation)
-O2   O1 → mem2reg → instsimplify
      → loop_simplify → licm → loop_unswitch
      → simplify_cfg → instsimplify → cse → dce → sink
      → codegen (φ elimination inside codegen) + peephole   (linear register allocation)
-O3   O2 + iv_strength_reduce (after licm), with codegen changed:
      φ elimination into copies + graph-colouring register allocation
```

The levels are **cumulative**: each one runs everything in the level below, then adds its own passes.
- **-O0 and -O1** work on the **alloca form**, which is irgen's output. Every variable is an `alloca`, read with `load` and written with `store`, and the `%tN` temporaries live inside one block.
- **-O2 and -O3** first convert to **SSA** with mem2reg. They stay in SSA until codegen.

| Level | IR form | Analyses that must exist |
|---|---|---|
| O0 | alloca | dominators (§9.6.1), natural loops (§9.6.6), liveness (§9.2.5) |
| O1 | alloca | + call graph, escape scan |
| O2 | SSA | + dominance frontiers, def-use chains, available expressions (§9.2.6), loop invariance, induction variables |
| O3 | SSA, then machine IR | + interference graph |

Terms:
- **Local** means within one basic block.
- **Global** means across the whole CFG of one function.
- **Dense** analyses keep a fact for every (program point, variable) pair.
- **Sparse** analyses keep a fact per SSA value and pass it along def-use edges.

---

## 1. -O0: CFG cleanup, rotation, codegen

### 1.1 simplify_cfg (§8.7.2, §8.7.3)
It applies these rules over and over until none of them applies:
1. **Constant branches:** `br i1 true, label %a, label %b` becomes `br label %a`. irgen emits these for `until(true)` and `do … while(false)`.
2. **Unreachable blocks:** blocks with no path from entry are deleted (§8.7.2).
3. **Jump threading through empty blocks** (§8.7.3): a block that contains only `br label %L2` is bypassed, so its predecessors branch to `%L2` directly.
4. **Block merging:** if B has one successor S, and S has one predecessor B, append S to B and drop the `br`. This makes blocks bigger, which helps every local pass and the local register allocator.

- **On SSA (it runs again at -O2):**
  - deleting an edge must also delete the matching φ entry;
  - bypassing an empty block must retarget the φ entries that named it;
  - merging must replace S's single-entry φ nodes with their incoming value.
- **Analyses needed:** predecessors and successors (`recomputeCFG`), and reachability.
- **Why first:** every later analysis (dominators, loops, φ placement) should only see reachable blocks and real edges.

### 1.2 Loop canonical form (needed by rotation, and by every loop pass at -O2)
Loops are found as **natural loops** (§9.6.6): a back edge `t → h` where `h` dominates `t`, plus every block that can reach `t` without passing through `h`. Each loop is then put into standard shape:
1. **Preheader:** a single block outside the loop that is the header's only outside predecessor and whose only successor is the header.
2. **Single latch:** only one back edge. Several back edges are redirected to one new latch block.
3. **Dedicated exits:** every exit block has predecessors only inside the loop. Edges that violate this are split.

On the alloca form these are pure CFG edits, because there are no φ nodes to repair. At -O2, `loop_simplify` (§3.2) re-establishes the same shape on SSA.

### 1.3 Loop rotation (not in the book)
This turns a loop that tests at the top (`for`, `while`) into a guarded loop that tests at the bottom:
```
; while (i < n) { body; i++; }
before:                                after:
pre:   br cond                         pre:     %c0 = icmp slt …  ; br %c0, body.ph, exit   ← guard (copied test)
cond:  %c = icmp slt …                 body.ph: br body
       br %c, body, exit               body:    … i++ … %c = icmp slt …
body:  … i++ … br cond                          br %c, body, exit                           ← test at the bottom
```
What it gives:
- **One branch per iteration instead of two:** a conditional branch at the bottom, instead of a conditional branch at the top plus a `jmp` back.
- **LICM can hoist more:**
  - the body now dominates the latch;
  - the preheader runs only when the loop runs at least once;
  - so at -O2, instructions that can trap (`sdiv`, `srem`) can also be hoisted into the guarded preheader.
- **The loop is in the shape the book uses** for repeat-loops (Fig. 10.16(b)).

- **How to do it on the alloca form:** copy the header's instructions into the guard with fresh `%t` names. They are loads and compares on temporaries that stay inside the block, so nothing needs repair. (On SSA it would need exit φ nodes. That is why rotation is placed before mem2reg.)
- **Safety:**
  - Only rotate if the header has no side effects, i.e. no calls or stores.
  - Keep the header small, since it gets duplicated.
  - irgen's `do`/`until` loops are already rotated; skip them.
- **Analyses needed:** dominators, natural loops, canonical form (§1.2).

### 1.4 Codegen at -O0
This is the existing backend, described in Ch. 8:
- next-use information (§8.4.2);
- `getReg`, with register and address descriptors (§8.6);
- live-out values written to their home slots at each block end;
- caller-saved registers spilled before each call.

There is no peephole pass at this level.

---

## 2. -O1: inlining and local optimizations (no SSA)

### 2.1 Inlining (§12.1; also §1.5.1)
Inlining replaces a call with a copy of the callee's body. Removing the call overhead is a small gain. The bigger gain is that the callee's code can now see the caller's constants, so the DAG pass and simplify_cfg (and, from -O2, mem2reg and instsimplify) can work across what used to be a call.

How to do it on the alloca form:
1. **Choose call sites:**
   - the call is *direct* and the callee is defined in this module;
   - the callee is not recursive, i.e. it is not in a strongly connected component of the call graph with the caller;
   - the callee is below a size threshold, or has only one call site.
   - Never inline:
     - virtual and indirect calls;
     - declarations such as `printf`;
     - variadic functions.
2. **Split the caller's block at the call:** the part before the call branches to the cloned entry block; the part after it becomes a *continuation* block.
3. **Clone the callee's blocks.** Map each formal parameter to the actual argument, and give the copied values and labels fresh names.
4. **Move the callee's `alloca`s into the caller's entry block,** so that a call inside a loop does not grow the stack on every iteration.
5. **Replace the returns:** each `ret v` becomes `store v, %ret.addr` followed by `br continuation`. The call's result becomes `load %ret.addr`.

- **Order:** work bottom-up over the call graph, callees before callers, so that each body you copy is already optimized.
- **Analyses needed:** a call graph and its strongly connected components (§12.1), and cloning with a map from old values to new ones.
- **Afterwards:** inlining leaves `br` chains and new constant arguments behind. The local DAG pass and simplify_cfg that follow clean these up.

### 2.2 local_dag: the DAG of a basic block (§8.5.1–8.5.7)
One pass per block builds the DAG with **value numbering** (§6.1.2): before creating a node `op(L, R)`, look up `(op, vn(L), vn(R))` in a hash table. Several optimizations come out of the same structure:

| Inside local_dag | Book | Example |
|---|---|---|
| Constant folding | §8.5.4 | `sub i32 0, 1` → `-1`; `icmp slt 3, 5` → `true` |
| Algebraic identities | §8.5.4 | `x+0`, `x*1` → `x`; `x*0`, `x-x` → `0` (integers only) |
| Local strength reduction | §8.5.4, §8.7.4 | `x*2^k` → `x << k`; `x*2` → `x+x` |
| Commutativity | §8.5.4 | `b+c` and `c+b` get the same node |
| Local CSE | §8.5.2 | a second `a-d` reuses the first node |
| Local DCE | §8.5.3 | a root with no live use is removed |
| Rebuilding the block | §8.5.7 | instructions are emitted again from the DAG in a valid order |

**Memory in the alloca form (§8.5.5, §8.5.6).** Each `load`/`store` is a node, and some instructions *kill* earlier nodes:
- `store v, %x` kills the loads of `%x`. After it, a `load %x` *is* `v`: this is **store-to-load forwarding**.
- Two loads of `%x` with no kill between them give one node: **redundant load elimination**.
- Two stores to `%x` with no load of `%x` or kill between them make the first store dead: **local dead-store elimination**.
- A store through an unknown pointer, or a `call`, kills every load of *escaped* memory: globals, and allocas whose address is passed somewhere or stored. Allocas that never escape survive.

```
%t0 = load i32, ptr %i                %t0 = load i32, ptr %i
%t1 = add i32 %t0, 1                  %t1 = add i32 %t0, 1
store i32 %t1, ptr %i         →       store i32 %t1, ptr %i
%t2 = load i32, ptr %i                %t3 = mul i32 %t1, %t1
%t3 = mul i32 %t2, %t2
```

- **Safety:**
  - Fold exactly as the machine computes, with integers wrapping at their width.
  - Never fold division by zero or `INT_MIN / -1`.
  - Fold floats in their own precision.
  - Most identities do not hold for floats: `x*0.0` is not 0 when x is NaN, and `x+0.0` is not x when x is -0.0.
  - Signed `x / 2^k` is **not** `x >> k` (for example `-7/2 = -3` but `-7>>1 = -4`).
  - The book's footnote on p. 536 says it directly: compile-time evaluation must match run time.
- **Analyses needed:** none for the block itself, plus one scan of the function to find which allocas escape.
- It runs once at -O1, right after inlining. It used to run a second time to fold what dense constant propagation substituted; that pass is gone (§2.3).

### 2.3 const_prop_dense (removed)
**Why it was removed:** measured over the 46 regression programs, without it -O1 is 2.2% larger in static instructions (+599 of 27742; 36 programs grow, none shrink) and -O2 and -O3 0.1% (+14 of 24084 and +14 of 17322, in 4 programs). At -O2 and above, mem2reg turns the variables it tracked into SSA values and instsimplify folds them, so the pass found almost nothing there; its gain was confined to -O1, which did not justify a third global analysis in that group. -O1 is now inline → local_dag → simplify_cfg, and local_dag still folds constants, forwards stored values to later loads and removes dead stores within each block. `cprop_edges` checks its constants from -O2.

What it did, for reference: constant propagation on the alloca form, using the iterative framework (§9.3):
- **Domain:** each block's IN and OUT map every *non-escaped* scalar alloca to a lattice value: ⊤ (no information yet), a constant c, or ⊥ (NAC, not a constant).
- **Meet** at block entry (§9.4.2):
  - ⊤ ∧ c = c;
  - c ∧ c = c;
  - c ∧ d = ⊥ when c ≠ d;
  - ⊥ ∧ anything = ⊥.
- **Transfer** (§9.4.3): walk the block's instructions in order.
  - `store c, %x`: x becomes c.
  - `store %t, %x`: x becomes the value of `%t`. Evaluate `%t` from the current lattice: loads and constants are known, so fold arithmetic whose operands are all constant.
  - A call does not affect non-escaped allocas.
- **Boundary:** at entry every variable is ⊤ (uninitialized), and parameter slots start as ⊥ after their incoming store.
- **Rewrite:** each `load %x` where IN says x = c is replaced by c. The second local_dag then folds what that exposes, and simplify_cfg removes the branches that became constant.
- The framework is monotone but **not distributive** (§9.4.4–9.4.5). The result is safe, but it can miss constants that only a path-by-path analysis would find.
- **Analyses needed:** the §9.3 framework (`dfa_framework`), and the escape scan. mem2reg uses the same "can be promoted" test, so share it.

### 2.4 simplify_cfg (again)
This run folds the branches local_dag made constant, removes blocks that are now unreachable, and merges the blocks inlining left behind (§1.1).

### 2.5 Codegen at -O1: linear register allocation and peephole
- **Register allocation:** the book's "linear" allocator is the §8.6 simple code generator. It makes one forward pass over each block, keeps register and address descriptors, and picks registers with `getReg` (§8.6.3), guided by next-use information (§8.4.2). (The *linear-scan* allocator of Poletto and Sarkar is **not in the book**; see Suggestions.)
- **Peephole** (§8.7), over the emitted instructions:
  - **Redundant loads and stores (§8.7.1):** `movq %rax, -8(%rbp)` followed by `movq -8(%rbp), %rax` drops the load.
  - **Unreachable code (§8.7.2):** code after a `jmp` that no label leads to.
  - **Flow of control (§8.7.3):**
    - `jmp L1; … L1: jmp L2` becomes `jmp L2`;
    - a `jmp` to the next label is dropped.
  - **Algebraic simplification (§8.7.4):** `addq $0` and `imulq $1` are dropped.
  - **Machine idioms (§8.7.5):**
    - `movq $0, %r` becomes `xorl %r32, %r32` (only when the flags are dead);
    - `cmpq $0, %r` becomes `testq %r, %r`;
    - `addq $1` becomes `incq`.
  - **Safety:** a pattern must never span a label.

---

## 3. -O2: SSA, global data-flow and loop optimizations

### 3.1 mem2reg (SSA construction)
- **Promotes** each scalar alloca whose only uses are loads and stores that use it as the address (the escape scan of §2.2).
- **Places φ nodes** at the iterated dominance frontier of each variable's store blocks.
- **Renames** by walking the dominator tree, keeping a stack of current values per variable.
- **Analyses needed:** dominators (§9.6.1) and dominance frontiers (by the Cooper–Harvey–Kennedy "runner" walk; not in the book).

### 3.2 sccp (removed)
Sparse conditional constant propagation (Wegman & Zadeck, not in the book) ran here until it was removed. What it alone could do is prove that a CFG edge never executes and leave it out of the φ meet, so it folded:
```
x = 1;  while (…) { if (x != 1) x = 2; … }     // x stays 1: the assignment never runs
sumTo(1) after inlining                          // a loop that provably runs once
```
Dense propagation (§2.3, also removed since) assumed every edge can execute and gave ⊥ in both cases.

**Why it was removed:** measured over the regression programs, -O2 without it is 1.2% larger in static instructions (+268 of 22483) and -O3 1.1% (+171 of 16138), all in 9 programs whose loops run once or never after inlining; the other 43 compile to identical code. Common constant branches are already folded by local_dag and simplify_cfg at -O1, and at -O2 mem2reg and instsimplify collapse a φ whose other entries came from paths simplify_cfg deleted. That gain did not justify the hardest analysis in the -O2 group, so the -O2 pipeline goes from mem2reg straight to instsimplify.

### 3.3 instsimplify: algebraic simplification on SSA (§8.5.4, §8.7.4)
This applies the identities from §2.2, now one instruction at a time on SSA, using def-use chains to look at an operand's defining instruction:
- `sext(sext x)` becomes `sext x`;
- `select true, a, b` becomes `a`;
- `icmp eq x, x` becomes `true` for integers;
- `(x + c1) + c2` becomes `x + (c1+c2)`.

**Optional extension: reassociation** (§8.5.4, "Associative laws").
- Give every value a *rank*: constants 0, arguments 1, 2, …, and each instruction the reverse-postorder number of its block.
- Flatten chains of the same integer `add`/`mul`/`and`/`or`/`xor`.
- Sort the operands by rank and rebuild the chain.

Effects:
- constants come together;
- `a+b+c` and `c+b+a` become the same expression, so CSE catches them;
- loop-invariant operands (low rank) are grouped together, so LICM can hoist them as one unit.

Integers only: float arithmetic is not associative.

It runs **twice**:
- after mem2reg, to tidy up before loop analysis;
- after the loop passes, to fold what they exposed.

### 3.4 Loop passes
All loop passes inherit from the `LoopPass` base class:
- it finds the natural loops (§9.6.6);
- it visits them innermost first;
- it gives each pass the dominators, def-use chains and loop structure.

**loop_simplify:** re-establishes the shape from §1.2 on SSA, and adds **LCSSA**. LCSSA means every value defined inside the loop and used outside it goes through a φ in the exit block:
```
exit:  %i.lcssa = phi i32 [ %i, %latch ]
```
- Creating a preheader moves the header's outside φ entries into it, merging them with a new φ if there were several.
- LCSSA is what makes unswitching manageable: copying a loop only adds entries to these exit φ nodes.

**licm: loop-invariant code motion** (§9.1.7; the invariance algorithm is from the 1st ed. §10.7, simplified by SSA).

An instruction is **invariant** when:
- it is pure: a binary operation, compare, cast, select or GEP; never a φ, load, store or call;
- every operand is a constant, an argument, a global, defined outside the loop, or itself invariant.

Repeat the marking until nothing changes. Then move the invariant instructions to the end of the preheader, visiting blocks in dominator-tree order. SSA stays valid and no φ changes are needed, because the preheader dominates every use.

**Rules:**
- `sdiv`/`srem` may only be hoisted when their block dominates all exits. Thanks to rotation (§1.3) this is now true for the whole body of most loops.
- Loads are never hoisted, since there is no alias analysis.

**iv_strength_reduce: induction variables** (§9.1.8, with the affine view of §9.8). **Runs at -O3 only; see "Why not at -O2" below.**
- A **basic induction variable** is a header φ that changes by a constant each iteration: `%i = phi [init, pre], [%i + c, latch]`.
- A **derived induction variable** is `j = a*i + b`, where a and b are invariant.
- **Strength reduction** replaces the multiplication with a new induction variable that is updated by addition:
  ```
  %j = mul i32 %i, 4      →     pre:   %j0 = mul i32 %init, 4
                                 head:  %j  = phi [ %j0, pre ], [ %j.next, latch ]
                                 latch: %j.next = add i32 %j, 4
  ```
  The main beneficiary is array indexing: the GEP for `a[i]` computes `base + i*size`.
- **Safety:** keep the original exit test. Rewriting it in terms of `j` (induction-variable elimination) is only valid if `a*n + b` cannot overflow.
- If `i` ends up with no uses, DCE removes it later.

**Why not at -O2: the linear allocator cannot profit from it.**
- Each induction variable the pass creates is a new φ that lives across the whole loop.
- The -O2 backend allocates registers one block at a time (§8.6 `getReg`). Every value live at a block boundary is stored to memory and reloaded in the next block, and a φ's value goes through its stack slot on every iteration.
- So the multiply it removes (a `movslq` + `imul` for `a[i]`, both cheap) is replaced by a store and a reload on the loop-carried path every iteration.
- Measured on the regression programs and two benchmarks with -O2 as it was: +5% static instructions, +10% stack accesses, and an array loop 10% slower (0.200s → 0.221s). LICM looked costly too, but only because it hoists the array's base address, which is what makes the address computation qualify for strength reduction.
- The pass is kept and runs at -O3, where the graph-colouring allocator can keep a loop-carried induction variable in one register for the whole loop. Step 8 measures whether that pays off.

**loop_unswitch** (not in the book).

If the loop contains a branch on a **loop-invariant** condition, move the test in front of the loop and keep one copy of the loop for each direction:
```
for (…) { if (flag) A; else B; }    →    if (flag) for (…) A;  else for (…) B;
```
- **Trivial unswitching:** if one side of the branch *leaves* the loop (`if (flag) break;`), there is no need to copy the loop. Move the test into the preheader.
- **Full unswitching:**
  - copy the loop's blocks, using a map from old values to new ones;
  - fold the condition to `true` in one copy and `false` in the other;
  - add the copy's entries to the LCSSA exit φ nodes.

  The code size doubles each time, so unswitch at most one condition per loop, and only small loops.
- **Order:** after LICM, which is what shows the condition is invariant. Before simplify_cfg, which deletes the dead side in each copy.
- **Note:** unswitching changes the CFG, so the loop pass driver must recompute the loops after it.

### 3.5 simplify_cfg, instsimplify (again)
These clean up after unswitching (dead arms in each copy, empty blocks) and after LICM and induction-variable strength reduction (new constant expressions). simplify_cfg now has to maintain φ nodes (§1.1).

### 3.6 cse: global common subexpressions (§9.1.4, §9.2.6)
Available expressions is a forward problem with intersection as the meet. The expressions are pure `(op, type, operands)` tuples. In SSA the operands are never redefined, so **the kill sets are empty**.
- If `x op y` is available at an instruction that computes it again, replace that instruction with the earlier result.
- The earlier result must **dominate** the later use. Choose the dominating definition by walking the dominator tree with a scoped hash table.

Running after LICM and unswitching lets CSE merge duplicates that ended up next to each other in a preheader, or in both copies' shared code.

### 3.7 dce: dead-code elimination (§9.1.6)
A mark-and-sweep over def-use chains:
1. Mark every instruction with side effects: stores, calls, terminators.
2. Mark every instruction that a marked instruction uses, and repeat.
3. Erase everything left unmarked.

This removes:
- old induction variables left behind by strength reduction;
- φ nodes with no users;
- expressions that CSE made redundant.

### 3.8 sink: downward code motion (§10.4.3)
The opposite of LICM: move a pure instruction *down* to where it is used.
- If all of its uses are in one successor block, and that block has this block as its only predecessor, move the instruction to the top of that block. Repeat.
- A use by a φ counts as a use at the end of the corresponding predecessor.
- **Never sink into a loop.** That would turn one computation into one per iteration. Comparing loop depths prevents it.
```
entry: %t = mul %a, %b ; br %c, then, end   →   entry: br %c, then, end
then:  call @print(%t)                          then:  %t = mul %a, %b ; call @print(%t)
```
**Benefits:**
- Work is skipped on paths that never use the result. The book describes this as partial dead-code elimination when the destination does not postdominate the source.
- For our local allocator, fewer values are live across block boundaries, so fewer are written to their home slots.

It runs **last**, after LICM, so it cannot undo hoisting.

### 3.9 Codegen at -O2: φ elimination inside codegen
SSA is not converted back in a pass. The backend handles φ nodes itself, using the *φ slot plus entry copy* scheme:
- Each φ `p` gets a **φ slot**, separate from p's own home.
- At the end of each predecessor (after the end-of-block flush), the incoming value is stored into p's φ slot.
- At the top of p's block, p is defined by loading its φ slot. From that point p is an ordinary value: it gets a register through `getReg`, and its own home if it needs one.

Why a separate slot and the copy at block entry:
- **Lost-copy problem:** a value like `x` that is still used after the loop exits must not be overwritten by the next iteration's store.
- **Swap problem:** with `a = φ(b)` and `b = φ(a)`, every store reads the incoming values, never another φ's slot.

Neither case needs critical edges to be split.

Registers are allocated with the same linear (§8.6) allocator as -O1, followed by peephole.

---

## 4. -O3: graph colouring

-O3 runs the -O2 passes plus `iv_strength_reduce` (§3.4) after `licm`. Codegen changes as described below.

### 4.1 Machine IR with virtual registers (prerequisite)
Graph colouring (§8.8.4) uses **two passes**:
1. Select instructions "as though there are an infinite number of symbolic registers".
2. Assign physical registers.

**As built** (`codegen/mir.h`):
- isel emits **machine IR** (`MInst`) at every level. Each register operand states its role: read (`Use`), written (`Def`) or both (`UseDef`).
- Registers an instruction touches without naming them are listed as **implicit** uses and defs:
  - `idiv` reads and writes `rax` and `rdx`;
  - `cltd` reads `rax` and writes `rdx`;
  - a call reads its argument registers and overwrites every caller-saved register;
  - the jump to the epilogue reads the return register.
- One printer turns the machine IR into text. At -O0 to -O2 the registers are physical, and the output is byte-identical to the text isel wrote before the machine IR existed (checked on all 39 programs at the time).
- At -O3 every value gets a **virtual register** (`%v12.l`, `%x3` until colouring).

The allocator never guesses what an instruction reads or writes. An instruction without that information (a raw text line) at -O3 stops compilation with an error.

### 4.2 φ elimination into copies
Instead of using φ slots, each φ becomes **copies** on each incoming edge:
1. **Parallel copies through temporaries.** On one edge, every incoming value is first copied into a fresh register, and only then into its φ's register. All the φ nodes of a block are thus assigned at once, so `(a, b) := (b, a)` works (the swap problem). Coalescing removes the extra copies.
2. **Copies on the edge itself.** After a conditional jump, each side does its own copies: the fall-through side straight away, the taken side behind a local label. This is edge splitting without a new IR block, and it avoids the lost-copy problem. A switch case whose block has φ nodes jumps to such a label too.

### 4.3 Graph-colouring allocation (§8.8.4)
- **Interference graph:**
  - one node per virtual register;
  - an edge between two nodes when "one is live at a point where the other is defined" (§8.8.4, p. 557);
  - built from liveness (§9.2.5) computed on the machine IR.
- **k** = the number of allocatable registers in each class: integer and float are coloured separately.
- **Simplify** (the k-neighbour rule): a node with **fewer than k neighbours** can always be coloured, whatever its neighbours get. Remove it, push it on a stack, and repeat.
- **Select:** pop the nodes in reverse order of removal, giving each a colour (register) that none of its already-coloured neighbours uses.
- **Spill:** if every remaining node has ≥ k neighbours, choose one to spill (Chaitin's heuristic). A common choice is the lowest `spill cost / degree`, with spill cost = Σ (uses + defs) × 10^loop depth.
  - Rewrite the spilled register: a store after each definition, a load before each use, each with a short-lived new virtual register.
  - Rebuild the graph and start again.

### 4.4 What a working allocator needs beyond §8.8.4
- **Precoloured nodes:** every physical register the machine IR names is a node with a fixed colour, so all **14 general registers** (not `rsp`, `rbp`) and **16 xmm registers** can be handed out:
  - argument and return registers;
  - `rdx:rax` for `idiv`, and `cl` for shifts;
  - at every `call`, all caller-saved registers are written, so they interfere with every value live across the call. Integers that survive a call end up in `rbx`/`r12`–`r15` (saved in the prologue) or on the stack; doubles always go to the stack (System V has no callee-saved xmm register).
- **Coalescing**, in two phases:
  1. **Briggs**, virtual pairs first: the merged node has fewer than k neighbours of significant degree (≥ k).
  2. **George**, a virtual register into a physical one: every neighbour of the virtual register is precoloured, of low degree, or already interferes with the physical one.

  Merging into a fixed register first was measured to block the virtual merges behind it (a leftover copy and jump on a loop's back edge).
- **Callee-saved registers:** only the ones used are saved and restored.
- Each function's `.s` starts with a comment: `# graph colouring: N virtual registers, C copies coalesced, S spilled, R rounds`. The regression tests check it.

### 4.5 Results and the strength-reduction ablation (step 8)
Static totals over the 42 regression programs:

| Level | Instructions | Stack accesses |
|---|---|---|
| -O2 | 20058 | 6741 |
| -O3 | 14251 | 1125 |

Runtime, median of 11 interleaved runs:

| Benchmark | -O0 | -O1 | -O2 | -O3 without iv_strength_reduce | -O3 with |
|---|---|---|---|---|---|
| calls and division | 0.250 | 0.186 | 0.213 | 0.185 | 0.183 |
| array scan + float recurrence | 0.201 | 0.199 | 0.200 | 0.104 | 0.098 |
| three arrays, indexed loops | 0.327 | 0.261 | 0.207 | 0.190 | 0.149 |

`iv_strength_reduce` was moved to -O3 because the linear allocator could not profit from it (§3.4, "Why not at -O2"). With graph colouring it does profit:
- up to **22% faster** on the array benchmark, 6% on the scan, no change on the call-bound one;
- statically +0.9% instructions and +6% stack accesses, from the extra induction-variable registers.

Two fixes were needed before it paid off:
- the variable's update had been inserted between the loop's compare and branch, which stopped codegen from fusing them;
- the coalescer's order, see §4.4.

---

## 5. Why passes are in this order

1. **Inlining comes first among the real optimizations.** It creates the constants and the dead code that everything after it removes.
2. **Rotation happens before SSA.** It is a trivial CFG edit there, and by the time LICM runs at -O2 the loops already have guarded preheaders.
3. **Each big transform is followed by a cleanup.**
   - inline → local_dag;
   - const_prop → local_dag + simplify_cfg;
   - loop passes → simplify_cfg + instsimplify;
   - CSE → DCE.

   This is why local_dag, simplify_cfg and instsimplify each run more than once.
4. **Simplify before loop analysis.** instsimplify runs before the loop passes, so the invariance and induction-variable tests see the simplest code.
5. **Within the loop group:**
   - loop_simplify comes first, because the others need its shape;
   - LICM before unswitching, because unswitching needs the condition proven invariant;
   - strength reduction after LICM, because the step and initial values must already be invariant and hoisted.
6. **Move code up before moving it down:** LICM early, sink last.
7. **DCE runs late,** after everything that leaves dead code behind.
8. **φ elimination and register allocation come last,** inside codegen. Peephole runs on the final instructions.

---

## 6. Summary

| Pass | Level | Book | Scope | Analyses needed |
|---|---|---|---|---|
| simplify_cfg | O0+ | §8.7.2–8.7.3 | global | predecessors and successors, reachability |
| loop canonical form | O0+ | §9.6.6 (natural loops) | loop | dominators, loops |
| loop_rotate | O0+ | not in book | loop | dominators, loops |
| linear allocation (`getReg`) | O0–O2 | §8.4.2, §8.6 | local | next-use, liveness |
| inline | O1+ | §12.1 | interprocedural | call graph, strongly connected components |
| local_dag | O1+ | §8.5.1–8.5.7, §6.1.2 | local | escape scan |
| peephole | O1+ | §8.7 | machine | none |
| mem2reg | O2+ | §6.2.4 (SSA) | global | dominators, dominance frontiers |
| instsimplify (+ reassociation) | O2+ | §8.5.4, §8.7.4 | per instruction | def-use, reverse postorder |
| loop_simplify + LCSSA | O2+ | §9.6.6 | loop | dominators, loops |
| licm | O2+ | §9.1.7 (1st ed. §10.7) | loop | loops, dominators |
| iv_strength_reduce | O3 | §9.1.8, §9.8 | loop | loops, def-use |
| loop_unswitch | O2+ | not in book | loop | invariance, LCSSA, cloning |
| cse | O2+ | §9.1.4, §9.2.6 | global | available expressions, dominator tree |
| dce | O2+ | §9.1.6 | global | def-use |
| sink | O2+ | §10.4.3 | global | dominators, def-use, loop depth |
| φ elimination (slots) | O2 | — | codegen | — |
| φ elimination (copies) + coalescing | O3 | Fig. 9.31 (edge splitting) | codegen | liveness |
| graph colouring | O3 | §8.8.4 | function | interference graph, machine IR |

---

## 7. Suggestions and improvements

1. **The pass manager needs a module-pass phase for inlining.**
   - `PassManager::run` (`src/passes/pass_manager.cpp`) is *function-major*: it runs every pass on one function, then moves to the next.
   - Inlining reads other functions and must visit them bottom-up over the call graph, so it cannot be a `FunctionPass`.
   - Suggestion: a `ModulePass` base class, and a pipeline made of *phases*:
     1. per-function passes for -O0;
     2. a module phase: inline;
     3. per-function passes for -O1 and -O2.
   - This also guarantees that every callee has been through simplify_cfg and rotation before it is copied into its callers.

2. **Rotation at -O0 changes the baseline, and pulls loop analysis into -O0.**
   - -O0 now needs dominators, natural loops and loop canonical form. In the current plan those arrive with step 3 and step 5.
   - -O0 also stops being "irgen output, unchanged".
   - To keep a trustworthy reference for testing, compare every level's program output with the `.ll` compiled by clang. The scratchpad harness already does this. Do not compare against our own -O0.

3. **"Linear register allocation as defined in the book."** The book has no *linear-scan* allocator. Its linear, one-pass allocator is §8.6 (`getReg`), which -O0 already uses. Choose one of these readings:
   - **(a) -O1 and -O2 keep the §8.6 allocator.** Then the only codegen difference between -O0 and -O1 is peephole. This matches the book most closely.
   - **(b) Add global allocation by usage counts (§8.8.2–8.8.3) at -O1 and -O2.**
     - Benefit formula 8.1: `Σ_B use(x,B) + 2·live(x,B)`.
     - Keep the values with the highest benefit inside each loop in a few reserved callee-saved registers for the whole loop.
     - It is in the book, it fits the current `reg_alloc`, and it is what makes LICM pay off: a hoisted invariant stays in a register instead of being reloaded from its home slot in every block.
   - **(c) Poletto–Sarkar linear scan.** Not in the book. It needs live intervals over a linear order of the instructions.

   I recommend (b) for -O2, keeping (a) at -O1.

4. **There is no global constant propagation.** After mem2reg only instsimplify folds constants, one instruction at a time, and at -O1 only local_dag, one block at a time. If a later pass (unrolling, see 7) creates constants that must flow across blocks, a sparse propagation over def-use chains without the conditional part is the simple way to add it back.

5. **Share code between passes.**
   - The escape test ("this alloca's address never leaves load/store") is needed by local_dag and mem2reg. Write it once.
   - The constant folder is needed by local_dag and instsimplify, and must give exactly the same results in both. One `fold(op, type, a, b)` helper.
   - Loop cloning (blocks plus a map from old values to new ones) is needed by inlining and unswitching. One utility.

6. **The `LoopPass` contract has to change.** The plan says `runOnLoop` must not change the CFG. Rotation and unswitching do change it. Options:
   - have `runOnLoop` return "CFG changed", after which the driver recomputes the loops and starts over;
   - or make these two `FunctionPass`es that use the loop analysis directly.

7. **Unrolling is no longer in the pipeline. It would be cheap to add later.** Unswitching already needs loop cloning and LCSSA, and strength reduction already finds the induction variables. With both in place, **full unrolling of loops with a small constant trip count** (§10.4 p. 735, Fig. 10.16) needs little new code. After unrolling, every copy's induction variable is a constant expression that instsimplify folds. It would fit at -O3 after unswitching, followed by `simplify_cfg / instsimplify`.

8. **Rotation, then LICM, gives a measurable demonstration.** The `sdiv` example from the plan's step-5 test can be extended: show the same loop at -O2 with rotation (division hoisted into the guarded preheader) and without it (division stays in the loop). It is a clear case for the report.

9. **Graph colouring is the biggest single piece of work.** -O3 requires all of these:
    - a machine IR with virtual registers;
    - isel rewritten to emit it, because today every result goes through `rax` and is allocated immediately;
    - liveness on the machine IR;
    - precoloured registers;
    - spill-code rewriting;
    - conservative coalescing.

    Build it as a separate backend path, selected only at -O3, and keep the §8.6 path unchanged for -O0 to -O2. That way a bug in the colouring allocator cannot break the lower levels, and both paths can be checked against each other.

10. **Testing for each level.** For every level in `run.sh`:
    - check `<stem>.opt.ll` with clang's verifier (for -O1 and above);
    - link and run `.s`, and compare with `test/expected`.

    Also add per-pass trace tests (`PassManager::setTrace`) on small programs that target each pass:
    - an inlined constant argument;
    - a store followed by a load in one block;
    - an invariant `mul` in a loop;
    - `a[i]` in a loop for strength reduction;
    - an invariant `if` for unswitching;
    - a value used on only one branch, for sink.
