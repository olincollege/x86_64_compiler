#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

// ASTNode constructors with added debug prints.

ASTNode* newIntLiteralNode(int value) {
#ifdef DEBUG

  printf("Debug: Creating new IntLiteral node with value = %d\n", value);

#endif

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
#ifdef DEBUG

  printf("Debug: Creating new Variable node. Name: %.*s\n", name->length,
         name->lexeme);

#endif

  ASTNode* node = malloc(sizeof(ASTNode));

  if (!node) {
    fprintf(stderr, "Error: Out of memory in newVariableNode\n");
    exit(1);
  }
  node->type = AST_VARIABLE;  // Using the variable type for declarations.
  node->as.variableName = name;
  return node;
}

ASTNode* newVariableDeclarationNode(Token* name, Token* type) {
#ifdef DEBUG

  printf(
      "Debug: Creating new VariableDeclaration node. Name: %.*s, Type: %.*s\n",
      name->length, name->lexeme, type->length, type->lexeme);
#endif

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newVariableDeclarationNode\n");
    exit(1);
  }
  node->type =
      AST_VARIABLE_DECLORATION;  // Using the variable type for declarations.
  node->as.variable_declaration.name = name;
  node->as.variable_declaration.type = type;
  return node;
}

ASTNode* newBinaryNode(ASTNode* left, TokenType operator, ASTNode * right) {
#ifdef DEBUG

  printf("Debug: Creating new Binary node with operator '%s'\n",
         tokenTypeToString(operator));
#endif

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
#ifdef DEBUG

  printf("Debug: Creating new Unary node with operator '%c'\n", operator);
#endif

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
#ifdef DEBUG

  printf("Debug: Creating new Block node with %d statement(s)\n", count);
#endif

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
#ifdef DEBUG

  printf("Debug: Creating new Function node. Name: %.*s, Return Type: %.*s\n",
         name->length, name->lexeme, returnType->length, returnType->lexeme);
#endif

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

ASTNode* newDeclarationNode(ASTNode* variableDeclaration, ASTNode* expression) {
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newDeclarationNode\n");
    exit(1);
  }
  node->type = AST_DECLARATION;
  node->as.declaration.variable = variableDeclaration;
  node->as.declaration.expression = expression;
  return node;
}

//////////////////
// Helper functions for the parser

int isTokenDataType(Token* token) {
#ifdef DEBUG

  printf("Debug: Checking if token is data type: ");
  printToken(token);

#endif

  if (token->type == TOKEN_INT_TYPE || token->type == TOKEN_VOID_TYPE) {
    return 1;
  }
  return 0;
}

Token* peekToken(Token* tokens, int* index) {
#ifdef DEBUG

  printf("Debug: peekToken at index %d\n", *index);
#endif

  return &tokens[(*index)];
}

Token* peekAheadToken(Token* tokens, int* index, int forward, int tokenCount) {
#ifdef DEBUG

  printf("Debug: peekAheadToken at index %d and forward = %d\n", *index,
         forward);
#endif

  if ((*index) + forward >= tokenCount) {
    return NULL;
  }
  return &tokens[(*index) + forward];
}
//////////////////
// Parser functions

ASTNode* parseVariableDeclaration(Token* tokens, int* tokenIndex,
                                  int tokenCount) {
#ifdef DEBUG

  printf("Debug: Entering parseVariableDeclaration at tokenIndex = %d\n",
         *tokenIndex);
#endif

  // Expect a data type token first.
  if (!isTokenDataType(peekToken(tokens, tokenIndex))) {
    fprintf(stderr, "Error: Expected a data type\n");
    return NULL;
  }
#ifdef DEBUG

  printf("Debug: Data type token: ");
  printToken(peekToken(tokens, tokenIndex));

#endif

  Token* type = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Check for identifier token.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_IDENTIFIER) {
    fprintf(stderr, "Error: Expected an identifier\n");
    return NULL;
  }
#ifdef DEBUG

  printf("Debug: Identifier token: ");
  printToken(peekToken(tokens, tokenIndex));

#endif

  Token* name = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  return newVariableDeclarationNode(name, type);
}

