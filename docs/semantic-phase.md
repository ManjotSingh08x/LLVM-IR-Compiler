# Semantic Phase: Symbol Tables and Type Checking

How symbol table building and type checking work in this compiler, the
decisions behind them, and the edge cases that shaped the AST. Written for
someone who has read the AST headers but not the semantic ones.

IR generation is deliberately not part of this phase.

---

## 1. What runs, in order

The analyzer has four phases, and a file must clear each one before the next
runs. A semantic error exits non-zero exactly as a syntax error does.

1. **Scanner** (`lexer.l`) produces tokens and the token table.
2. **Parser** (`parser.y`) builds the AST. It stops here on syntax errors.
3. **SymbolBuilder** (`semantic/symbolbuilder.*`) builds the scope tree,
   declares every name, resolves every written type, assigns mangled names.
4. **TypeChecker** (`semantic/typecheck.*`) types every expression, checks
   every use, and writes conversions back into the tree.

Files added in this phase:

| File | Holds |
| --- | --- |
| `semantic/visitor.h` | the shared tree visitor, moved here from `ast/` |
| `semantic/types.h` / `.cpp` | the recursive type system, its helpers, LLVM lowering |
| `semantic/symbols.h` / `.cpp` | symbols, the scope tree, the owning `SymbolTable`, the dump |
| `semantic/symbolbuilder.h` / `.cpp` | pass 1: declare names, resolve types, mangle |
| `semantic/typecheck.h` / `.cpp` | pass 2: type and check expressions |
| `semantic/constants.h` / `.cpp` | literal values and integer constant expressions |
| `semantic/diagnostics.h` | the error list, sorted into source order |
| `declarator.h` | parser-only declarator holders, never in the tree |

Every accepted file writes two files beside the token table:
`outputs/<name>-ast.txt`, the tree including the inserted conversions, and
`outputs/<name>-symbols.txt`, the scope tree with each symbol's full type,
mangled name and LLVM type.

The visitor moved out of `ast/` because it is the mechanism the semantic passes
are built on; `ast_printer` is a debugging tool that happens to use the same
mechanism, so it now includes `semantic/visitor.h`. The visitor's return type
is a template parameter, which is what lets the printer return `void` and the
type checker return `const Type *` from the same dispatch.

---

## 2. The recursive type system

A compound type holds a pointer to the type it is built from. That is what
makes the struct self-recursive, and what lets any depth of declarator be
represented. `int **(*p)[10]` becomes a four-link chain:

```text
Pointer -> Array 10 -> Pointer -> Pointer -> Int
```

Read outwards from `Int`: a pointer, a pointer to that, an array of ten of
those, a pointer to that array. `typeToString` walks the same chain backwards
to print `int**(*)[10]`.

The kinds are `Void Bool Char Int Float Double`, three internal ones
(`Null Auto Error`), and the compound `Pointer Reference Array Function Record
Closure`.

### Why this is not llvm::Type

This was the question worth settling, since `LLVMContext` already owns a type
system and reusing it would mean less code. It cannot be done. Since LLVM 15 a
pointer is opaque: `llvm::PointerType` is built from a context and an address
space only, and `PointerType::get(Type *Elem, …)` discards `Elem`. Every
pointer is `ptr`. Had our types been LLVM's, the checker would lose the
information it exists to check:

| Written | As llvm::Type | What is lost |
| --- | --- | --- |
| `int*`, `int**`, `Dog*` | all `ptr` | `*p` and `p->x` have no result type |
| `int&` vs `int*` | both `ptr` | a reference cannot be told from a pointer |
| `bool`, `char` | `i1`, `i8` | nothing marks them as bool or char |
| `Dog` | a struct of unnamed fields | no member names, access, or base classes |

So `d = c` with `Dog *d` and `Cat *c` would compare equal and pass silently.
Our types carry the pointee; `toLLVM` discards it only at the last step, which
is right, because that is exactly what the IR does.

