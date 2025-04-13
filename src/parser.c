#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

// ASTNode constructors with added debug prints.

ASTNode* newIntLiteralNode(int value) {
  printf("Debug: Creating new IntLiteral node with value = %d\n", value);
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newIntLiteralNode\n");
    exit(1);
  }
  node->type = AST_INT_LITERAL;
  node->as.intLiteral = value;
  return node;
}

ASTNode* newVariableNode(Token* name) {
  printf("Debug: Creating new Variable node. Name: %.*s\n", name->length,
         name->lexeme);

  ASTNode* node = malloc(sizeof(ASTNode));

  if (!node) {
    fprintf(stderr, "Error: Out of memory in newVariableNode\n");
    exit(1);
  }
  node->type = AST_VARIABLE;  // Using the variable type for declarations.
  node->as.variableName = name;
  return node;
}

ASTNode* newVariableDeclorationNode(Token* name, Token* type) {
  printf(
      "Debug: Creating new VariableDecloration node. Name: %.*s, Type: %.*s\n",
      name->length, name->lexeme, type->length, type->lexeme);
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newVariableDeclorationNode\n");
    exit(1);
  }
  node->type =
      AST_VARIABLE_DECLORATION;  // Using the variable type for declarations.
  node->as.variable_decloration.name = name;
  node->as.variable_decloration.type = type;
  return node;
}

ASTNode* newBinaryNode(ASTNode* left, TokenType operator, ASTNode * right) {
  printf("Debug: Creating new Binary node with operator '%s'\n",
         tokenTypeToString(operator));
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newBinaryNode\n");
    exit(1);
  }
  node->type = AST_BINARY;
  node->as.binary.left = left;
  node->as.binary._operator = operator;
  node->as.binary.right = right;
  return node;
}

ASTNode* newUnaryNode(char operator, ASTNode * operand) {
  printf("Debug: Creating new Unary node with operator '%c'\n", operator);
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newUnaryNode\n");
    exit(1);
  }
  node->type = AST_UNARY;
  node->as.unary._operator = operator;
  node->as.unary.operand = operand;
  return node;
}

ASTNode* newBlockNode(ASTNode** statements, int count) {
  printf("Debug: Creating new Block node with %d statement(s)\n", count);
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newBlockNode\n");
    exit(1);
  }
  node->type = AST_BLOCK;
  node->as.block.statements = statements;
  node->as.block.count = count;
  return node;
}

ASTNode* newFunctionNode(Token* name, Token* returnType, ASTNode** parameters,
                         int count, ASTNode** statements, int statementCount,
                         ASTNode* returnStatement) {
  printf("Debug: Creating new Function node. Name: %.*s, Return Type: %.*s\n",
         name->length, name->lexeme, returnType->length, returnType->lexeme);
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newFunctionNode\n");
    exit(1);
  }
  node->type = AST_FUNCTION_DECLARATION;
  node->as.function.name = name;
  node->as.function.returnType = returnType;
  node->as.function.parameters = parameters;
  node->as.function.paramCount = count;
  node->as.function.statements = statements;
  node->as.function.statementCount = statementCount;
  node->as.function.returnStatement = returnStatement;
  return node;
}

ASTNode* newDeclarationNode(ASTNode* variableDecloration, ASTNode* expression) {
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newDeclarationNode\n");
    exit(1);
  }
  node->type = AST_DECLARATION;
  node->as.declaration.variable = variableDecloration;
  node->as.declaration.expression = expression;
  return node;
}

//////////////////
// Helper functions for the parser

int isTokenDataType(Token* token) {
  printf("Debug: Checking if token is data type: ");
  printToken(token);
  if (token->type == TOKEN_INT_TYPE || token->type == TOKEN_VOID_TYPE) {
    return 1;
  }
  return 0;
}

Token* peekToken(Token* tokens, int* index) {
  printf("In peekToken\n");
  return &tokens[(*index)];
}

Token* peekAheadToken(Token* tokens, int* index, int forward, int tokenCount) {
  printf("Debug: peekAheadToken at index %d and forward = %d\n", *index,
         forward);
  if ((*index) + forward >= tokenCount) {
    return NULL;
  }
  return &tokens[(*index) + forward];
}
//////////////////
// Parser functions

