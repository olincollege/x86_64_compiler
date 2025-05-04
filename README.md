# Simple C Compiler Front-end

## Overview

This project is a simple C compiler front‑end that performs lexical analysis, parsing, AST generation, and x86 code generation. It reads a C‑like source file, tokenizes it, builds an abstract syntax tree (AST), and outputs both the AST and corresponding x86 instructions.

## Features

- **Lexer**: Tokenizes the input source code into a stream of tokens (identifiers, keywords, literals, operators).
- **Parser**: Uses recursive descent to construct an AST from the token stream, handling expressions, statements, and function declarations.
- **Code Generator**: Traverses the AST to emit x86 assembly instructions, managing variable memory and instruction lists.
- **Driver**: A `main.c` executable reads a source file (`test.txt` by default), invokes the lexer, parser, and codegen, then prints tokens and AST nodes.
- **Unit Tests**: A dedicated test file contains all unit tests to validate lexer, parser, and codegen components.

## Directory Structure

```
├── src/
│   ├── lexer.c
│   ├── lexer.h
│   ├── parser.c
│   ├── parser.h
│   ├── codegen.c
│   ├── codegen.h
│   └── main.c
├── test/
│   └── all_tests.c   ← Unit tests for each module
├── test.txt          ← Sample input source file
└── CMakeLists.txt    ← Build configuration
```

## Prerequisites

- **C Compiler**: GCC or Clang
- **CMake**: Version 3.10+
- **Make**

## Building the Project

```bash
mkdir build
cd build
cmake ..
make
```

This generates the `main` executable and links the `lexer` library as specified in `CMakeLists.txt`.

## Running

To run the compiler front‑end on the sample input:

```bash
./main ../test.txt
```

- **Token Output**: A sequence of tokens will be printed.
- **AST Output**: The parsed AST will be printed to the console.

## Testing

Unit tests reside in `test/all_tests.c`. To compile and run tests:

```bash
# From build directory
gcc -o tests ../test/all_tests.c -I../src
./tests
```

Ensure all tests pass before making changes.

## Contributing

1. Fork the repository.
2. Create a feature branch: `git checkout -b feature/your-feature`.
3. Commit your changes with clear messages.
4. Submit a pull request for review.

## License

This project is licensed under the MIT License. See `LICENSE` for details.