### What the helpers do

- `isSameType` compares structure recursively, and records by identity. Two
  `struct Point` declarations are two types; one `Point` used twice is one type.
- `decay` turns an array into a pointer to its first element and a function
  into a function pointer. Every operator except `&` does this to its operand.
- `commonArithmeticType` is the usual arithmetic conversion, narrowed to this
  subset: double, then float, then int.
- `findConversion` returns both a rank for overload resolution and the ordered
  list of casts to apply.
- `promote` is the integer promotion: `bool` and `char` become `int`.

---

## 3. Who owns what

The `SymbolTable` at the root of the scope tree owns both the scopes and every
compound type. There is no separate `TypeContext` class, and no uniquing.

```cpp
class SymbolTable {
    std::vector<std::unique_ptr<Type>> types;  // every compound type ever made
    std::unique_ptr<Scope>             root;   // owns its children recursively
};
```

The builtins (`int`, `char`, `bool`, `float`, `double`, `void`, null, auto,
error) are `static` objects in `types.cpp`. They need no owner, and `intType()`
and friends return the same pointer every time.

### Why an owner is needed at all

**Most types belong to no symbol.** The type checker creates them while
checking expressions:

- `&x` needs a fresh `Pointer(typeof x)`, and `&p` where `p` is `int **` needs
  `int ***`
- `this` needs `Pointer(Record Dog)`
- `"hello"` needs `Array(6, char)`
- an array decaying to a pointer needs that pointer type
- `a + b` and `cond ? x : y` need their converted result types
- a method's lowered signature needs a new `FunctionType` with `this` in front

These share their inner parts with symbol types — the pointee of `&x`'s type
*is* `x`'s type — so the alternatives all fail:

| Approach | Why not |
| --- | --- |
| each symbol owns its type by `unique_ptr` | breaks sharing; every expression would deep-copy types |
| `shared_ptr` | leaks on `struct Node { Node *next; }`, since the field type points back at the record |
| raw `new`, no owner | leaks everything |
| `llvm::FoldingSet` + `BumpPtrAllocator` | works, but it is uniquing machinery we do not need |
| `LLVMContext` owns them | impossible; it owns `llvm::Type`, not ours |

Per-scope ownership was considered and rejected: every scope lives until
compilation ends, so it would free nothing sooner, and it would mean threading
the right scope into every function that builds a type.

`LLVMContext` appears in exactly one place, `toLLVM`. The symbol dump creates a
throwaway one purely to show what each type lowers to, which also means every
test run exercises the lowering code even though no IR is emitted yet.

---

## 4. The parser rewrite

The old `Declarator` was flat and lived in the AST:

```cpp
struct Declarator {                // deleted
    int            pointerDepth;   // only ever 0 or 1
    bool           isReference;
    DeclaratorForm form;           // Plain | FunctionPointer | PointerToArray | ReferenceToArray
    NodeList<Expr> dims;
};
```

It could not express `int** (*p)[10]`: a single depth counter plus a four-way
enum has no room for stars on both sides of the parentheses. It is gone.
`VarDecl` and `ParamDecl` now hold a `TypeSpec` tree instead.

### TypeSpec is a syntax tree, not a sema::Type

`ast::TypeSpec` gained the wrapping kinds `Pointer`, `Reference`, `Array` and
`Function` alongside the three name kinds. It stays a *syntax* tree because at
parse time `Dog` is not resolved and `[N+1]` is still an expression. Each node
carries `const sema::Type *resolved = nullptr`, which the symbol builder fills.

### The hole trick

A declarator is read before its base type is known: in `int *p[3]`, the parser
reduces `*p[3]` while `int` is still on the stack. So a declarator builds its
own part of the chain with a **hole** — a null `inner` at the innermost end —
and the shared type name is plugged into that hole once the declaration reduces.

```text
*p[3]     builds  Array 3 -> Pointer -> (hole)
then plug int  =  Array 3 -> Pointer -> Int
```