ASTNode* parseVariableDecloration(Token* tokens, int* tokenIndex,
                                  int tokenCount) {
  printf("Debug: Entering parseVariableDecloration at tokenIndex = %d\n",
         *tokenIndex);

  // Expect a data type token first.
  if (!isTokenDataType(peekToken(tokens, tokenIndex))) {
    fprintf(stderr, "Error: Expected a data type\n");
    return NULL;
  }
  printf("Debug: Data type token: ");
  printToken(peekToken(tokens, tokenIndex));
  Token* type = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Check for identifier token.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_IDENTIFIER) {
    fprintf(stderr, "Error: Expected an identifier\n");
    return NULL;
  }
  printf("Debug: Identifier token: ");
  printToken(peekToken(tokens, tokenIndex));
  Token* name = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  return newVariableDeclorationNode(name, type);
}

int isVariableOrLiteral(Token* token) {
  printf("Debug: isVariableOrLiteral at tokenIndex = %d\n");
  if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_INT_LITERAL) {
    return 1;
  }
  return 0;
}

int convertTokenToInt(Token* token) {
  printf("Debug: convertTokenToInt at tokenIndex = %d\n");

  // int convertSubstringToInt(const char *str, size_t len) {
  // Allocate memory for a null-terminated string copy of the substring.
  char* buf = malloc(token->length + 1);
  if (!buf) {
    fprintf(stderr, "Error: Out of memory\n");
    return 0;  // Or use another error signal
  }

  // Copy the substring and add a null terminator.
  memcpy(buf, token->lexeme, token->length);
  buf[token->length] = '\0';

  // Use strtol for conversion with error checking.
  char* endptr;
  errno = 0;  // Clear errno before conversion
  long value = strtol(buf, &endptr, 10);

  // Clean up the temporary buffer.
  free(buf);

  // Check for conversion errors.
  if (endptr == buf) {
    fprintf(stderr, "Error: No digits found in substring\n");
    return 0;
  }
  if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
    fprintf(stderr, "Error: Number out of range\n");
    return 0;
  }

  return (int)value;
}

ASTNode* parseVariableOrLiteral(Token* tokens, int* tokenIndex,
                                int tokenCount) {
  printf("Debug: Entering parseVariableOrLiteral at tokenIndex = %d\n",
         *tokenIndex);
  if (peekToken(tokens, tokenIndex)->type == TOKEN_IDENTIFIER) {
    printf("Is identifier\n");
    ASTNode* node = newVariableNode(peekToken(tokens, tokenIndex));
    (*tokenIndex)++;
    return node;
  } else if (peekToken(tokens, tokenIndex)->type == TOKEN_INT_LITERAL) {
    printf("Is Int\n");
    ASTNode* node =
        newIntLiteralNode(convertTokenToInt(peekToken(tokens, tokenIndex)));
    (*tokenIndex)++;
    return node;
  }
}

ASTNode* parseExpression(Token* tokens, int* tokenIndex, int tokenCount) {
  printf("Debug: Entering parseExpression at tokenIndex = %d\n", *tokenIndex);
  // For now, this is a placeholder.
  // A complete implementation would parse an expression, possibly using
  // recursive descent.
  if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type == TOKEN_RPAREN ||
      peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type ==
          TOKEN_SEMICOLON) {
    printf("Debug: No expression, just value\n");

    return parseVariableOrLiteral(tokens, tokenIndex, tokenCount);
  }

  if (peekToken(tokens, tokenIndex)->type == TOKEN_LPAREN) {
    // Deal with this later
  }
  if (isVariableOrLiteral(peekToken(tokens, tokenIndex)) == 1) {
    ASTNode* leftSide = parseVariableOrLiteral(tokens, tokenIndex, tokenCount);
    TokenType _operator = peekToken(tokens, tokenIndex)->type;
    (*tokenIndex)++;

    return newBinaryNode(leftSide, _operator,
                         parseExpression(tokens, tokenIndex, tokenCount));
  }
}

ASTNode* parseStatement(Token* tokens, int* tokenIndex, int tokenCount) {
  printf("Debug: Entering parseStatement at tokenIndex = %d\n", *tokenIndex);

  // If the token represents the start of a variable declaration:
  if (isTokenDataType(peekToken(tokens, tokenIndex))) {
    ASTNode* variableDeclorationNode =
        parseVariableDecloration(tokens, tokenIndex, tokenCount);
    if (variableDeclorationNode == NULL) {
      fprintf(stderr, "Error: Failed to parse variable declaration\n");
      return NULL;
    }
    // Check if there is an assignment following the declaration.
    if (peekAheadToken(tokens, tokenIndex, 0, tokenCount) != NULL &&
        peekAheadToken(tokens, tokenIndex, 0, tokenCount)->type !=
            TOKEN_ASSIGN) {
      (*tokenIndex)++;
      printf("Debug: No assignment operator found after variable declaration");
      return variableDeclorationNode;
    }

    // Otherwise, if an assignment operator is present, parse the expression.
    (*tokenIndex)++;
    ASTNode* expression = parseExpression(tokens, tokenIndex, tokenCount);
    if (expression == NULL) {
      fprintf(stderr, "Error: Failed to parse expression\n");
      return NULL;
    }
    (*tokenIndex)++;
    return newDeclarationNode(variableDeclorationNode, expression);
  }

  // Placeholder for other types of statements.
  return NULL;
}

