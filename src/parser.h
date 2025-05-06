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
  AST_INVALID,
  // Add more as needed...
} ast_node_type;

typedef struct ast_node {
  ast_node_type type;  // Helps identify which kind of node this is.
  // int line;         // Optional: storing line number for debugging or error
  // messages.
  union {
    // For integer literals.
    struct {
      int int_literal;
      Token* token;  // The token representing the integer literal.
    } int_literal;

    Token* variable_name;

    // For a variable or identifier.
    struct {
      Token* name;  // The name of the variable.
      Token* type;  // TODO (nividh): Rename to variabletype // Type of
                    // variable, e.g., "int", "float"
    } variable_declaration;

    // For binary expressions.
    struct {
      struct ast_node* left;
      TokenType _operator;  // You might use a char or a TokenType here.
      struct ast_node* right;
    } binary;

    struct {
      struct ast_node* condition;
      struct ast_node* body;
    } while_statement;

    struct {
      struct ast_node* condition;
      struct ast_node* body;
    } if_elif_else_statement;

    // For unary expressions.
    struct {
      char _operator;
      struct ast_node* operand;
    } unary;

    struct {
      Token* name;  // The name of the function.
      Token* return_type;
      struct ast_node** parameters;  // List of parameters (ASTNodes).
      int param_count;               // Number of parameters.
      struct ast_node* statements;  // Block for the statements in the function.
    } function;

    struct {
      Token* name;                   // The name of the function.
      struct ast_node** parameters;  // List of parameters (ASTNodes).
      int param_count;               // Number of parameters.
    } function_call;

    struct {
      struct ast_node* variable;
      struct ast_node* expression;
    } declaration;

    // For blocks (a list of statements).
    struct {
      struct ast_node** statements;
      int count;  // The number of statements in the block.
    } block;

    struct {
      struct ast_node* expression;
    } _return;

    // For an assignment, declaration, or other composite structures,
    // you can add additional fields or even nested structs here.
  } as;
} ast_node;
/*
Parses a function call node from the token stream.

Scans tokens starting at `*token_index` to parse a function call (e.g.,
`foo(arg1, arg2)`), constructing an AST node representing the function call.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current index in token array.
  token_count: Total number of tokens.

Returns:
  ast_node* representing the function call.
*/
ast_node* parse_function_call(Token* tokens, int* token_index, int token_count);

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
void print_ast(FILE* output, ast_node* node, int indent);

/*
Prints an array of AST nodes to a file.

Outputs all given AST nodes to a file named "ast" with indentation and
structure.

Args:
  nodes: Array of ast_node pointers.
  count: Number of AST nodes.

Returns:
  void
*/
void printASTFile(ast_node** nodes, int count);

/*
Prints the AST output to stdout or a file.

Iterates over the array of AST nodes and prints each one, optionally to a file.

Args:
  nodes: Array of ast_node pointers.
  count: Number of AST nodes.
  outputToFile: 1 to print to file, 0 to stdout.

Returns:
  void
*/
void print_ast_output(ast_node** nodes, int count, int output_to_file);

/*
Parses a block (compound statement) of code.

Parses a sequence of statements enclosed in `{}` or a single statement and
returns a block AST node.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current index in token array.
  token_count: Total number of tokens.

Returns:
  ast_node* representing the block.
*/
ast_node* parse_block(Token* tokens, int* token_index, int token_count);

/*
Creates a new AST node for an integer literal.

Constructs a node representing a constant integer.

Args:
  value: The integer value.
  token: Token representing the integer.

Returns:
  ast_node* representing the literal.
*/
ast_node* new_int_literal_node(int value, Token* token);

/*
Creates a new AST node for a variable.

Constructs a node representing a variable usage.

Args:
  name: Token representing the variable name.

Returns:
  ast_node* representing the variable.
*/
ast_node* new_variable_node(Token* name);

/*
Creates a new AST node for a variable declaration.

Constructs a node for declaring a variable with a type.

Args:
  name: Token for variable name.
  type: Token for variable type.

Returns:
  ast_node* representing the declaration.
*/
ast_node* new_variable_declaration_node(Token* name, Token* type);

/*
Creates a new binary expression node.

Constructs a node representing a binary operation (e.g., +, -, etc.) between two
subexpressions.

Args:
  left: Left-hand expression.
  operator: Operator token type.
  right: Right-hand expression.

Returns:
  ast_node* representing the binary expression.
*/
ast_node* new_binary_node(ast_node* left, TokenType operator, ast_node * right);

/*
Creates a new unary expression node.

Constructs a node for unary operations such as negation.

Args:
  operator: Operator character.
  operand: Operand node.

Returns:
  ast_node* representing the unary expression.
*/
ast_node* new_unary_node(char operator, ast_node * operand);

/*
Creates a new block node containing multiple statements.

Constructs a block node from an array of statement AST nodes.

Args:
  statements: Array of ast_node pointers.
  count: Number of statements.

Returns:
  ast_node* representing the block.
*/
ast_node* new_block_node(ast_node** statements, int count);