`plug(chain, piece)` walks to the end of `chain` and attaches `piece`. One type
name serves several declarators because each gets `clone()` of it, so
`int a, *b, c[2];` is three chains sharing nothing. The helpers (`pointers`,
`arrays`, `functionOf`, `plug`, `addPointers`) live in `parser.y`, and the
holders they work on live in `declarator.h`, which is parser-only and never
reaches the tree.

Worth knowing: the array chain is built with the **first dimension outermost**,
so `[2][3]` is `Array 2 -> Array 3 -> T`. That is the order indexing needs —
`grid[1][2]` peels `Array 2` first.

### Grammar changes

`pointer` is now one-or-more stars, used in every declarator, parameter, return
type and `new` type. `paren_suffix` takes stars inside the parentheses, and
`addPointers` attaches those written before them, which is what
`int **(*p)[10]` needs. The grammar still has **zero conflicts**.

Fully nested C declarators such as `int (*(*f)(int))[3]` were deliberately not
added: the current grammar resolves the `a * b;` ambiguity with careful
precedence, and nesting would risk that for little gain in this subset.

A class type followed directly by a parenthesised declarator is offered only at
file scope, through `named_parenthesized_declaration`. Inside a class `Dog (`
would begin a constructor, and inside a block it has the shape of a call, so
`struct Dog (*dogs)[2]` is written there instead.

### _CLASSSPECIFIER removed

The old token table appended `_CLASSSPECIFIER` to a name whose declarator
carried `*`, `&` or `[]`, so `char *argv[]` reported `CHAR_CLASSSPECIFIER`.
Nothing in the AST or the semantic phase ever read it, and the
`pendName(index, stars, amps, dims)` bookkeeping existed only to feed it —
which was also the main obstacle to the rewrite. It is removed: `pendName(index)`
records just the token, a name reports its written type specifier
(`int **p` → `INT`), and full types live in the symbol dump instead.

---

## 5. The scope tree

```cpp
class Scope {
    ScopeKind kind;        // Global | Record | Function | Block | Lambda
    Scope    *parent;
    std::vector<std::unique_ptr<Scope>>    children;
    std::vector<std::unique_ptr<Symbol>>   symbols;  // declaration order
    llvm::StringMap<std::vector<Symbol *>> table;    // a vector per name, for overloads
    llvm::StringMap<ast::LabelStmt *>      labels;   // function and lambda scopes
};
```

Scopes are **persistent**: nothing is popped or discarded. This is why
`llvm::ScopedHashTable` was not used — it throws a scope away when you leave
it, and codegen will need to walk back in.

The tree nodes that open a scope point at theirs: `Program::scope`,
`FunctionDecl::scope`, `RecordDecl::scope`, `CompoundStmt::scope`,
`ForStmt::scope`, `LambdaExpr::scope`. That replaces the side map a
`DenseMap<ASTNode*, Scope*>` would have needed, and keeps the rule that
results live on the node.

`table` holds a *vector* per name because overloads share one name. Everything
else in a scope is one symbol per name, so the vector has one element.

### Two passes, because class bodies see everything at once

A method body can use a member declared below it:

```cpp
class Animal {
public:
    int readIt() { return secret; }   // uses secret, declared later
private:
    int secret;
};
```

So the builder runs twice over the file:

1. **Declare**: every record with all of its members, every function
   signature, every global. Mangled names are assigned here, which must come
   after the overload set is known.
2. **Bodies**: the scopes of every function, block and lambda, with their
   parameters and locals.

A record is also declared before its own members are, which is what makes
`struct Node { Node *next; };` work.

### Out-of-class definitions reparent

`void Dog::bark() { … }` written at file scope gets a function scope whose
**parent is `Dog`'s record scope**, not the global one. That single line is what
lets an unqualified `legs` inside the body find the inherited member:

```cpp
void Dog::bark() {
    legs = 4;   // found because the function scope's parent is Dog's
}
```