ASTNode* parseFunction(Token* tokens, int* tokenIndex, int tokenCount) {
  printf("Debug: Entering parseFunction at tokenIndex = %d\n", *tokenIndex);

  Token* name;
  Token* returnType;
  ASTNode** parameters = malloc(100 * sizeof(ASTNode*));
  ASTNode** statements = malloc(100 * sizeof(ASTNode*));
  int parameterCount = 0;
  int statementCount = 0;

  // Parse return type.
  printf("Debug: Parsing function return type token at index %d: ",
         *tokenIndex);
  printToken(peekToken(tokens, tokenIndex));
  returnType = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Parse function name.
  printf("Debug: Parsing function name token at index %d: ", *tokenIndex);
  printToken(peekToken(tokens, tokenIndex));
  name = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Ensure a '(' token follows.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_LPAREN) {
    fprintf(stderr, "Error: Expected '(' after function name\n");
    return NULL;
  }
  printf("Debug: Found '(' token: ");
  printToken(peekToken(tokens, tokenIndex));
  (*tokenIndex)++;

  // Parse parameters until a ')' token is found.
  while (peekToken(tokens, tokenIndex)->type != TOKEN_RPAREN) {
    printf("Debug: Parsing parameter %d at tokenIndex = %d: ",
           parameterCount + 1, *tokenIndex);
    printToken(peekToken(tokens, tokenIndex));
    parameters[parameterCount++] =
        parseVariableDecloration(tokens, tokenIndex, tokenCount);
    if (peekToken(tokens, tokenIndex)->type == TOKEN_COMMA) {
      printf("Debug: Found comma token between parameters: ");
      printToken(peekToken(tokens, tokenIndex));
      (*tokenIndex)++;
    }
  }
  // Skip the closing ')'
  printf("Debug: Found ')' token for parameter list: ");
  printToken(peekToken(tokens, tokenIndex));
  (*tokenIndex)++;

  // Check for '{' to begin the function body.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_LBRACE) {
    fprintf(stderr, "Error: Expected '{' after function parameters\n");
    return NULL;
  }
  printf("Debug: Found '{' token for function body: ");
  printToken(peekToken(tokens, tokenIndex));
  (*tokenIndex)++;

  ASTNode* returnNode = NULL;

  // Parse function body statements until a '}' is encountered.
  while (peekToken(tokens, tokenIndex)->type != TOKEN_RBRACE) {
    printf("Debug: Parsing statement %d at tokenIndex = %d: ",
           statementCount + 1, *tokenIndex);
    printToken(peekToken(tokens, tokenIndex));

    if (peekToken(tokens, tokenIndex)->type == TOKEN_RETURN) {
      printf("Debug: Found 'return' keyword\n");
      (*tokenIndex)++;
      returnNode = parseExpression(tokens, tokenIndex, tokenCount);
      break;
    }

    statements[statementCount++] =
        parseStatement(tokens, tokenIndex, tokenCount);
  }
  printf("Debug: Found '}' token ending function body: ");
  printToken(peekToken(tokens, tokenIndex));
  (*tokenIndex)++;  // Skip the closing brace

  printf("Debug: Finished parsing function '%.*s'\n", name->length,
         name->lexeme);
  return newFunctionNode(name, returnType, parameters, parameterCount,
                         statements, statementCount, returnNode);
}

ASTNode** parseFile(Token* tokens, int tokenCount) {
  printf("Debug: Entering parseFile. Total tokens: %d\n", tokenCount);
  int tokenIndex = 0;
  ASTNode** astNodes = malloc(100 * sizeof(ASTNode*));
  for (int i = 0; i < 100; i++) {
    astNodes[i] = NULL;
  }
  int astNodesIndex = 0;

  // Loop until end-of-file token is reached.
  while (tokenIndex < tokenCount) {
    if (peekToken(tokens, &tokenIndex)->type == TOKEN_EOF) {
      break;
    }
    if (isTokenDataType(peekToken(tokens, &tokenIndex)) == 1) {
      printf("Is data type\n");
      if (peekAheadToken(tokens, &tokenIndex, 1, tokenCount)->type ==
          TOKEN_IDENTIFIER) {
        printf("Is identifier\n");
        if (peekAheadToken(tokens, &tokenIndex, 2, tokenCount)->type ==
            TOKEN_LPAREN) {
          printf("Is function\n");
          printf("Debug: Parsing function starting at tokenIndex %d\n",
                 tokenIndex);
          astNodes[astNodesIndex++] =
              parseFunction(tokens, &tokenIndex, tokenCount);
          continue;
        }
      }
    }
    tokenIndex++;  // Avoid infinite loop if no matching constructs are found.
  }
  return astNodes;
}
#include <stdio.h>

