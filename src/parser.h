#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
typedef enum {
  AST_INT_LITERAL,
  AST_VARIABLE,
  AST_VARIABLE_DECLARATION,
  AST_BINARY,  // For binary operators like +, -, *, etc.
  AST_UNARY,   // For unary operators such as negation
  AST_ASSIGNMENT,
  AST_DECLARATION,
  AST_FUNCTION_DECLARATION,
  AST_FUNCTION_CALL,
  AST_IF_STATEMENT,
  AST_WHILE_STATEMENT,
  AST_BLOCK,
  AST_RETURN,
  AST_FOR_STATEMENT,
  AST_ELSE_IF_STATEMENT,
  AST_ELSE_STATEMENT,
  // Add more as needed...
} ASTNodeType;

typedef struct ASTNode {
  ASTNodeType type;  // Helps identify which kind of node this is.
  // int line;         // Optional: storing line number for debugging or error
  // messages.
  union {
    // For integer literals.
    struct {
      int intLiteral;
      Token* token;  // The token representing the integer literal.
    } intLiteral;

    Token* variableName;

    // For a variable or identifier.
    struct {
      Token* name;  // The name of the variable.
      Token* type;  // TODO: Rename to variabletype // Type of variable, e.g.,
                    // "int", "float"
    } variable_declaration;

    // For binary expressions.
    struct {
      struct ASTNode* left;
      TokenType _operator;  // You might use a char or a TokenType here.
      struct ASTNode* right;
    } binary;

    struct {
      struct ASTNode* condition;
      struct ASTNode* body;
    } while_statement;

    struct {
      struct ASTNode* condition;
      struct ASTNode* body;
    } if_elif_else_statement;

    // For unary expressions.
    struct {
      char _operator;
      struct ASTNode* operand;
    } unary;

    struct {
      Token* name;  // The name of the function.
      Token* returnType;
      struct ASTNode** parameters;  // List of parameters (ASTNodes).
      int paramCount;               // Number of parameters.
      struct ASTNode* statements;   // Block for the statements in the function.
    } function;

    struct {
      Token* name;                  // The name of the function.
      struct ASTNode** parameters;  // List of parameters (ASTNodes).
      int paramCount;               // Number of parameters.
    } function_call;

    struct {
      struct ASTNode* variable;
      struct ASTNode* expression;
    } declaration;

    // For blocks (a list of statements).
    struct {
      struct ASTNode** statements;
      int count;  // The number of statements in the block.
    } block;

    struct {
      struct ASTNode* expression;
    } _return;

    // For an assignment, declaration, or other composite structures,
    // you can add additional fields or even nested structs here.
  } as;
} ASTNode;
/*
Parses a function call node from the token stream.

Scans tokens starting at `*tokenIndex` to parse a function call (e.g.,
`foo(arg1, arg2)`), constructing an AST node representing the function call.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current index in token array.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* representing the function call.
*/
ASTNode* parseFunctionCall(Token* tokens, int* tokenIndex, int tokenCount);

/*
Recursively prints the AST starting from the given node.

Traverses the AST recursively and prints it with proper indentation to visualize
the structure.

Args:
  file: Output stream to print to.
  node: Root of the AST subtree.
  indent: Current indentation level.

Returns:
  void
*/
void printAST(FILE* file, ASTNode* node, int indent);

/*
Prints an array of AST nodes to a file.

Outputs all given AST nodes to a file named "ast" with indentation and
structure.

Args:
  nodes: Array of ASTNode pointers.
  count: Number of AST nodes.

Returns:
  void
*/
void printASTFile(ASTNode** nodes, int count);

/*
Prints the AST output to stdout or a file.

Iterates over the array of AST nodes and prints each one, optionally to a file.

Args:
  nodes: Array of ASTNode pointers.
  count: Number of AST nodes.
  outputToFile: 1 to print to file, 0 to stdout.

Returns:
  void
*/
void printASTOutput(ASTNode** nodes, int count, int outputToFile);

/*
Parses a block (compound statement) of code.

Parses a sequence of statements enclosed in `{}` or a single statement and
returns a block AST node.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current index in token array.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* representing the block.
*/
ASTNode* parseBlock(Token* tokens, int* tokenIndex, int tokenCount);

/*
Creates a new AST node for an integer literal.

Constructs a node representing a constant integer.

Args:
  value: The integer value.
  token: Token representing the integer.

Returns:
  ASTNode* representing the literal.
*/
ASTNode* newIntLiteralNode(int value, Token* token);

/*
Creates a new AST node for a variable.

Constructs a node representing a variable usage.

Args:
  name: Token representing the variable name.

Returns:
  ASTNode* representing the variable.
*/
ASTNode* newVariableNode(Token* name);

/*
Creates a new AST node for a variable declaration.

Constructs a node for declaring a variable with a type.

Args:
  name: Token for variable name.
  type: Token for variable type.

Returns:
  ASTNode* representing the declaration.
*/
ASTNode* newVariableDeclarationNode(Token* name, Token* type);

/*
Creates a new binary expression node.

Constructs a node representing a binary operation (e.g., +, -, etc.) between two
subexpressions.

Args:
  left: Left-hand expression.
  operator: Operator token type.
  right: Right-hand expression.

Returns:
  ASTNode* representing the binary expression.
*/
ASTNode* newBinaryNode(ASTNode* left, TokenType operator, ASTNode * right);

/*
Creates a new unary expression node.

Constructs a node for unary operations such as negation.

Args:
  operator: Operator character.
  operand: Operand node.

Returns:
  ASTNode* representing the unary expression.
*/
ASTNode* newUnaryNode(char operator, ASTNode * operand);