### Parameters and the body share one scope

`FunctionDecl::scope` holds the parameters, and the builder then sets
`node->body->scope` to the *same* scope rather than making a child. So a local
cannot quietly shadow a parameter:

```cpp
int f(int a) {
    int a = 2;   // error: redefinition of 'a'
}
```

A nested `{ }` does get its own child scope, where shadowing is legal.

### Declaration-order lookup

Since the whole table is filled before the checker runs, a naive lookup would
find the wrong `x` here:

```cpp
int x = 1;
{
    int y = x;      // must find the outer x
    int x = 2;
}
```

So every symbol stores its line and column, and lookups in block, function and
global scopes **skip symbols declared after the use**. Record scopes do not
skip, because of the class-body rule above. Builtins are stored at line 0,
which is always "before".

This is also why `int b = later; int later = 1;` reports *use of undeclared
identifier*, which is what C++ says too.

### Labels and goto

Labels go in a separate `labels` map on the function or lambda scope, not in
the symbol table, because `goto` may jump forward to a label the checker has
not reached yet. The checker therefore collects every `GotoStmt` in a list and
resolves them all in `resolveGotos` once the body is done, setting
`GotoStmt::target`.

---

## 6. Length-prefixed mangling

Each name part is written as its length followed by the name, and the parameter
types are always appended.

| Source | Mangled |
| --- | --- |
| `int add(int, int)` | `global_3add_ii` |
| `void Animal::setAge(int)` | `class_6Animal_6setAge_i` |
| `Animal()` and `Animal(int)` | `class_6Animal_C_v`, `class_6Animal_C_i` |
| `Animal::~Animal()` | `class_6Animal_D_v` |
| `int Outer::Inner::f()` | `class_5Outer5Inner_1f_v` |
| `static int Counter::total` | `class_7Counter_5total` |

Parameter type codes:

| Code | Type |
| --- | --- |
| `v b c i f d` | void, bool, char, int, float, double |
| `P<T>` | pointer to T |
| `R<T>` | reference to T |
| `A<n>_<T>` | array of n T |
| `F<ret><params>E` | function |
| `<len><name>` | a record |
| `N…E` | a nested record, wrapped so its parts cannot read as separate parameters |

Constructors and destructors use `C` and `D`. These cannot be confused with a
name part, because a name part always begins with a digit.

### What the plain scheme would have broken

The first proposal was `global_<name>` and `class_<class>_<method>`. It
collides three ways, all of which occur in the existing tests or are easy to
hit:

| Collision | Example |
| --- | --- |
| overloads | `Animal()` and `Animal(int)` both give `class_Animal_Animal` (test4 has exactly this) |
| underscores in names | class `A_B` method `c`, and class `A` method `B_c`, both give `class_A_B_c` |
| `main` and libc | `global_main` never links as an executable; the same for `printf` and `scanf` |

So `main`, `printf` and `scanf` keep their own names. As a backstop,
`setMangledName` keeps a map of every name assigned and reports an error if two
symbols ever land on the same one — a guard that should never fire, but which
turns a silent wrong-call bug in codegen into a message now.

---

## 7. Methods become global functions

The builder never rewrites the AST into global functions. Instead each
`FunctionSymbol` carries **two** signatures:

```cpp
const FunctionType *type;         // signature(): what callers are checked against
const FunctionType *loweredType;  // what codegen emits, with this prepended
```

For `void Animal::setAge(int)`:

- `signature()` is `void (int)` — the call `a.setAge(3)` checks against this
- `loweredType` is `void (Animal*, int)` — this is what the IR will define

A **static** method gets no `this`, so both are the same. A free function also
has both equal. The dump shows this:

```text
method setAge : void (int) public [class_6Animal_6setAge_i]
  this-lowered: void (Animal*, int) llvm: void (ptr, i32)
```

Static *data* members become globals with a mangled name
(`class_7Counter_5total`); non-static fields stay in `RecordSymbol::fields` in
layout order and get no mangled name, because they are reached by offset.