// Helper function to print indentation.
static void printIndent(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("  ");  // Two spaces per indent level.
  }
}

// Recursively prints a single AST node.
void printAST(ASTNode* node, int indent) {
  if (node == NULL) {
    printIndent(indent);
    printf("NULL\n");
    return;
  }
  printIndent(indent);
  switch (node->type) {
    case AST_INT_LITERAL:
      printf("IntLiteral: %d\n", node->as.intLiteral);
      break;
    case AST_VARIABLE_DECLORATION:
      // Here we check whether this is a declaration-based variable or a plain
      // variable.
      if (node->as.variable_decloration.type != NULL) {
        printf("Variable Declaration: %.*s of type %.*s\n",
               node->as.variable_decloration.name->length,
               node->as.variable_decloration.name->lexeme,
               node->as.variable_decloration.type->length,
               node->as.variable_decloration.type->lexeme);
      } else {
        printf("Variable: %.*s\n", node->as.variableName->length,
               node->as.variableName->lexeme);
      }
      break;
    case AST_VARIABLE:
      printf("Variable: %.*s\n", node->as.variableName->length,
             node->as.variableName->lexeme);
      break;
    case AST_BINARY:
      printf("Binary Expression: '%s'\n",
             tokenTypeToString(node->as.binary._operator));
      printIndent(indent + 1);
      printf("Left:\n");
      printAST(node->as.binary.left, indent + 2);
      printIndent(indent + 1);
      printf("Right:\n");
      printAST(node->as.binary.right, indent + 2);
      break;
    case AST_UNARY:
      printf("Unary Expression: '%c'\n", node->as.unary._operator);
      printIndent(indent + 1);
      printf("Operand:\n");
      printAST(node->as.unary.operand, indent + 2);
      break;
    case AST_ASSIGNMENT:
      printf("Assignment -- details not implemented.\n");
      break;
    case AST_EXPRESSION_STATEMENT:
      printf("Expression Statement -- details not implemented.\n");
      break;
    case AST_DECLARATION:
      printf("Declaration:\n");
      printIndent(indent + 1);
      printf("Variable Declaration:\n");
      printAST(node->as.declaration.variable, indent + 2);
      printIndent(indent + 1);
      printf("Expression:\n");
      printAST(node->as.declaration.expression, indent + 2);
      break;
    case AST_FUNCTION_DECLARATION:
      printf("Function Declaration: %.*s returns %.*s\n",
             node->as.function.name->length, node->as.function.name->lexeme,
             node->as.function.returnType->length,
             node->as.function.returnType->lexeme);
      printIndent(indent + 1);
      printf("Parameters (%d):\n", node->as.function.paramCount);
      for (int i = 0; i < node->as.function.paramCount; i++) {
        printAST(node->as.function.parameters[i], indent + 2);
      }
      printIndent(indent + 1);
      printf("Body Statements (%d):\n", node->as.function.statementCount);
      for (int i = 0; i < node->as.function.statementCount; i++) {
        printAST(node->as.function.statements[i], indent + 2);
      }
      printIndent(indent + 1);
      printf("Return expression");
      printAST(node->as.function.returnStatement, indent + 2);
      break;
    case AST_IF_STATEMENT:
      printf("If Statement -- details not implemented.\n");
      break;
    case AST_WHILE_STATEMENT:
      printf("While Statement -- details not implemented.\n");
      break;
    case AST_BLOCK:
      printf("Block with %d statement(s):\n", node->as.block.count);
      for (int i = 0; i < node->as.block.count; i++) {
        printAST(node->as.block.statements[i], indent + 1);
      }
      break;
    default:
      printf("Unknown AST Node\n");
      break;
  }
}

// New function to print the entire file's AST.
// 'nodes' is an array of ASTNode pointers and 'count' is the total number
// allocated.
void printASTFile(ASTNode** nodes, int count) {
  printf("Printing AST for the entire file:\n");
  for (int i = 0; i < count; i++) {
    if (nodes[i] != NULL) {
      printf("\n--- AST Node %d ---\n", i);
      printAST(nodes[i], 0);
    }
  }
}