/*
Creates a new block node containing multiple statements.

Constructs a block node from an array of statement AST nodes.

Args:
  statements: Array of ASTNode pointers.
  count: Number of statements.

Returns:
  ASTNode* representing the block.
*/
ASTNode* newBlockNode(ASTNode** statements, int count);

/*
Creates a new function declaration node.

Constructs an AST node representing a function definition with parameters and
body.

Args:
  name: Token for function name.
  returnType: Token for return type.
  parameters: Array of parameter ASTNodes.
  count: Number of parameters.
  statements: Block node for function body.

Returns:
  ASTNode* representing the function.
*/
ASTNode* newFunctionNode(Token* name, Token* returnType, ASTNode** parameters,
                         int count, ASTNode* statements);

/*
Creates a return statement node.

Constructs a node representing a `return` expression.

Args:
  expression: ASTNode for the return value.

Returns:
  ASTNode* representing the return.
*/
ASTNode* newReturnNode(ASTNode* expression);

/*
Creates a declaration node for a variable initialized with an expression.

Represents `int x = 3;` as a declaration node holding both variable and value.

Args:
  variableDeclaration: ASTNode for the variable declaration.
  expression: ASTNode for the right-hand expression.

Returns:
  ASTNode* representing the full declaration.
*/
ASTNode* newDeclarationNode(ASTNode* variableDeclaration, ASTNode* expression);

/*
Creates a conditional node for if/else if/else statements.

Constructs a conditional block node based on the type of statement.

Args:
  type: One of AST_IF_STATEMENT, AST_ELSE_IF_STATEMENT, AST_ELSE_STATEMENT.
  condition: ASTNode for the condition (NULL for else).
  body: Block node for the statement body.

Returns:
  ASTNode* representing the conditional.
*/
ASTNode* newIfElifElseNode(ASTNodeType type, ASTNode* condition, ASTNode* body);

/*
Creates a node representing a while loop.

Constructs an AST node for the `while (condition) { body }` structure.

Args:
  condition: ASTNode for the loop condition.
  body: ASTNode for the loop body.

Returns:
  ASTNode* representing the loop.
*/
ASTNode* newWhileNode(ASTNode* condition, ASTNode* body);

/*
Checks whether the token is a data type keyword.

Useful in detecting whether a token starts a variable declaration.

Args:
  token: Pointer to the token.

Returns:
  1 if token is a data type, 0 otherwise.
*/
int isTokenDataType(Token* token);

/*
Returns the current token without advancing.

Peeks at the current token index.

Args:
  tokens: Array of tokens.
  index: Pointer to current index.

Returns:
  Token* at the current index.
*/
Token* peekToken(Token* tokens, const int* index);

/*
Peeks ahead by a number of tokens.

Used to look ahead during parsing without advancing the index.

Args:
  tokens: Array of tokens.
  index: Pointer to current index.
  forward: Number of tokens to look ahead.
  tokenCount: Total number of tokens.

Returns:
  Token* at the forward offset.
*/
Token* peekAheadToken(Token* tokens, const int* index, int forward,
                      int tokenCount);

/*
Parses a variable declaration from tokens.

Detects and builds an AST node for a type-name pair.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current token index.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* representing the variable declaration.
*/
ASTNode* parseVariableDeclaration(Token* tokens, int* tokenIndex,
                                  int tokenCount);

/*
Converts a token representing an integer literal to an int.

Safely parses a numeric token into a C integer.

Args:
  token: Token representing an integer.

Returns:
  int value parsed from token.
*/
int convertTokenToInt(Token* token);

/*
Parses either a variable or integer literal.

Handles basic expressions like identifiers or constants.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to token index.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* for the variable or literal.
*/
ASTNode* parseVariableOrLiteral(Token* tokens, int* tokenIndex, int tokenCount);

/*
Parses a full expression (e.g., binary expressions).

Implements parsing logic for simple expressions using binary operators.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current token index.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* representing the expression.
*/
ASTNode* parseExpression(Token* tokens, int* tokenIndex, int tokenCount);

/*
Parses a `while` loop statement.

Constructs the AST node representing a while loop and its body.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current index.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* for the while loop.
*/
ASTNode* parseWhileStatement(Token* tokens, int* tokenIndex, int tokenCount);

/*
Parses an `if`, `else if`, or `else` statement.

Handles conditional blocks and branching structures.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current index.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* representing the conditional.
*/
ASTNode* parseIfElifElseStatement(Token* tokens, int* tokenIndex,
                                  int tokenCount);

/*
Parses a general statement.

Dispatches based on the leading token to determine what kind of statement is
present.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current index.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* for the parsed statement.
*/
ASTNode* parseStatement(Token* tokens, int* tokenIndex, int tokenCount);

/*
Parses a function declaration from tokens.

Builds an AST node representing a function with parameters and a body.

Args:
  tokens: Array of tokens.
  tokenIndex: Pointer to current index.
  tokenCount: Total number of tokens.

Returns:
  ASTNode* for the function declaration.
*/
ASTNode* parseFunction(Token* tokens, int* tokenIndex, int tokenCount);

/*
Parses an entire file and returns an array of top-level AST nodes.

Processes all top-level constructs like functions.

Args:
  tokens: Array of tokens.
  tokenCount: Total number of tokens.

Returns:
  Array of ASTNode* representing the file's top-level structure.
*/
ASTNode** parseFile(Token* tokens, int tokenCount);

/*
Prints indentation spaces to an output stream.

Used to align AST pretty-printing.

Args:
  output: Output stream.
  indent: Number of indentation levels.

Returns:
  void
*/
static void printIndent(FILE* output, int indent);
