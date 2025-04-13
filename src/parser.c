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

ASTNode* newVariableNode(Token* name, Token* type) {
  printf("Debug: Creating new Variable node. Name: %.*s, Type: %.*s\n",
         name->length, name->lexeme, type->length, type->lexeme);
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory in newVariableNode\n");
    exit(1);
  }
  node->type = AST_VARIABLE;
  // Store pointers to the tokens (which hold the lexeme, length, etc.)
  node->as.variable.name = name;
  node->as.variable.type = type;
  return node;
}

ASTNode* newBinaryNode(ASTNode* left, char operator, ASTNode * right,
                       int line) {
  printf("Debug: Creating new Binary node with operator '%c' at line %d\n",
         operator, line);
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
                         int count, ASTNode** statements, int statementCount) {
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
  return node;
}

//////////////////

int isTokenDataType(Token* token) {
  // Debug: Print token being checked for data type.
  printf("Debug: Checking if token is data type: ");
  printToken(token);
  if (token->type == TOKEN_INT_TYPE || token->type == TOKEN_VOID_TYPE) {
    return 1;
  }
  return 0;
}

Token* peekToken(Token* tokens, int index) { return &tokens[index]; }

Token* peekAheadToken(Token* tokens, int index, int forward, int tokenCount) {
  if (index + forward >= tokenCount) {
    return NULL;
  }
  return &tokens[index + forward];
}

ASTNode* parseVariable(Token* tokens, int* tokenIndex, int tokenCount) {
  printf("Debug: Entering parseVariable at tokenIndex = %d\n", *tokenIndex);
  // Expect a data type first.
  if (!isTokenDataType(peekToken(tokens, *tokenIndex))) {
    fprintf(stderr, "Error: Expected a data type\n");
    return NULL;
  }

  // Debug: Print data type token.
  printf("Debug: Data type token: ");
  printToken(peekToken(tokens, *tokenIndex));

  Token* type = peekToken(tokens, *tokenIndex);
  *tokenIndex += 1;

  // Check for the identifier.
  if (peekToken(tokens, *tokenIndex)->type != TOKEN_IDENTIFIER) {
    fprintf(stderr, "Error: Expected an identifier\n");
    return NULL;
  }

  // Debug: Print identifier token.
  printf("Debug: Identifier token: ");
  printToken(peekToken(tokens, *tokenIndex));

  Token* name = peekToken(tokens, *tokenIndex);
  *tokenIndex += 1;
  return newVariableNode(name, type);
}

ASTNode* parseStatement(Token* tokens, int* tokenIndex, int tokenCount) {
  printf("Debug: Entering parseStatement at tokenIndex = %d\n", *tokenIndex);
  // Placeholder for statement parsing.
  *tokenIndex += 1;
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
  printToken(peekToken(tokens, *tokenIndex));
  returnType = peekToken(tokens, *tokenIndex);
  *tokenIndex += 1;

  // Parse function name.
  printf("Debug: Parsing function name token at index %d: ", *tokenIndex);
  printToken(peekToken(tokens, *tokenIndex));
  name = peekToken(tokens, *tokenIndex);
  *tokenIndex += 1;

  // Ensure we have a '('.
  if (peekToken(tokens, *tokenIndex)->type != TOKEN_LPAREN) {
    fprintf(stderr, "Error: Expected '(' after function name\n");
    return NULL;
  }
  printf("Debug: Found '(' token: ");
  printToken(peekToken(tokens, *tokenIndex));
  *tokenIndex += 1;

  // Parse parameters.
  while (peekToken(tokens, *tokenIndex)->type != TOKEN_RPAREN) {
    printf("Debug: Parsing parameter %d at tokenIndex = %d: ",
           parameterCount + 1, *tokenIndex);
    printToken(peekToken(tokens, *tokenIndex));
    parameters[parameterCount++] =
        parseVariable(tokens, tokenIndex, tokenCount);
    if (peekToken(tokens, *tokenIndex)->type == TOKEN_COMMA) {
      printf("Debug: Found comma token between parameters: ");
      printToken(peekToken(tokens, *tokenIndex));
      *tokenIndex += 1;
    }
  }
  // Skip the closing ')'
  printf("Debug: Found ')' token for parameter list: ");
  printToken(peekToken(tokens, *tokenIndex));
  *tokenIndex += 1;

  // Check for '{' to begin function body.
  if (peekToken(tokens, *tokenIndex)->type != TOKEN_LBRACE) {
    fprintf(stderr, "Error: Expected '{' after function parameters\n");
    return NULL;
  }
  printf("Debug: Found '{' token for function body: ");
  printToken(peekToken(tokens, *tokenIndex));
  *tokenIndex += 1;

  // Parse statements until we see a '}'
  while (peekToken(tokens, *tokenIndex)->type != TOKEN_RBRACE) {
    printf("Debug: Parsing statement %d at tokenIndex = %d: ",
           statementCount + 1, *tokenIndex);
    printToken(peekToken(tokens, *tokenIndex));
    statements[statementCount++] =
        parseStatement(tokens, tokenIndex, tokenCount);
  }
  printf("Debug: Found '}' token ending function body: ");
  printToken(peekToken(tokens, *tokenIndex));
  *tokenIndex += 1;  // Skip the closing brace

  printf("Debug: Finished parsing function '%.*s'\n", name->length,
         name->lexeme);
  return newFunctionNode(name, returnType, parameters, parameterCount,
                         statements, statementCount);
}

ASTNode** parseFile(Token* tokens, int tokenCount) {
  printf("Debug: Entering parseFile. Total tokens: %d\n", tokenCount);
  int tokenIndex = 0;
  ASTNode** astNodes = malloc(100 * sizeof(ASTNode*));
  for (int i = 0; i < 100; i++) {
    astNodes[i] = NULL;
  }

  int astNodesIndex = 0;

  // The following loop is provided as context; note that without proper
  // advancing of tokenIndex, this could become an infinite loop. Here we
  // increment tokenIndex when nothing matches.
  while (tokenIndex < tokenCount) {
    if (peekToken(tokens, tokenIndex)->type == TOKEN_EOF) {
      break;  // End of file reached
    }
    if (isTokenDataType(peekToken(tokens, tokenIndex)) == 1) {
      printf("Is data type\n");
      if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type ==
          TOKEN_IDENTIFIER) {
        printf("Is identifier\n");
        if (peekAheadToken(tokens, tokenIndex, 2, tokenCount)->type ==
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

/////////////////////////////////////////////////////////////
#include <stdio.h>

// Helper function to print indentation.
static void printIndent(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("  ");  // Two spaces per indent level.
  }
}

// Recursively prints the AST tree.
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

    case AST_VARIABLE:
      // Print variable's name and type using token lexeme and length.
      printf("Variable: %.*s of type %.*s\n", node->as.variable.name->length,
             node->as.variable.name->lexeme, node->as.variable.type->length,
             node->as.variable.type->lexeme);
      break;

    case AST_BINARY:
      printf("Binary Expression: '%c'\n", node->as.binary._operator);
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
      // Placeholder for assignment details.
      printf("Assignment -- details not implemented.\n");
      break;

    case AST_EXPRESSION_STATEMENT:
      // Placeholder for an expression statement.
      printf("Expression Statement -- details not implemented.\n");
      break;

    case AST_DECLARATION:
      // Placeholder for declarations.
      printf("Declaration -- details not implemented.\n");
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
      break;

    case AST_IF_STATEMENT:
      // Placeholder for if-statement structure.
      printf("If Statement -- details not implemented.\n");
      break;

    case AST_WHILE_STATEMENT:
      // Placeholder for while-statement structure.
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
