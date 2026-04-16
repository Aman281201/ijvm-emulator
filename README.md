# IJVM Emulator in C

A complete implementation of the **Integer Java Virtual Machine (IJVM)** — a stack-based virtual machine described in Andrew Tanenbaum's *Structured Computer Organization*. Written in C with a dynamic operand stack, constant pool, local variable frames, and full method dispatch via `INVOKEVIRTUAL`/`IRETURN`.

## What is IJVM?

IJVM is a simplified subset of the JVM instruction set, designed to illustrate how a stack machine executes programs. It operates on 32-bit signed integers (`word_t`), reads programs from a binary format (`.ijvm`), and supports a constant pool, method calls with arguments, and local variable scoping per call frame.

## What's Implemented

### Opcodes

| Category | Opcode | Description |
|----------|--------|-------------|
| **Constants** | `BIPUSH byte` | Push a sign-extended byte |
| | `LDC_W index` | Push a word from the constant pool |
| **Stack** | `DUP` | Duplicate top of stack |
| | `POP` | Discard top of stack |
| | `SWAP` | Swap the top two values |
| **Arithmetic** | `IADD` | Integer add |
| | `ISUB` | Integer subtract |
| | `IAND` | Bitwise AND |
| | `IOR` | Bitwise OR |
| **Local Variables** | `ILOAD index` | Push local variable |
| | `ISTORE index` | Pop into local variable |
| | `IINC index, const` | Increment local variable |
| **Control Flow** | `GOTO offset` | Unconditional branch |
| | `IFEQ offset` | Branch if top == 0 |
| | `IFLT offset` | Branch if top < 0 |
| | `IF_ICMPEQ offset` | Branch if top two are equal |
| **Methods** | `INVOKEVIRTUAL index` | Call method (saves frame, passes args) |
| | `IRETURN` | Return integer, restore caller frame |
| **I/O** | `IN` | Read a character from stdin |
| | `OUT` | Write a character to stdout |
| **Prefix** | `WIDE` | Extend next instruction to use 16-bit index |
| **Control** | `NOP` | No operation |
| | `HALT` | Stop execution |
| | `ERR` | Halt with error |

### Method Calls

`INVOKEVIRTUAL` fully manages the call stack:
- Saves the caller's `lv_offset`, `lv_pointer`, and return address on the operand stack
- Allocates a new local variable frame for the callee
- Passes arguments from the stack into the new frame
- `IRETURN` pops the frame, restores the caller, and pushes the return value

This supports arbitrary recursion depth, tested with Fibonacci and recursive sum programs.

### `WIDE` Prefix

`WIDE ILOAD`, `WIDE ISTORE`, and `WIDE IINC` extend the index to 16 bits, allowing programs to reference more than 256 local variables.

## Architecture

```
┌──────────────────────────────────────────────────┐
│                  IJVM Machine                    │
│                                                  │
│  ┌──────────┐  ┌──────────┐  ┌────────────────┐ │
│  │ Constant │  │   Text   │  │ Local Variable │ │
│  │   Pool   │  │ (opcodes)│  │    Frames      │ │
│  └──────────┘  └──────────┘  └────────────────┘ │
│                                                  │
│  ┌────────────────────────────────────────────┐  │
│  │         Operand Stack (dynamic)            │  │
│  │  [  val  |  val  |  frame_ptr  |  ret_pc ] │  │
│  └────────────────────────────────────────────┘  │
│                                                  │
│   PC ──► text[PC] ──► step() ──► dispatch        │
└──────────────────────────────────────────────────┘
```

The operand stack doubles as the call stack — `INVOKEVIRTUAL` pushes the saved `lv_pointer`, `lv_offset`, number of frame slots, and return PC directly onto it. `IRETURN` unwinds by popping them in reverse.

## Building

Requires `clang` and `make`.

```bash
make ijvm          # build the emulator binary
make clean         # remove build artifacts
```

Optional compiler flags:

```bash
make clean && make ijvm CFLAGS=-DDEBUG    # enable debug prints (see include/util.h)
make pedantic                              # stricter warning flags
```

## Running

```bash
./ijvm <path-to-binary.ijvm>
```

Examples:

```bash
./ijvm files/examples/collatz.ijvm
./ijvm files/advanced/Tanenbaum.ijvm
./ijvm files/task5/fib.ijvm
```

Assemble `.jas` sources to `.ijvm` using the bundled GoJASM assembler:

```bash
make tools                            # download GoJASM into tools/
make ijvm_binaries                    # assemble all .jas files in files/
```

## Test Suite

| Test | What it covers |
|------|----------------|
| `test1` — BINARIES | Binary loading, magic number, text section, constant pool |
| `test2` — STACK | `BIPUSH`, `IADD`, `ISUB`, `IAND`, `IOR`, `DUP`, `POP`, `SWAP`, `LDC_W`, `IN`/`OUT` |
| `test3` — FLOW | `GOTO`, `IFEQ`, `IFLT`, `IF_ICMPEQ`, Collatz conjecture |
| `test4` — VARS | `ILOAD`, `ISTORE`, `IINC`, `WIDE` variants |
| `test5` — METHODS | `INVOKEVIRTUAL`, `IRETURN`, argument passing, nested calls, recursion, Fibonacci |
| `testadvanced1` — WIDE | Edge cases for the `WIDE` prefix with 16-bit indices |
| `testadvanced2` — ALL INSTRUCTIONS | Full opcode coverage in a single program |
| `testadvanced3` — TANENBAUM | Tanenbaum's reference IJVM program |
| `testadvanced4` — CALCULATOR | Interactive arithmetic (SimpleCalc) |
| `testadvanced5` — CALC\_FACTORIAL | Recursive factorial via IJVM methods |
| `testadvanced6` — MANDELBROT | Mandelbrot set rendered in IJVM |
| `testadvanced7` — TALL\_STACK | Deep stack / many local variable frames |

```bash
make testbasic       # run test1–test5
make testadvanced    # run testadvanced1–testadvanced7
make testall         # run everything
make testleaks       # run all tests under Valgrind
make testsanitizers  # run with ASan + UBSan (requires clang)
make coverage        # LLVM coverage report
make grade           # print numeric grade (requires Python 3)
```

## Project Structure

```
ijvm-emulator/
├── src/
│   ├── machine.c        # VM core: init, step, run, all opcode handlers
│   ├── main.c           # CLI entry point
│   └── util.c           # Endianness helpers (big-endian binary parsing)
├── include/
│   ├── ijvm.h           # Public VM API (DO NOT MODIFY — grading interface)
│   └── util.h           # Debug print macros (dprintf levels 1–5)
├── tests/
│   ├── test1–5.c        # Basic test suites
│   ├── testadvanced1–7.c
│   ├── testbonus*.c     # Bonus: garbage collection, tail calls
│   └── testutil.h       # RUN_TEST / END_TEST macros
├── files/
│   ├── task1–5/         # .ijvm binaries + .jas sources per test suite
│   ├── advanced/        # Complex programs (Tanenbaum, Mandelbrot, etc.)
│   ├── examples/        # Collatz, recursive sum, ones
│   └── bonus/           # Tail calls, GC, networking, Brainfuck interpreter
├── tools/
│   ├── Makefile         # Downloads GoJASM assembler
│   └── grade.py         # Grading script
└── Makefile
```

## Binary Format

IJVM binaries follow the format:

```
[magic: 0x1DEADFAD] [origin_cp] [const_pool_size] [const_pool_data...]
[origin_text] [text_size] [text_data...]
```

All multi-byte values are big-endian. `machine.c` byte-swaps them on load via `swap_u32`.