---

## 8. Edge cases and decisions written into the AST

The type checker's results live on the AST nodes, as side tables were ruled
out. Every field starts null or false and is filled only when the checker
visits. This section covers the ones that are not obvious.

### Expr gained two fields, not one

```cpp
class Expr : public ASTNode {
    const sema::Type *type = nullptr;
    bool              isLValue = false;
};
```

The type alone is not enough. `isLValue` is needed in four places that cannot
be answered from the type:

- `x = 1` is fine, `7 = 1` is not
- `&x` is fine, `&42` is not
- `x++` needs something that can be changed
- `int &r = x` needs `x` to name an object

### InitListExpr gained a constructor slot

This is the edge case that forced a field nobody would predict. A brace list
usually initialises an aggregate — but when the record has constructors, the
same syntax *runs one*:

```cpp
struct Point { int x; int y; };
class Dog { public: Dog(int n); };

Point p{1, 2};        // aggregate: fills x and y, no constructor
Dog   d{3};           // calls Dog(int)
Point *np = new Point{5, 6};   // aggregate, through new
Dog   *nd = new Dog{3};        // calls Dog(int), through new
```

Codegen has to tell these apart and cannot re-derive it without redoing
overload resolution, so `InitListExpr::constructor` records which constructor
ran, or stays null for an aggregate. `initFromList` decides: a record with a
non-empty `constructors` list treats the items as constructor arguments,
otherwise they fill `fields` in order.

`VarDecl::constructor` and `NewExpr::constructor` exist for the same reason,
and a brace-initialised `VarDecl` copies the list's answer up into its own slot
so codegen has one place to look.

### Four declaration forms reach the same constructor logic

`InitStyle` distinguishes them, and all four end at `construct()`:

```cpp
Dog a;           // None     -> default constructor
Dog b(3);        // CtorArgs -> Dog(int)
Dog c{3};        // Brace    -> Dog(int), via InitListExpr
Dog *d = new Dog(3);         // NewForm::Args
```

`None` also constructs **arrays of objects**: `Dog pack[4];` default-constructs
each element, so the checker walks down through the array type to find the
element record.

A record with **no** declared constructors has an implicit default and copy
constructor, which have no symbol. `construct()` returns null for those, which
is a legitimate answer, not a failure — so codegen must read null as
"zero-initialise or copy", not as "something went wrong".

### ImplicitCastExpr: conversions are nodes, not flags

Every silent conversion in the source becomes a real node wrapped around the
operand, Clang-style. Without this, codegen would have to recompute the same
conversions and could disagree with the checker.

| CastKind | When |
| --- | --- |
| `IntegralCast` | between bool, char, int |
| `IntegralToFloating`, `FloatingToIntegral`, `FloatingCast` | the number conversions |
| `ToBool` | a number or pointer used as a condition |
| `ArrayToPointer` | an array used as a value |
| `FunctionToPointer` | a function name used as a value |
| `NullToPointer` | `null` assigned to a pointer |
| `DerivedToBase` | a `Derived*` or `Derived` used as its base |
| `PointerToVoid` | an object pointer to `void *` |

test7 inserts 72 of them: 48 array-to-pointer, 12 integral, 4 to-bool, 4
floating, 2 null-to-pointer, 2 function-to-pointer. The printer shows them, so
`outputs/*-ast.txt` is a good way to see what the checker decided.

`DerivedToBase` is the one with work left for codegen: with multiple
inheritance the pointer must be adjusted to the base subobject. The checker
records that the conversion happens; the GEP is codegen's job.

### auto is a type, and deduction is a special case

`auto` is a real `TypeKind`, so a declaration can be built before its
initializer is checked. `checkVarDecl` then handles it before everything else,
because the normal path needs a known type:

```cpp
auto square = [](int x) { return x * x; };   // deduced: that lambda's closure type
auto n = 5;                                  // int
auto bad;                                    // error: no initializer
auto arr{1, 2};                              // error: cannot deduce from a brace list
```

Deduction applies `decay`, so `auto s = "hi"` gives `char *`, not
`char[3]`. A member cannot be `auto`, and a function cannot return `auto`.
If a name is used while its own type is still `Auto`, that is reported rather
than silently accepted — `auto x = x;`.

### Array sizes from the initializer

`sizeFromInitializer` fills in a size the declarator left empty:

```cpp
int  a[] = {1, 2, 3};    // int[3]
char s[] = "hi";         // char[3] — two characters and the \0
int  b[];                // error: needs a size or an initializer
```

A size of 0 inside an `ArrayType` means "unsized", and only the outermost
dimension may be left empty — `int grid[][3]` is fine as a parameter,
`int grid[2][]` never is.

### String literals are arrays

`"hello"` has type `char[6]` and **is an lvalue**. Both matter:

- the length includes the `\0`, so `char s[5] = "hello"` is an error (needs 6)
- being an array means it decays to `char *` when passed, which is why
  `printf("%d", x)` type-checks against `int (char*, ...)`

The length is counted with escapes decoded, so `"tab\there"` is 8 characters
plus the `\0`. The lexer keeps the raw spelling with quotes, so
`stringLiteralLength` decodes it.

### Parameters are adjusted, not stored as written

```cpp
int sumGrid(int m[][3], int rows);   // m is really int (*)[3]
int apply(int (*g)(int));            // already written as a pointer
```

`adjustParameter` applies `decay` to array and function parameters at
declaration time, so the symbol's type is already the pointer. This is why
`char *argv[]` shows as `char**` in the dump, and why it gives the type system
a multi-level pointer to exercise even in the old tests.

The function case only arises through the parenthesised form: a bare function
declarator as a parameter (`int f(int g(int))`) is not in this grammar, so a
callable parameter is written `int (*g)(int)`.

### f(void) means no parameters

`isVoidParamList` catches the C spelling: one unnamed parameter of type `void`
means an empty parameter list, not one void parameter. `void` is otherwise
rejected as a parameter or variable type.

### Lambdas get a type each, and captures are resolved outwards

Every `LambdaExpr` has its own `ClosureType`. Its signature is not known until
the body is checked, because the return type comes from the `return`
statements, so `ClosureType::signature` is filled at the end of
`visitLambdaExpr`. Two returns of different types is an error.

`usedCaptures` is the field codegen needs: it holds both the listed captures
*and* the ones a `[=]` or `[&]` default picked up while checking the body. A
lambda that uses a local it did not capture is an error, and the capture must
be added to **every** lambda in between when they nest. `this` is captured the
same way, which is why `this` inside a non-capturing lambda in a method is
rejected.

The closure lowers to a struct of its captures: by-copy captures by value,
by-reference captures as a pointer. From test7:

```text
cube     : lambda <262:17>  llvm: {}            // [] captures nothing
byValue  : lambda <265:22>  llvm: { i32 }       // [squares]
byRef    : lambda <266:22>  llvm: { ptr }       // [&squares]
allValue : lambda <267:22>  llvm: { i32 }       // [=] picked up squares
allRef   : lambda <268:22>  llvm: { ptr }       // [&] picked up squares
mixed    : lambda <269:22>  llvm: { i32, ptr }  // [squares, &trace]
```

### Other result fields

| Field | Holds |
| --- | --- |
| `DeclRefExpr::symbol` | the declaration a name resolved to |
| `MemberExpr::symbol` | the field or method |
| `CallExpr::function` | the chosen overload, null for a call through a pointer or lambda |
| `GotoStmt::target` | the `LabelStmt`, resolved after the body |
| `CaseStmt::constant` | the evaluated label value |
| `TypeSpec::resolved` | the `sema::Type` for a written type |
| `LambdaCapture::symbol` | the captured variable |

---