/*
Creates a new function declaration node.

Constructs an AST node representing a function definition with parameters and
body.

Args:
  name: Token for function name.
  return_type: Token for return type.
  parameters: Array of parameter ASTNodes.
  count: Number of parameters.
  statements: Block node for function body.

Returns:
  ast_node* representing the function.
*/
ast_node* new_function_node(Token* name, Token* return_type,
                            ast_node** parameters, int count,
                            ast_node* statements);

/*
Creates a return statement node.

Constructs a node representing a `return` expression.

Args:
  expression: ast_node for the return value.

Returns:
  ast_node* representing the return.
*/
ast_node* new_return_node(ast_node* expression);

/*
Creates a declaration node for a variable initialized with an expression.

Represents `int x = 3;` as a declaration node holding both variable and value.

Args:
  variableDeclaration: ast_node for the variable declaration.
  expression: ast_node for the right-hand expression.

Returns:
  ast_node* representing the full declaration.
*/
ast_node* new_declaration_node(ast_node* variable_declaration,
                               ast_node* expression);

/*
Creates a conditional node for if/else if/else statements.

Constructs a conditional block node based on the type of statement.

Args:
  type: One of AST_IF_STATEMENT, AST_ELSE_IF_STATEMENT, AST_ELSE_STATEMENT.
  condition: ast_node for the condition (NULL for else).
  body: Block node for the statement body.

Returns:
  ast_node* representing the conditional.
*/
ast_node* new_if_elif_else_node(ast_node_type type, ast_node* condition,
                                ast_node* body);

/*
Creates a node representing a while loop.

Constructs an AST node for the `while (condition) { body }` structure.

Args:
  condition: ast_node for the loop condition.
  body: ast_node for the loop body.

Returns:
  ast_node* representing the loop.
*/
ast_node* new_while_node(ast_node* condition, ast_node* body);

/*
Checks whether the token is a data type keyword.

Useful in detecting whether a token starts a variable declaration.

Args:
  token: Pointer to the token.

Returns:
  1 if token is a data type, 0 otherwise.
*/
int is_token_data_type(Token* token);

/*
Returns the current token without advancing.

Peeks at the current token index.

Args:
  tokens: Array of tokens.
  index: Pointer to current index.

Returns:
  Token* at the current index.
*/
Token* peek_token(Token* tokens, const int* index);

/*
Peeks ahead by a number of tokens.

Used to look ahead during parsing without advancing the index.

Args:
  tokens: Array of tokens.
  index: Pointer to current index.
  forward: Number of tokens to look ahead.
  token_count: Total number of tokens.

Returns:
  Token* at the forward offset.
*/
Token* peek_ahead_token(Token* tokens, const int* index, int forward,
                        int token_count);

/*
Parses a variable declaration from tokens.

Detects and builds an AST node for a type-name pair.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current token index.
  token_count: Total number of tokens.

Returns:
  ast_node* representing the variable declaration.
*/
ast_node* parse_variable_declaration(Token* tokens, int* token_index,
                                     int token_count);

/*
Converts a token representing an integer literal to an int.

Safely parses a numeric token into a C integer.

Args:
  token: Token representing an integer.

Returns:
  int value parsed from token.
*/
int convert_token_to_int(Token* token);

/*
Parses either a variable or integer literal.

Handles basic expressions like identifiers or constants.

Args:
  tokens: Array of tokens.
  token_index: Pointer to token index.
  token_count: Total number of tokens.

Returns:
  ast_node* for the variable or literal.
*/
ast_node* parse_variable_or_literal(Token* tokens, int* token_index,
                                    int token_count);

/*
Parses a full expression (e.g., binary expressions).

Implements parsing logic for simple expressions using binary operators.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current token index.
  token_count: Total number of tokens.

Returns:
  ast_node* representing the expression.
*/
ast_node* parse_expression(Token* tokens, int* token_index, int token_count);

/*
Parses a `while` loop statement.

Constructs the AST node representing a while loop and its body.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current index.
  token_count: Total number of tokens.

Returns:
  ast_node* for the while loop.
*/
ast_node* parse_while_statement(Token* tokens, int* token_index,
                                int token_count);

/*
Parses an `if`, `else if`, or `else` statement.

Handles conditional blocks and branching structures.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current index.
  token_count: Total number of tokens.

Returns:
  ast_node* representing the conditional.
*/
ast_node* parse_if_elif_else_statement(Token* tokens, int* token_index,
                                       int token_count);

/*
Parses a general statement.

Dispatches based on the leading token to determine what kind of statement is
present.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current index.
  token_count: Total number of tokens.

Returns:
  ast_node* for the parsed statement.
*/
ast_node* parse_statement(Token* tokens, int* token_index, int token_count);

/*
Parses a function declaration from tokens.

Builds an AST node representing a function with parameters and a body.

Args:
  tokens: Array of tokens.
  token_index: Pointer to current index.
  token_count: Total number of tokens.

Returns:
  ast_node* for the function declaration.
*/
ast_node* parse_function(Token* tokens, int* token_index, int token_count);

/*
Parses an entire file and returns an array of top-level AST nodes.

Processes all top-level constructs like functions.

Args:
  tokens: Array of tokens.
  token_count: Total number of tokens.

Returns:
  Array of ast_node* representing the file's top-level structure.
*/
ast_node** parse_file(Token* tokens, int token_count);
