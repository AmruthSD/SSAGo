# SSAGo — an optimizing compiler for a Go-like language, built on LLVM

Written from scratch in C++, SSAGo lowers a Go-inspired source language into a hand-rolled SSA intermediate representation, runs it through a small suite of classic optimization passes, and emits LLVM IR for native code generation. It also ships a lightweight concurrency runtime — implemented with Boost.Context — supporting goroutines and wait groups.

## Features

**Language**

- Variables and assignment
- Integer and floating-point arithmetic
- Conditionals and while loops
- Functions, function calls, and recursion
- Structs
- Global variables
- Dynamic memory allocation
- Goroutines and WaitGroups
- External (C) function calls

**Compiler**

- Custom SSA intermediate representation
- Dominator tree construction
- Constant folding and propagation
- Dead code elimination
- Global value numbering
- LLVM IR generation and native code emission

## Pipeline

```
Source Code
    │
    ▼
  Lexer
    │
    ▼
  Parser  ──▶  AST
    │
    ▼
Custom SSA IR
    │
    ▼
Optimization Passes
    │   • Constant Folding & Propagation
    │   • Dead Code Elimination
    │   • Global Value Numbering
    ▼
  LLVM IR
    │
    ▼
Native Executable
```

## Intermediate Representation

The AST is lowered into a custom SSA-based IR before LLVM IR generation. A representative snippet:

```
add  a, b, t1
mul  t1, c, t2
icmp_lt t2, 10, t3
condbr t3, then, else
```

The IR models:

- Basic blocks and control flow graphs
- Phi nodes
- SSA values
- Arithmetic and comparison instructions
- Function calls
- Memory operations

## Optimization Passes

### Constant Folding

Evaluates constant expressions at compile time.

```
2 + 3   →   5
```

### Constant Propagation

Propagates known constant values through the IR.

```
x = 5
y = x + 1
→
y = 6
```

### Dead Code Elimination (DCE)

Removes instructions whose results are never used and which have no side effects.

### Global Value Numbering (GVN)

Identifies redundant, equivalent computations using SSA form and dominator-tree analysis, then relies on DCE to remove what's left behind.

```
t1 = add a, b
t2 = add a, b
→
t2 → t1   (t2's uses are rewritten to t1, then t2 is eliminated)
```

## Concurrency Runtime

The runtime supports lightweight, user-space concurrency modeled on Go's goroutines.

### Goroutines

```go
go hello()
```

Goroutines are implemented as user-space coroutines using **Boost.Context**. Each goroutine has its own stack and execution context, and context switches happen entirely in user space — no OS thread scheduling involved. Execution is performed by saving and restoring CPU register state and stack pointers, allowing many goroutines to multiplex over a small number of OS threads at a fraction of the cost of OS-level context switches.

### WaitGroups

```go
WaitGrp wg = waitgroup_new();
waitgroup_add(wg, 1);
go hello(x, wg);
waitgroup_wait(wg);
```

WaitGroups are implemented using native threading primitives for synchronization between goroutines.

## LLVM Integration

Once lowered to LLVM IR, the compiler hands off to LLVM for:

- Instruction selection
- Register allocation
- Target-specific optimization
- Machine code generation

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Running

```bash
./LinkAndRunOutput.sh input.txt
```

This compiles `input.txt`, generates LLVM IR, and produces a native executable.
