# x86-64 Compiler

## Overview

We implemented a C compiler from scratch in C that outputs x86-64 assembly! We did this to learn about low-level code generation and gain insight towards the compilation process. Note that this compiler uses only 32-bit operations, and supports a small subset of the C language. Currently, this includes functions, variable declarations, expressions, basic control flow (if, else if, else) and basic arithmetic operations.

## Features
 - Lexer: The lexer is responsible for tokenizing the source code. It reads the input file character by character and returns a sequence of tokens.
 - Parser: The parser is responsible for analyzing the syntax of the source code. It takes the output from the lexer and generates an Abstract Syntax Tree (AST) representation of the program.
 - Code Generator: Walks the AST and emits corresponding x86-64 assembly instructions.
 - Modular Design: The compiler is structured into separate components for clarity and maintainability.

## Project Structure

```
x86_64_compiler/
├── src/
│   ├── lexer.c          # Lexical analysis
│   ├── parser.c         # Syntax analysis
│   ├── codegen.c        # Code generation
│   └── main.c           # Compiler entry point
├── test/                # Unit Testing
│   ├── test_lexer.c
│   ├── test_parser.c
│   ├── test_codegen.c
│   ├── test_compiler.c
├── CMakeLists.txt       # Build configuration
├── .clang-format        # Code formatting rules
├── .clang-tidy          # Static analysis configuration
└── README.md            # Project documentation
```

##  Dependencies

 - A Linux machine
 - `git` to clone the repository
 - `CMake` version 3.22

# Build and Run

Clone the repository:
```
$ git clone https://github.com/olincollege/x86_64_compiler.git
```
Build it with:
```
$ mkdir build && cd build
$ cmake ..
$ make
```

## Run Compiler on Test File

Clone the repository:
```
$ git clone https://github.com/olincollege/x86_64_compiler.git
```

Run compiler on `test.txt`:
```
$ cd src/
$ ./run.sh
```

The script `run.sh` performs a full pipeline test of the compiler. It:
1. Removes any previous tokens file.
2. Compiles all .c files in the current directory to produce the compiler executable (a.out).
3. Runs the compiler on a test C file, generating chat.s (x86-64 assembly).
4. Assembles the assembly file into an object file test.o using as.
5. Links the object file into an executable test using ld.
6. Clears the terminal and runs the test binary.
7. Prints the output labeled as "Return Value".

## Future Work

The following features are planned for future development:

- Support for more advanced syntax elements
- Error handling and reporting
- Semantic analysis