int isVariableOrLiteral(Token* token) {
#ifdef DEBUG

  printf("Debug: isVariableOrLiteral at tokenIndex = %d\n");
#endif

  if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_INT_LITERAL) {
    return 1;
  }
  return 0;
}

int convertTokenToInt(Token* token) {
#ifdef DEBUG

  printf("Debug: convertTokenToInt at tokenIndex = %d\n");
#endif

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
#ifdef DEBUG

  printf("Debug: Entering parseVariableOrLiteral at tokenIndex = %d\n",
         *tokenIndex);
#endif

  if (peekToken(tokens, tokenIndex)->type == TOKEN_IDENTIFIER) {
    ASTNode* node = newVariableNode(peekToken(tokens, tokenIndex));
    (*tokenIndex)++;
    return node;
  } else if (peekToken(tokens, tokenIndex)->type == TOKEN_INT_LITERAL) {
    ASTNode* node =
        newIntLiteralNode(convertTokenToInt(peekToken(tokens, tokenIndex)));
    (*tokenIndex)++;
    return node;
  }
}

ASTNode* parseExpression(Token* tokens, int* tokenIndex, int tokenCount) {
#ifdef DEBUG

  printf("Debug: Entering parseExpression at tokenIndex = %d\n", *tokenIndex);
#endif

  // For now, this is a placeholder.
  // A complete implementation would parse an expression, possibly using
  // recursive descent.
  if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type == TOKEN_RPAREN ||
      peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type ==
          TOKEN_SEMICOLON) {
#ifdef DEBUG

    printf("Debug: No expression, just value\n");

#endif

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
#ifdef DEBUG

  printf("Debug: Entering parseStatement at tokenIndex = %d\n", *tokenIndex);
#endif

  // If the token represents the start of a variable declaration:
  if (isTokenDataType(peekToken(tokens, tokenIndex))) {
    ASTNode* variableDeclarationNode =
        parseVariableDeclaration(tokens, tokenIndex, tokenCount);
    if (variableDeclarationNode == NULL) {
      fprintf(stderr, "Error: Failed to parse variable declaration\n");
      return NULL;
    }
    // Check if there is an assignment following the declaration.
    if (peekAheadToken(tokens, tokenIndex, 0, tokenCount) != NULL &&
        peekAheadToken(tokens, tokenIndex, 0, tokenCount)->type !=
            TOKEN_ASSIGN) {
      (*tokenIndex)++;
#ifdef DEBUG

      printf("Debug: No assignment operator found after variable declaration");
#endif

      return variableDeclarationNode;
    }

    // Otherwise, if an assignment operator is present, parse the expression.
    (*tokenIndex)++;
    ASTNode* expression = parseExpression(tokens, tokenIndex, tokenCount);
    if (expression == NULL) {
      fprintf(stderr, "Error: Failed to parse expression\n");
      return NULL;
    }
    (*tokenIndex)++;
    return newDeclarationNode(variableDeclarationNode, expression);
  }
  if (peekToken(tokens, tokenIndex)->type == TOKEN_RETURN) {
#ifdef DEBUG

    printf("Debug: Found 'return' keyword\n");
#endif

    (*tokenIndex)++;
    ASTNode* returnNode = parseExpression(tokens, tokenIndex, tokenCount);
    return returnNode;
  }

  // Placeholder for other types of statements.
  return NULL;
}

ASTNode* parseFunction(Token* tokens, int* tokenIndex, int tokenCount) {
#ifdef DEBUG

  printf("Debug: Entering parseFunction at tokenIndex = %d\n", *tokenIndex);
#endif

  Token* name;
  Token* returnType;
  ASTNode** parameters = malloc(100 * sizeof(ASTNode*));
  ASTNode** statements = malloc(100 * sizeof(ASTNode*));
  int parameterCount = 0;
  int statementCount = 0;

#ifdef DEBUG

  // Parse return type.
  printf("Debug: Parsing function return type token at index %d: ",
         *tokenIndex);
  printToken(peekToken(tokens, tokenIndex));

#endif

  returnType = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

#ifdef DEBUG

  // Parse function name.
  printf("Debug: Parsing function name token at index %d: ", *tokenIndex);
  printToken(peekToken(tokens, tokenIndex));

#endif

  name = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Ensure a '(' token follows.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_LPAREN) {
    fprintf(stderr, "Error: Expected '(' after function name\n");
    return NULL;
  }
