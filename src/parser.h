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
  AST_EXPRESSION_STATEMENT,
  AST_DECLARATION,
  AST_FUNCTION_DECLARATION,
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
    int intLiteral;

    Token* variableName;

    // For a variable or identifier.
    struct {
      Token* name;  // The name of the variable.
      Token* type;  // Type of variable, e.g., "int", "float"
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

ASTNode** parseFile(Token* tokens, int tokenCount);

ASTNode* newIntLiteralNode(int value);

void printAST(FILE* file, ASTNode* node, int indent);
void printASTFile(ASTNode** nodes, int count);
void printASTOutput(ASTNode** nodes, int count, int outputToFile);
ASTNode* parseBlock(Token* tokens, int* tokenIndex, int tokenCount);