## 9. Things not discussed that you need to know

### The error type stops error cascades

A failed expression gets `errorType()`, and every later check accepts it
silently. `findConversion` returns `Exact` if either side is an error. So one
mistake produces one message:

```cpp
int x = undeclared + 1;   // one error, not also "invalid operands to '+'"
```

The practical consequence: **a null or error type is never a reason to report
something**. Every check in `typecheck.cpp` tests `isError` first and returns
early.

### Access control checks two separate things

`checkAccess` is longer than expected because `private`/`protected` on the
member and the access level of the *inheritance path* are different questions:

```cpp
class Base { public: int open; };
class Hidden : private Base { };       // inherits Base privately
class FromHidden : public Hidden {
    int f() { return open; }           // error: open is public in Base,
};                                     // but Hidden inherited it privately
```

So the lookup returns the path of bases it walked (`Lookup::path`), and
`checkAccess` walks it checking each hop. A name found through two different
bases is ambiguous and reported as such.

Unwritten access defaults differ by keyword: a `class` defaults to `private`
members and private inheritance, a `struct` to `public` for both.

### Overload resolution ranks every argument

`findConversion` gives each argument a rank (`Exact` < `Promotion` <
`Conversion` < `Impossible`). A candidate is **viable** if no argument is
`Impossible`. Among viable candidates, one wins if no argument converts worse
than another candidate's and at least one converts better. Otherwise the call
is ambiguous and reported.

```cpp
int pick(int a);
int pick(double a);
pick(1);     // Exact for int, Conversion for double -> int wins
pick(1.5);   // Exact for double -> double wins
pick(p);     // Impossible for both -> "no 'pick' matches the arguments (Point)"
```

Constructors go through the same function, which is how `Animal withTwo(3, 4)`
is caught when only `Animal()` and `Animal(int)` exist.

### Variadic calls promote

`printf` and `scanf` are predeclared as `int (char*, ...)` and left unmangled.
Arguments past the named parameters get the C default promotions — `float`
becomes `double`, `bool` and `char` become `int` — and records, void and
closures are rejected there outright.

### Constant evaluation is needed in three places

`evaluateConstant` handles literals, the arithmetic, bitwise, logical and
comparison operators, and `?:`. It is used for array sizes, `case` labels, and
the inner dimensions of `new T[n][3]`. It returns false for anything naming a
variable, which is what produces "an array size must be an integer constant".

Integer literals are decoded for all four bases the lexer accepts (`42`,
`0x1F`, `0755`, `0b1010`), and a literal too large for `int` is reported.

### new T[n][3] only varies in its first dimension

```cpp
int *p = new int[n];                  // n may be any integer expression
int (*m)[3] = new int[2][3];          // 3 must be constant; result is int (*)[3]
int (*q)[3] = new int[n][3];          // fine: only the first varies
```

The inner dimensions become part of the element type, so the result type is a
pointer to that element, which is why `new int[2][3]` gives `int (*)[3]` and
needs a pointer-to-array declarator to hold it.

### Records lower to named LLVM structs, bases first

`lowerRecord` emits bases in declaration order, then fields in layout order,
and looks the struct up by name first (`StructType::getTypeByName`) so each
record is created once per context. Creating the struct before setting its body
is what makes `struct Node { Node *next; }` work — the field only needs `ptr`.

```text
class Dog        : %class.Dog { %class.Animal, i1 }
class Registered : %class.Registered { %class.Animal, %class.Trackable, i32 }
class Outer      : %class.Outer { %struct.Outer.Inner, %class.Outer.Nested }
```

Base subobjects come first because that makes a `Derived*` to `Base*`
conversion a no-op for the first base. The second base needs a real offset,
which is the codegen work mentioned earlier.

### One namespace for types and variables

`struct Point` and `Point` name the same thing here. C's separate tag namespace
is not needed for this subset, so a clash between a type name and a variable
name in one scope is an error.

### Diagnostics are sorted