#ifdef DEBUG

  printf("Debug: Found '(' token: ");
  printToken(peekToken(tokens, tokenIndex));

#endif

  (*tokenIndex)++;

  // Parse parameters until a ')' token is found.
  while (peekToken(tokens, tokenIndex)->type != TOKEN_RPAREN) {
#ifdef DEBUG

    printf("Debug: Parsing parameter %d at tokenIndex = %d: ",
           parameterCount + 1, *tokenIndex);
    printToken(peekToken(tokens, tokenIndex));

#endif

    parameters[parameterCount++] =
        parseVariableDeclaration(tokens, tokenIndex, tokenCount);
    if (peekToken(tokens, tokenIndex)->type == TOKEN_COMMA) {
#ifdef DEBUG

      printf("Debug: Found comma token between parameters: ");
      printToken(peekToken(tokens, tokenIndex));

#endif

      (*tokenIndex)++;
    }
  }
#ifdef DEBUG

  // Skip the closing ')'
  printf("Debug: Found ')' token for parameter list: ");
  printToken(peekToken(tokens, tokenIndex));

#endif

  (*tokenIndex)++;

  // Check for '{' to begin the function body.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_LBRACE) {
    fprintf(stderr, "Error: Expected '{' after function parameters\n");
    return NULL;
  }

#ifdef DEBUG
  printf("Debug: Found '{' token for function body: ");
  printToken(peekToken(tokens, tokenIndex));
#endif

  (*tokenIndex)++;

  ASTNode* returnNode = NULL;

  // Parse function body statements until a '}' is encountered.
  while (peekToken(tokens, tokenIndex)->type != TOKEN_RBRACE) {
#ifdef DEBUG
    printf("Debug: Parsing statement %d at tokenIndex = %d: ",
           statementCount + 1, *tokenIndex);
    printToken(peekToken(tokens, tokenIndex));
#endif

    statements[statementCount++] =
        parseStatement(tokens, tokenIndex, tokenCount);
  }

#ifdef DEBUG
  printf("Debug: Found '}' token ending function body: ");
  printToken(peekToken(tokens, tokenIndex));

#endif

  (*tokenIndex)++;  // Skip the closing brace

#ifdef DEBUG

  printf("Debug: Finished parsing function '%.*s'\n", name->length,
         name->lexeme);
#endif

  return newFunctionNode(name, returnType, parameters, parameterCount,
                         statements, statementCount, returnNode);
}

