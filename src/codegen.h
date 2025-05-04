#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef struct variableInMemory {
  char* variableName;
  int memory_difference;
  int variable_type;
} variableInMemory;

typedef struct memory {
  variableInMemory** variables;
  int variableCapacity;
  int numberOfVariables;
  int nextStartingLocation;
} memory;

typedef struct listOfX86Instructions {
  char** instructions;
  int instructionCount;
  int instructionCapacity;
} listOfX86Instructions;

typedef struct {
  TokenType symbol;
  const char* name;
} map;

// ───── Memory and Instruction Management ─────

/*
Initializes a memory struct for variable tracking.

Sets default capacity, allocates initial space, and sets starting memory offset
for stack variables.

Args:
  mem: Pointer to memory struct.

Returns:
  void
*/
void initMemory(memory* mem);

/*
Adds a variable to the memory tracking system.

Stores its name and stack offset in the memory table.

Args:
  mem: Pointer to memory struct.
  variableName: Name of the variable to add.

Returns:
  void
*/
void addVariableToMemory(memory* mem, char* variableName);

/*
Finds the stack memory location of a variable.

Searches the memory table for a variable name and returns its offset.

Args:
  mem: Pointer to memory struct.
  lexeme: Name of the variable.
  length: Length of the variable name.

Returns:
  Stack offset (int) if found, or 0 if not found.
*/
int get_variable_memory_location(memory* mem, const char* lexeme, int length);

/*
Generates the memory address string for a variable (e.g., [rbp-4]).

Creates a string suitable for use in x86 assembly from a variable's memory
offset.

Args:
  mem: Pointer to memory struct.
  lexeme: Variable name.
  length: Length of variable name.

Returns:
  String with formatted memory address.
*/
char* get_variable_memory_location_with_pointer(memory* mem, const char* lexeme,
                                                int length);

/*
Initializes a list to hold x86 instructions.

Allocates initial memory and sets instruction count to 0.

Args:
  list: Pointer to the listOfX86Instructions struct.

Returns:
  void
*/
void initListOfInstructions(listOfX86Instructions* list);

/*
Adds an x86 instruction to the instruction list.

Automatically resizes the list if needed.

Args:
  list: Pointer to instruction list.
  instruction: x86 instruction string.

Returns:
  void
*/
void addInstruction(listOfX86Instructions* list, char* instruction);

/*
Prints the memory map (variable names and their offsets).

Useful for debugging memory layout and tracking.

Args:
  mem: Pointer to memory struct.

Returns:
  void
*/
void printMemory(memory* mem);

// ───── Instruction Generation ─────

/*
Generates x86 instructions from a variable, literal, or binary expression AST
node.

Dispatches to specialized handlers based on node type.

Args:
  node: Pointer to the AST node.
  list: List of generated x86 instructions.
  mem: Variable memory tracking struct.

Returns:
  void
*/
void ASTVariableLiteralOrBinaryToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem);

/*
Generates x86 code from either a variable or a literal.

Used when the node type is guaranteed to be a simple operand.

Args:
  node: ASTNode representing the operand.
  list: Instruction list.
  mem: Memory state.

Returns:
  void
*/
void ASTVariableOrLiteralNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                   memory* mem);

/*
Generates x86 code for a binary expression.

Evaluates left and right subtrees and emits the appropriate assembly based on
the operator.

Args:
  node: AST_BINARY node.
  list: Instruction list.
  mem: Memory context.
  first: Flag to indicate if this is the root binary node in an expression.

Returns:
  void
*/
void ASTBinaryNodeToX86(ASTNode* node, listOfX86Instructions* list, memory* mem,
                        int first);

/*
Generates x86 code for a variable declaration.

Emits memory allocation and possibly initialization for declared variables.

Args:
  node: AST_VARIABLE_DECLARATION node.
  list: Instruction list.
  mem: Memory context.

Returns:
  void
*/
void ASTVariableDeclarationNodeToX86(ASTNode* node, memory* mem);

/*
Generates x86 code for a full declaration (type + assignment).

Handles both memory allocation and assignment logic.

Args:
  node: AST_DECLARATION node.
  list: Instruction list.
  mem: Memory context.

Returns:
  void
*/
void ASTDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                             memory* mem);

/*
Generates x86 code for a return statement.

Loads the return value into the appropriate register and emits `ret`.

Args:
  node: AST_RETURN node.
  list: Instruction list.
  mem: Memory context.

Returns:
  void
*/
void ASTReturnNodeToX86(ASTNode* node, listOfX86Instructions* list,
                        memory* mem);

/*
Generates x86 instructions for a general AST statement.

Dispatches based on the node type to the appropriate handler.

Args:
  node: AST statement node.
  list: Instruction list.
  mem: Memory context.

Returns:
  void
*/
void ASTStatementNodeToX86(ASTNode* node, listOfX86Instructions* list,
                           memory* mem);

/*
Generates x86 code for a block of statements.

Sequentially emits instructions for each node in the block.

Args:
  node: AST_BLOCK node.
  list: Instruction list.
  mem: Memory context.

Returns:
  void
*/
void ASTBlockNodeToX86(ASTNode* node, listOfX86Instructions* list, memory* mem);

/*
Generates x86 instructions for a function call.

Prepares arguments, handles calling convention, and emits `call`.

Args:
  node: AST_FUNCTION_CALL node.
  list: Instruction list.
  mem: Memory context.

Returns:
  void
*/
void ASTFunctionCallNodeToX86(ASTNode* node, listOfX86Instructions* list,
                              memory* mem);

/*
Generates full x86 instructions for a function.

Emits function prologue, body, and epilogue, and handles memory context.

Args:
  node: AST_FUNCTION_DECLARATION node.
  list: Instruction list.

Returns:
  void
*/
void ASTFunctionNodeToX86(ASTNode* node, listOfX86Instructions* list);

/*
Generates x86 instructions for all top-level function nodes.

Traverses the AST function declarations and emits their corresponding x86 code.

Args:
  nodes: Array of ASTNode pointers.
  list: Output instruction list.
  numberOfFunctions: Number of AST function nodes.

Returns:
  void
*/
void ListOfASTFunctionNodesToX86(ASTNode** nodes, listOfX86Instructions* list,
                                 int numberOfFunctions);

// ───── Output ─────

/*
Prints all x86 instructions in the list.

Used to emit the final generated assembly.

Args:
  list: Instruction list.

Returns:
  void
*/
void printInstructions(listOfX86Instructions* list);

// ───── Helpers ─────

/*
Gets the corresponding x86 operation name from a token type.

Maps token types like `TOKEN_PLUS` to `"add"`.

Args:
  op: TokenType representing the operator.

Returns:
  String with x86 operation name, or NULL if unknown.
*/
const char* get_op_name(TokenType operator);

/*
Returns the name of a general-purpose Linux x86 register.

Used for allocating function arguments or temporaries.

Args:
  i: Index into a register mapping.

Returns:
  Name of the corresponding register (e.g., "edi", "esi", etc.).
*/
const char* getLowLinuxRegistersName(int index);