The passes do not visit the file front to back — pass 1 declares everything,
pass 2 walks bodies — so `Diagnostics::print` stable-sorts by line and column
before printing. Error output therefore reads in source order regardless of
which pass found what.

### Build differences between macOS and Linux

Homebrew's LLVM is built against libc++, so its headers must be compiled with
`clang++`; the local `g++` is Homebrew GCC with libstdc++ and would fail to
link. The Makefile uses `clang++` and Homebrew's `llvm-config` on Darwin, and
`g++` with `llvm-config` from the `PATH` on Linux. Both are overridable:

```bash
make CXX=g++-14 LLVM_CONFIG=/usr/bin/llvm-config-20
```

Only `core` and `support` are linked. `llvm-config --cxxflags` adds
`-fno-exceptions`, which is fine since this code uses none.

---

## 10. Limits, and what codegen will need

### Deliberately not done

- **No IR generation.** `toLLVM` is written and exercised by the dump, but
  nothing emits instructions.
- **No nested C declarators** (`int (*(*f)(int))[3]`), to keep the grammar
  conflict-free.
- **No virtual functions**, so no vtables. The grammar has no `virtual`.

### Accepted although a real compiler might complain

- A function or method **declared but never defined** is accepted, because that
  is a link-time error, not a type error. test4 declares `Animal(int, int)`
  without defining it.
- `delete p` on memory from `new[]` is undefined behaviour but not a type
  error, so test3 and test7 still pass.
- A `switch` with no `default` is fine, as is a non-void function with no
  `return` on some path — reachability analysis is not part of this phase.

### Pre-existing grammar limits met while writing tests

- **A record cannot be defined inside a function body.** `struct Point { int x; };`
  at block scope does not parse; records are file-scope or class members. This
  is why test20 declares its struct at file scope.
- `static` on an out-of-class method definition is rejected, matching C++:
  it is written on the in-class declaration.

### What codegen can rely on

Everything it needs is on the tree, so no pass has to be redone:

| Needs | Where |
| --- | --- |
| the type of any expression | `Expr::type` |
| whether to load or use an address | `Expr::isLValue` |
| which declaration a name means | `DeclRefExpr::symbol`, `MemberExpr::symbol` |
| which function a call runs | `CallExpr::function`, then its `loweredType` |
| every conversion, in order | the `ImplicitCastExpr` chain |
| which constructor to call | `VarDecl`/`NewExpr`/`InitListExpr::constructor` |
| field offsets | `RecordSymbol::fields`, in layout order |
| the symbol for any global | its `mangledName` |
| a switch label's value | `CaseStmt::constant` |
| a goto's destination | `GotoStmt::target` |
| a lambda's real captures | `LambdaExpr::usedCaptures` |

The scope tree is still alive and reachable from the same nodes, so codegen can
walk back into any function's scope to find its locals.

### Test coverage added

| Test | Covers |
| --- | --- |
| test18 | undeclared and redeclared names, use before declaration, bad initialisation and assignment, bad deref/index/member/delete, array sizes, `this` outside a method — 25 errors |
| test19 | private and protected access, and access through a privately inherited base — 6 errors |
| test20 | argument counts, non-converting arguments, no matching overload, calling a non-function, function pointer mismatch, member function without an object — 11 errors |
| test21 | `break`/`continue` outside a loop, missing and duplicate labels, non-integer `switch`, duplicate and non-constant `case`, two `default`s, non-scalar conditions, a `for` variable after its loop — 15 errors |
| test22 | the type system, as a file that must be accepted: multi-level pointers, pointers to arrays, pointer to function pointer, reference to array, `new` with pointer element types, pointer arithmetic, `null` and `void *` |

test4 was corrected rather than left failing: `int Animal::age = 0;` defined a
non-static member, and `Animal withTwo(3, 4)` called a constructor that did not
exist. `age` became a `static int created` with a matching out-of-class
definition, and an `Animal(int, int)` was declared.