ASTNode** parseFile(Token* tokens, int tokenCount) {
#ifdef DEBUG

  printf("Debug: Entering parseFile. Total tokens: %d\n", tokenCount);
#endif

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
#ifdef DEBUG

      printf("Is data type\n");
#endif

      if (peekAheadToken(tokens, &tokenIndex, 1, tokenCount)->type ==
          TOKEN_IDENTIFIER) {
#ifdef DEBUG

        printf("Is identifier\n");
#endif

        if (peekAheadToken(tokens, &tokenIndex, 2, tokenCount)->type ==
            TOKEN_LPAREN) {
#ifdef DEBUG
          printf("Debug: Parsing function starting at tokenIndex %d\n",
                 tokenIndex);
#endif

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
#include <stdlib.h>

// Assuming ASTNode and its related definitions are provided elsewhere
// For example, AST_INT_LITERAL, AST_VARIABLE_DECLORATION, etc.

// Helper function to print indentation to the given output stream.
static void printIndent(FILE* output, int indent) {
  for (int i = 0; i < indent; i++) {
    fprintf(output, "  ");  // Two spaces per indent level.
  }
}

// Recursively prints a single AST node to the given output stream.
void printAST(FILE* output, ASTNode* node, int indent) {
  if (node == NULL) {
    printIndent(output, indent);
    fprintf(output, "NULL\n");
    return;
  }

  printIndent(output, indent);
  switch (node->type) {
    case AST_INT_LITERAL:
      fprintf(output, "IntLiteral: %d\n", node->as.intLiteral);
      break;
    case AST_VARIABLE_DECLORATION:
      // Check if this is a variable declaration with a type.
      if (node->as.variable_declaration.type != NULL) {
        fprintf(output, "Variable Declaration: %.*s of type %.*s\n",
                node->as.variable_declaration.name->length,
                node->as.variable_declaration.name->lexeme,
                node->as.variable_declaration.type->length,
                node->as.variable_declaration.type->lexeme);
      } else {
        fprintf(output, "Variable: %.*s\n", node->as.variableName->length,
                node->as.variableName->lexeme);
      }
      break;
    case AST_VARIABLE:
      fprintf(output, "Variable: %.*s\n", node->as.variableName->length,
              node->as.variableName->lexeme);
      break;
    case AST_BINARY:
      fprintf(output, "Binary Expression: '%s'\n",
              tokenTypeToString(node->as.binary._operator));
      printIndent(output, indent + 1);
      fprintf(output, "Left:\n");
      printAST(output, node->as.binary.left, indent + 2);
      printIndent(output, indent + 1);
      fprintf(output, "Right:\n");
      printAST(output, node->as.binary.right, indent + 2);
      break;
    case AST_UNARY:
      fprintf(output, "Unary Expression: '%c'\n", node->as.unary._operator);
      printIndent(output, indent + 1);
      fprintf(output, "Operand:\n");
      printAST(output, node->as.unary.operand, indent + 2);
      break;
    case AST_ASSIGNMENT:
      fprintf(output, "Assignment -- details not implemented.\n");
      break;
    case AST_EXPRESSION_STATEMENT:
      fprintf(output, "Expression Statement -- details not implemented.\n");
      break;
    case AST_DECLARATION:
      fprintf(output, "Declaration:\n");
      printIndent(output, indent + 1);
      fprintf(output, "Variable Declaration:\n");
      printAST(output, node->as.declaration.variable, indent + 2);
      printIndent(output, indent + 1);
      fprintf(output, "Expression:\n");
      printAST(output, node->as.declaration.expression, indent + 2);
      break;
    case AST_FUNCTION_DECLARATION:
      fprintf(output, "Function Declaration: %.*s returns %.*s\n",
              node->as.function.name->length, node->as.function.name->lexeme,
              node->as.function.returnType->length,
              node->as.function.returnType->lexeme);
      printIndent(output, indent + 1);
      fprintf(output, "Parameters (%d):\n", node->as.function.paramCount);
      for (int i = 0; i < node->as.function.paramCount; i++) {
        printAST(output, node->as.function.parameters[i], indent + 2);
      }
      printIndent(output, indent + 1);
      fprintf(output, "Body Statements (%d):\n",
              node->as.function.statementCount);
      for (int i = 0; i < node->as.function.statementCount; i++) {
        printAST(output, node->as.function.statements[i], indent + 2);
      }
      printIndent(output, indent + 1);
      fprintf(output, "Return expression:\n");
      printAST(output, node->as.function.returnStatement, indent + 2);
      break;
    case AST_IF_STATEMENT:
      fprintf(output, "If Statement -- details not implemented.\n");
      break;
    case AST_WHILE_STATEMENT:
      fprintf(output, "While Statement -- details not implemented.\n");
      break;
    case AST_BLOCK:
      fprintf(output, "Block with %d statement(s):\n", node->as.block.count);
      for (int i = 0; i < node->as.block.count; i++) {
        printAST(output, node->as.block.statements[i], indent + 1);
      }
      break;
    default:
      fprintf(output, "Unknown AST Node\n");
      break;
  }
}

// This function prints the entire file's AST either to the console or to a file
// based on the 'outputToFile' flag. If 'outputToFile' is nonzero, the output
// will be saved to a file called "ast", otherwise the output will be printed to
// stdout.
void printASTOutput(ASTNode** nodes, int count, int outputToFile) {
  FILE* output;
  if (outputToFile == 1) {
    output = fopen("ast", "w");
    if (output == NULL) {
      perror("Error opening file 'ast'");
      exit(EXIT_FAILURE);
    }
  } else {
    output = stdout;
  }

  fprintf(output, "Printing AST for the entire file:\n");
  for (int i = 0; i < count; i++) {
    if (nodes[i] != NULL) {
      fprintf(output, "\n--- AST Node %d ---\n", i);
      printAST(output, nodes[i], 0);
    }
  }

  if (outputToFile) {
    fclose(output);
  }
}
