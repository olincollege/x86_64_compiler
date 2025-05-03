#include "parser.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

// ASTNode constructors with added debug prints.
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...)                                           \
  (void)fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __func__, __LINE__, \
                ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

ASTNode* newIntLiteralNode(int value, Token* token) {
  DEBUG_PRINT("Debug: Creating new IntLiteral node with value = %d\n", value);
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newIntLiteralNode\n");
    exit(1);
  }
  node->type = AST_INT_LITERAL;
  node->as.intLiteral.intLiteral = value;
  node->as.intLiteral.token = token;
  return node;
}

ASTNode* newVariableNode(Token* name) {
  DEBUG_PRINT("Debug: Creating new Variable node. Name: %.*s\n", name->length,
              name->lexeme);

  ASTNode* node = malloc(sizeof(ASTNode));

  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newVariableNode\n");
    exit(1);
  }
  node->type = AST_VARIABLE;  // Using the variable type for declarations.
  node->as.variableName = name;
  return node;
}

ASTNode* newVariableDeclarationNode(Token* name, Token* type) {
  DEBUG_PRINT(
      "Debug: Creating new VariableDeclaration node. Name: %.*s, Type: %.*s\n",
      name->length, name->lexeme, type->length, type->lexeme);

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr,
                  "Error: Out of memory in newVariableDeclarationNode\n");
    exit(1);
  }
  node->type =
      AST_VARIABLE_DECLARATION;  // Using the variable type for declarations.
  node->as.variable_declaration.name = name;
  node->as.variable_declaration.type = type;
  return node;
}

ASTNode* newBinaryNode(ASTNode* left, TokenType operator, ASTNode* right) {
  DEBUG_PRINT("Debug: Creating new Binary node with operator '%s'\n",
              tokenTypeToString(operator));

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newBinaryNode\n");
    exit(1);
  }
  node->type = AST_BINARY;
  node->as.binary.left = left;
  node->as.binary._operator = operator;
  node->as.binary.right = right;
  return node;
}

ASTNode* newUnaryNode(char operator, ASTNode* operand) {
  DEBUG_PRINT("Debug: Creating new Unary node with operator '%c'\n", operator);

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newUnaryNode\n");
    exit(1);
  }
  node->type = AST_UNARY;
  node->as.unary._operator = operator;
  node->as.unary.operand = operand;
  return node;
}

ASTNode* newBlockNode(ASTNode** statements, int count) {
  DEBUG_PRINT("Debug: Creating new Block node with %d statement(s)\n", count);

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newBlockNode\n");
    exit(1);
  }
  node->type = AST_BLOCK;
  node->as.block.statements = statements;
  node->as.block.count = count;
  return node;
}

ASTNode* newFunctionNode(Token* name, Token* returnType, ASTNode** parameters,
                         int count, ASTNode* statements) {
  DEBUG_PRINT(
      "Debug: Creating new Function node. Name: %.*s, Return Type: %.*s\n",
      name->length, name->lexeme, returnType->length, returnType->lexeme);

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newFunctionNode\n");
    exit(1);
  }
  node->type = AST_FUNCTION_DECLARATION;
  node->as.function.name = name;
  node->as.function.returnType = returnType;
  node->as.function.parameters = parameters;
  node->as.function.paramCount = count;
  node->as.function.statements = statements;
  return node;
}

ASTNode* newFunctionCallNode(Token* name, ASTNode** parameters,
                             int paramCount) {
  DEBUG_PRINT("Debug: Creating new function call");
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newFunctionCallNode\n");
    exit(1);
  }
  node->type = AST_FUNCTION_CALL;
  node->as.function_call.name = name;
  node->as.function_call.parameters = parameters;
  node->as.function_call.paramCount = paramCount;
  return node;
}

ASTNode* newReturnNode(ASTNode* expression) {
  DEBUG_PRINT("Debug: Creating new Return node.\n");

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newReturnNode\n");
    exit(1);
  }
  node->type = AST_RETURN;
  node->as._return.expression = expression;
  return node;
}

ASTNode* newDeclarationNode(ASTNode* variableDeclaration, ASTNode* expression) {
  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newDeclarationNode\n");
    exit(1);
  }
  node->type = AST_DECLARATION;
  node->as.declaration.variable = variableDeclaration;
  node->as.declaration.expression = expression;
  return node;
}

ASTNode* newIfElifElseNode(ASTNodeType type, ASTNode* condition,
                           ASTNode* body) {
  DEBUG_PRINT("Debug: Creating new If/Elif/Else node.\n");

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newIfElifElseNode\n");
    exit(1);
  }
  node->type = type;
  node->as.if_elif_else_statement.condition = condition;
  node->as.if_elif_else_statement.body = body;
  return node;
}

ASTNode* newWhileNode(ASTNode* condition, ASTNode* body) {
  DEBUG_PRINT("Debug: Creating new while node\n");

  ASTNode* node = malloc(sizeof(ASTNode));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in newWhileNode\n");
    exit(1);
  }
  node->type = AST_WHILE_STATEMENT;
  node->as.while_statement.condition = condition;
  node->as.while_statement.body = body;
  return node;
}

//////////////////
// Helper functions for the parser

int isTokenDataType(Token* token) {
  DEBUG_PRINT("Debug: Checking if token is data type: ");
  printToken(token);

  if (token->type == TOKEN_INT_TYPE || token->type == TOKEN_VOID_TYPE) {
    return 1;
  }
  return 0;
}

Token* peekToken(Token* tokens, int* index) {
  // DEBUG_PRINT("Debug: peekToken at index %d\n", *index);

  return &tokens[(*index)];
}

Token* peekAheadToken(Token* tokens, int* index, int forward, int tokenCount) {
  DEBUG_PRINT("Debug: peekAheadToken at index %d and forward = %d\n", *index,
              forward);
  printToken(&tokens[(*index) + forward]);

  if ((*index) + forward >= tokenCount) {
    return NULL;
  }
  return &tokens[(*index) + forward];
}
//////////////////
// Parser functions

ASTNode* parseVariableDeclaration(Token* tokens, int* tokenIndex,
                                  int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseVariableDeclaration at tokenIndex = %d\n",
              *tokenIndex);

  // Expect a data type token first.
  if (!isTokenDataType(peekToken(tokens, tokenIndex))) {
    (void)fprintf(stderr, "Error: Expected a data type\n");
    return NULL;
  }

  DEBUG_PRINT("Debug: Data type token: ");
  printToken(peekToken(tokens, tokenIndex));

  Token* type = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Check for identifier token.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_IDENTIFIER) {
    (void)fprintf(stderr, "Error: Expected an identifier\n");
    return NULL;
  }

  DEBUG_PRINT("Debug: Identifier token: ");
  printToken(peekToken(tokens, tokenIndex));

  Token* name = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  return newVariableDeclarationNode(name, type);
}

int convertTokenToInt(Token* token) {
  DEBUG_PRINT("convertTokenToInt");

  // Allocate memory for a null-terminated string copy of the substring.
  char* buf = malloc(token->length + 1);
  if (!buf) {
    (void)fprintf(stderr, "Error: Out of memory\n");
    return 0;  // Error signal
  }

  // Copy the substring and add a null terminator.
  memcpy(buf, token->lexeme, token->length);
  buf[token->length] = '\0';

  // Convert to integer
  char* endptr;
  long value = strtol(buf, &endptr, 10);
  // Clean up the temporary buffer.
  free(buf);

  // Check if any characters were converted
  if (endptr == buf) {
    (void)fprintf(stderr, "Error: No digits found in substring\n");
    return 0;
  }

  // // Check if there are any non-digit trailing characters
  // while (*endptr != '\0') {
  //   if (!isspace((unsigned char)*endptr)) {
  //     (void)fprintf(stderr, "Error: Invalid characters after number%s\n",
  //     (*endptr)); return 0;
  //   }
  //   endptr++;
  // }

  // Manual range check (since we're avoiding errno)
  if (value < INT_MIN || value > INT_MAX) {
    (void)fprintf(stderr, "Error: Number out of range for int\n");
    return 0;
  }

  return (int)value;
}

ASTNode* parseVariableOrLiteral(Token* tokens, int* tokenIndex,
                                int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseVariableOrLiteral at tokenIndex = %d\n",
              *tokenIndex);

  if (peekToken(tokens, tokenIndex)->type == TOKEN_IDENTIFIER) {
    ASTNode* node = newVariableNode(peekToken(tokens, tokenIndex));
    (*tokenIndex)++;
    return node;
  } else if (peekToken(tokens, tokenIndex)->type == TOKEN_INT_LITERAL) {
    ASTNode* node =
        newIntLiteralNode(convertTokenToInt(peekToken(tokens, tokenIndex)),
                          &(tokens[*tokenIndex]));
    (*tokenIndex)++;
    return node;
  }
}

static int isVariableOrLiteral(const Token* tok) {
  return tok &&
         (tok->type == TOKEN_IDENTIFIER || tok->type == TOKEN_INT_LITERAL);
}

ASTNode* parseExpression(Token* tokens, int* tokenIndex, int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseExpression at tokenIndex = %d\n",
              *tokenIndex);

  ASTNode* node = NULL;

  if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type == TOKEN_LPAREN) {
    DEBUG_PRINT("Debug: Next Left Parenthis in parseexpression");
    node = parseFunctionCall(tokens, tokenIndex, tokenCount);
  } else {
    DEBUG_PRINT("Debug: Next Left Parenthis not in parseexpression");
    node = parseVariableOrLiteral(tokens, tokenIndex, tokenCount);
  }

  // For now, this is a placeholder.
  // A complete implementation would parse an expression, possibly using
  // recursive descent.
  if (peekAheadToken(tokens, tokenIndex, 0, tokenCount)->type == TOKEN_RPAREN ||
      peekAheadToken(tokens, tokenIndex, 0, tokenCount)->type ==
          TOKEN_SEMICOLON) {
    DEBUG_PRINT("Debug: No expression, just value\n");
    return node;
  }

  if (peekToken(tokens, tokenIndex)->type == TOKEN_LPAREN) {
    // Deal with this later
  }
  DEBUG_PRINT("Debug: Variable or literal with second part\n");
  ASTNode* leftSide = node;
  // parseVariableOrLiteral(tokens, tokenIndex, tokenCount);
  TokenType _operator = peekToken(tokens, tokenIndex)->type;
  (*tokenIndex)++;

  return newBinaryNode(leftSide, _operator,
                       parseExpression(tokens, tokenIndex, tokenCount));
}

// parse for
// parse while

ASTNode* parseWhileStatement(Token* tokens, int* tokenIndex, int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseWhileStatement at tokenIndex = %d\n",
              *tokenIndex);

  ASTNode* condition;
  ASTNode* body;

  if (peekToken(tokens, tokenIndex)->type != TOKEN_WHILE) {
    return NULL;
  }
  (*tokenIndex)++;

  if (peekToken(tokens, tokenIndex)->type != TOKEN_LPAREN) {
    (void)fprintf(stderr, "Error: Expected '(' at tokenIndex = %d\n",
                  *tokenIndex);
    return NULL;
  }

  (*tokenIndex)++;  // Skip left parenthis

  condition = parseExpression(tokens, tokenIndex, tokenCount);

  if (peekToken(tokens, tokenIndex)->type != TOKEN_RPAREN) {
    (void)fprintf(stderr, "Error: Expected '(' at tokenIndex = %d\n",
                  *tokenIndex);
    return NULL;
  }

  (*tokenIndex)++;  // Skip right parenthis

  body = parseBlock(tokens, tokenIndex, tokenCount);

  return newWhileNode(condition, body);
}

ASTNode* parseIfElifElseStatement(Token* tokens, int* tokenIndex,
                                  int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseIfStatement at tokenIndex = %d\n",
              *tokenIndex);

  ASTNode* condition = NULL;
  ASTNode* body = NULL;

  ASTNodeType nodeType;

  if (peekToken(tokens, tokenIndex)->type == TOKEN_IF) {
    nodeType = AST_IF_STATEMENT;
    (*tokenIndex)++;
  } else if (peekToken(tokens, tokenIndex)->type == TOKEN_ELSE) {
    DEBUG_PRINT("Else\n\n");
    if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type == TOKEN_IF) {
      nodeType = AST_ELSE_IF_STATEMENT;
      DEBUG_PRINT("Elseif\n\n");

      (*tokenIndex)++;
      (*tokenIndex)++;
    } else if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type ==
               TOKEN_LBRACE) {
      DEBUG_PRINT("Elseelse\n\n");

      nodeType = AST_ELSE_STATEMENT;
      (*tokenIndex)++;
    } else {
      DEBUG_PRINT("asdfasdfsada\n");
      (void)fprintf(stderr,
                    "Error1: Expected 'if', 'else' or 'else if' at "
                    "tokenIndex = %d\n",
                    *tokenIndex);
      return NULL;
    }
  } else {
    (void)fprintf(
        stderr,
        "Error2: Expected 'if', 'else if' or 'else' at tokenIndex = %d\n",
        *tokenIndex);
    return NULL;
  }

  // Condition only for if or else if, not else
  if (nodeType != AST_ELSE_STATEMENT) {
    if (peekToken(tokens, tokenIndex)->type != TOKEN_LPAREN) {
      (void)fprintf(stderr, "Error: Expected '(' at tokenIndex = %d\n",
                    *tokenIndex);
      return NULL;
    }
    (*tokenIndex)++;  // Skip left parenthis

    condition = parseExpression(tokens, tokenIndex, tokenCount);

    if (peekToken(tokens, tokenIndex)->type != TOKEN_RPAREN) {
      (void)fprintf(stderr, "Error: Expected '(' at tokenIndex = %d\n",
                    *tokenIndex);
      return NULL;
    }

    (*tokenIndex)++;  // Skip right parenthis
  }

  body = parseBlock(tokens, tokenIndex, tokenCount);

  return newIfElifElseNode(nodeType, condition, body);
}

ASTNode* parseFunctionCall(Token* tokens, int* tokenIndex, int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseFunctionCall at tokenIndex = %d\n",
              *tokenIndex);
  Token* name = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;
  ASTNode** parameters = (ASTNode**)malloc(
      sizeof(ASTNode*) * (10));  // TODO should not be a static number
  int parameterCount = 0;
  ASTNode* body;
  if (peekToken(tokens, tokenIndex)->type != TOKEN_LPAREN) {
    (void)fprintf(stderr, "Error: Expected '(' at tokenIndex = %d\n",
                  *tokenIndex);
    return NULL;
  }
  (*tokenIndex)++;  // Skip left parenthis
  while (peekToken(tokens, tokenIndex)->type != TOKEN_RPAREN) {
    parameters[parameterCount++] =
        parseVariableOrLiteral(tokens, tokenIndex, tokenCount);
    if (peekToken(tokens, tokenIndex)->type == TOKEN_RPAREN) {
      break;
    } else if (peekToken(tokens, tokenIndex)->type == TOKEN_COMMA) {
      (*tokenIndex)++;
    } else {
      (void)fprintf(stderr, "Error something else expected\n");
      return NULL;
    }
  }
  (*tokenIndex)++;  // Skip right parenthis
  return newFunctionCallNode(name, parameters, parameterCount);
}

ASTNode* parseStatement(Token* tokens, int* tokenIndex, int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseStatement at tokenIndex = %d\n",
              *tokenIndex);

  // If the token represents the start of a variable declaration:
  if (isTokenDataType(peekToken(tokens, tokenIndex))) {
    ASTNode* variableDeclarationNode =
        parseVariableDeclaration(tokens, tokenIndex, tokenCount);
    if (variableDeclarationNode == NULL) {
      (void)fprintf(stderr, "Error: Failed to parse variable declaration\n");
      return NULL;
    }
    // Check if there is an assignment following the declaration.
    if (peekToken(tokens, tokenIndex) != NULL &&
        peekToken(tokens, tokenIndex)->type != TOKEN_ASSIGN) {
      (*tokenIndex)++;

      DEBUG_PRINT(
          "Debug: No assignment operator found after variable declaration");

      return variableDeclarationNode;
    }

    // Otherwise, if an assignment operator is present, parse the expression.
    (*tokenIndex)++;
    ASTNode* expression = parseExpression(tokens, tokenIndex, tokenCount);
    if (expression == NULL) {
      (void)fprintf(stderr, "Error: Failed to parse expression\n");
      return NULL;
    }
    (*tokenIndex)++;  // Skip semicolon

    return newDeclarationNode(variableDeclarationNode, expression);
  }

  // Case below
  switch (peekToken(tokens, tokenIndex)->type) {
    case TOKEN_RETURN:

      DEBUG_PRINT("Debug: Found 'return' keyword\n");

      (*tokenIndex)++;  // Skip "Return"
      ASTNode* returnExpression =
          parseExpression(tokens, tokenIndex, tokenCount);
      if (peekToken(tokens, tokenIndex)->type != TOKEN_SEMICOLON) {
        (void)fprintf(stderr, "Error: Expected semicolon after return\n");
        return NULL;
      }
      (*tokenIndex)++;  // skip semicolon

      return newReturnNode(returnExpression);
    case TOKEN_SEMICOLON:

      DEBUG_PRINT("Debug: Found semicolon after statement\n");

      (*tokenIndex)++;
      return NULL;
    case TOKEN_IF:
      return parseIfElifElseStatement(tokens, tokenIndex, tokenCount);
    case TOKEN_ELSE:
      return parseIfElifElseStatement(tokens, tokenIndex, tokenCount);
    case TOKEN_WHILE:
      return parseWhileStatement(tokens, tokenIndex, tokenCount);
    case TOKEN_IDENTIFIER:
      // TODO: Maybe switch case for the next part
      if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type ==
          TOKEN_ASSIGN) {
        ASTNode* varaibleName = newVariableNode(peekToken(tokens, tokenIndex));
        (*tokenIndex)++;
        (*tokenIndex)++;  // consume the assign operator
        ASTNode* expressionNode =
            parseExpression(tokens, tokenIndex, tokenCount);
        return newDeclarationNode(varaibleName, expressionNode);
        // newDeclarationNode()
      } else if (peekAheadToken(tokens, tokenIndex, 1, tokenCount)->type ==
                 TOKEN_LPAREN) {
        return parseFunctionCall(tokens, tokenIndex, tokenCount);
      } else {
        return NULL;
      }
    default:
      // Placeholder for other types of statements.
      (*tokenIndex)++;  // consume the token to avoid infinite loop

      return NULL;
  }
}

ASTNode* parseBlock(Token* tokens, int* tokenIndex, int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseBlock at tokenIndex = %d\n", *tokenIndex);

  ASTNode** statements = (ASTNode**)malloc(100 * sizeof(ASTNode*));
  int statementCount = 0;

  if (peekToken(tokens, tokenIndex)->type != TOKEN_LBRACE) {
    // There isn't a left brace so only parse next statement
    ASTNode* newNode = parseStatement(tokens, tokenIndex, tokenCount);
    if (newNode != NULL) {
      statements[statementCount++] = newNode;
    }
    return newBlockNode(statements, statementCount);
  }

  // There is a left brace
  (*tokenIndex)++;  // Move to the next token after '{'

  ASTNode* returnExpression = NULL;

  // Parse function body statements until a '}' is encountered.
  while (peekToken(tokens, tokenIndex)->type != TOKEN_RBRACE) {
    DEBUG_PRINT("Debug: Parsing statement %d at tokenIndex = %d: ",
                statementCount + 1, *tokenIndex);
    printToken(peekToken(tokens, tokenIndex));

    ASTNode* newNode = parseStatement(tokens, tokenIndex, tokenCount);
    if (newNode != NULL) {
      statements[statementCount++] = newNode;
    }
  }

  (*tokenIndex)++;  // Move to the next token after '}'
  return newBlockNode(statements, statementCount);
}

ASTNode* parseFunction(Token* tokens, int* tokenIndex, int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseFunction at tokenIndex = %d\n",
              *tokenIndex);

  Token* name;
  Token* returnType;
  ASTNode** parameters = (ASTNode**)malloc(100 * sizeof(ASTNode*));
  ASTNode* statements;
  ASTNode* returnStatement = NULL;

  int parameterCount = 0;
  int statementCount = 0;

  // Parse return type.
  DEBUG_PRINT("Debug: Parsing function return type token at index %d: ",
              *tokenIndex);
  printToken(peekToken(tokens, tokenIndex));

  returnType = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Parse function name.
  DEBUG_PRINT("Debug: Parsing function name token at index %d: ", *tokenIndex);
  printToken(peekToken(tokens, tokenIndex));

  name = peekToken(tokens, tokenIndex);
  (*tokenIndex)++;

  // Ensure a '(' token follows.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_LPAREN) {
    (void)fprintf(stderr, "Error: Expected '(' after function name\n");
    return NULL;
  }

  DEBUG_PRINT("Debug: Found '(' token: ");
  printToken(peekToken(tokens, tokenIndex));

  (*tokenIndex)++;

  // Parse parameters until a ')' token is found.
  while (peekToken(tokens, tokenIndex)->type != TOKEN_RPAREN) {
    DEBUG_PRINT("Debug: Parsing parameter %d at tokenIndex = %d: ",
                parameterCount + 1, *tokenIndex);
    printToken(peekToken(tokens, tokenIndex));

    parameters[parameterCount++] =
        parseVariableDeclaration(tokens, tokenIndex, tokenCount);
    if (peekToken(tokens, tokenIndex)->type == TOKEN_COMMA) {
      DEBUG_PRINT("Debug: Found comma token between parameters: ");
      printToken(peekToken(tokens, tokenIndex));

      (*tokenIndex)++;
    }
  }

  // Skip the closing ')'
  DEBUG_PRINT("Debug: Found ')' token for parameter list: ");
  printToken(peekToken(tokens, tokenIndex));

  (*tokenIndex)++;

  // Check for '{' to begin the function body.
  if (peekToken(tokens, tokenIndex)->type != TOKEN_LBRACE) {
    (void)fprintf(stderr, "Error: Expected '{' after function parameters\n");
    return NULL;
  }

  DEBUG_PRINT("Debug: Found '{' token for function body: ");
  printToken(peekToken(tokens, tokenIndex));

  statements = parseBlock(tokens, tokenIndex, tokenCount);

  DEBUG_PRINT("Debug: Found '}' token ending function body: ");
  printToken(peekToken(tokens, tokenIndex));

  //   (*tokenIndex)++;  // Skip the closing brace

  DEBUG_PRINT("Debug: Finished parsing function '%.*s'\n", name->length,
              name->lexeme);

  return newFunctionNode(name, returnType, parameters, parameterCount,
                         statements);
}

ASTNode** parseFile(Token* tokens, int tokenCount) {
  DEBUG_PRINT("Debug: Entering parseFile. Total tokens: %d\n", tokenCount);

  int tokenIndex = 0;
  ASTNode** astNodes = (ASTNode**)malloc(100 * sizeof(ASTNode*));
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
      DEBUG_PRINT("Is data type\n");

      if (peekAheadToken(tokens, &tokenIndex, 1, tokenCount)->type ==
          TOKEN_IDENTIFIER) {
        DEBUG_PRINT("Is identifier\n");

        if (peekAheadToken(tokens, &tokenIndex, 2, tokenCount)->type ==
            TOKEN_LPAREN) {
          DEBUG_PRINT("Debug: Parsing function starting at tokenIndex %d\n",
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

/*
Prints indentation spaces to an output stream.

Used to align AST pretty-printing.

Args:
  output: Output stream.
  indent: Number of indentation levels.

Returns:
  void
*/
static void printIndent(FILE* output, int indent) {
  // DEBUG_PRINT("printIndent with indent = %d", indent);
  for (int i = 0; i < indent; i++) {
    (void)fprintf(output, "  ");
  }
}

/* (Other helper / parser functions remain unchanged, but each already contains
 * DEBUG_PRINT calls or had them added earlier.) */

/* -------------------------------------------------------------------------- */
/*                                AST Printer                                 */
/* -------------------------------------------------------------------------- */

void printAST(FILE* output, ASTNode* node, int indent) {
  // DEBUG_PRINT("Entering printAST (indent=%d)", indent);

  if (node == NULL) {
    printIndent(output, indent);
    (void)fprintf(output, "NULL\n");
    return;
  }

  printIndent(output, indent);
  switch (node->type) {
    case AST_INT_LITERAL:
      (void)fprintf(output, "IntLiteral: %d\n", node->as.intLiteral.intLiteral);
      break;

    case AST_VARIABLE_DECLARATION:
      if (node->as.variable_declaration.type != NULL) {
        (void)fprintf(output, "Variable Declaration: %.*s of type %.*s\n",
                      node->as.variable_declaration.name->length,
                      node->as.variable_declaration.name->lexeme,
                      node->as.variable_declaration.type->length,
                      node->as.variable_declaration.type->lexeme);
      } else {
        (void)fprintf(output, "Variable: %.*s\n", node->as.variableName->length,
                      node->as.variableName->lexeme);
      }
      break;

    case AST_VARIABLE:
      (void)fprintf(output, "Variable: %.*s\n", node->as.variableName->length,
                    node->as.variableName->lexeme);
      break;

    case AST_BINARY:
      (void)fprintf(output, "Binary Expression: '%s'\n",
                    tokenTypeToString(node->as.binary._operator));
      printIndent(output, indent + 1);
      (void)fprintf(output, "Left:\n");
      printAST(output, node->as.binary.left, indent + 2);
      printIndent(output, indent + 1);
      (void)fprintf(output, "Right:\n");
      printAST(output, node->as.binary.right, indent + 2);
      break;

    case AST_UNARY:
      (void)fprintf(output, "Unary Expression: '%c'\n",
                    node->as.unary._operator);
      printIndent(output, indent + 1);
      (void)fprintf(output, "Operand:\n");
      printAST(output, node->as.unary.operand, indent + 2);
      break;

    case AST_ASSIGNMENT:
      (void)fprintf(output, "Assignment -- details not implemented.\n");
      break;

    case AST_DECLARATION:
      (void)fprintf(output, "Declaration:\n");
      printIndent(output, indent + 1);
      (void)fprintf(output, "Variable Declaration:\n");
      printAST(output, node->as.declaration.variable, indent + 2);
      printIndent(output, indent + 1);
      (void)fprintf(output, "Expression:\n");
      printAST(output, node->as.declaration.expression, indent + 2);
      break;

    case AST_FUNCTION_DECLARATION:
      (void)fprintf(output, "Function Declaration: %.*s returns %.*s\n",
                    node->as.function.name->length,
                    node->as.function.name->lexeme,
                    node->as.function.returnType->length,
                    node->as.function.returnType->lexeme);
      printIndent(output, indent + 1);
      (void)fprintf(output, "Parameters (%d):\n", node->as.function.paramCount);
      for (int i = 0; i < node->as.function.paramCount; i++) {
        printAST(output, node->as.function.parameters[i], indent + 2);
      }
      printIndent(output, indent + 1);
      (void)fprintf(output, "Body Statements:\n");
      printAST(output, node->as.function.statements, indent + 2);
      break;

    case AST_FUNCTION_CALL:
      (void)fprintf(output, "Function Call: %.*s with %d argument(s)\n",
                    node->as.function_call.name->length,
                    node->as.function_call.name->lexeme,
                    node->as.function_call.paramCount);
      for (int i = 0; i < node->as.function_call.paramCount; i++) {
        printAST(output, node->as.function_call.parameters[i], indent + 1);
      }
      break;

    case AST_IF_STATEMENT:
      (void)fprintf(output, "If Statement:\n");
      printIndent(output, indent + 1);
      (void)fprintf(output, "Condition:\n");
      printAST(output, node->as.if_elif_else_statement.condition, indent + 2);
      printIndent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      printAST(output, node->as.if_elif_else_statement.body, indent + 2);
      break;

    case AST_ELSE_IF_STATEMENT:
      (void)fprintf(output, "Else If Statement:\n");
      printIndent(output, indent + 1);
      (void)fprintf(output, "Condition:\n");
      printAST(output, node->as.if_elif_else_statement.condition, indent + 2);
      printIndent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      printAST(output, node->as.if_elif_else_statement.body, indent + 2);
      break;

    case AST_ELSE_STATEMENT:
      (void)fprintf(output, "Else Statement:\n");
      printIndent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      printAST(output, node->as.if_elif_else_statement.body, indent + 2);
      break;

    case AST_WHILE_STATEMENT:
      (void)fprintf(output, "While Statement:\n");
      printIndent(output, indent + 1);
      (void)fprintf(output, "Condition:\n");
      printAST(output, node->as.while_statement.condition, indent + 2);
      printIndent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      printAST(output, node->as.while_statement.body, indent + 2);
      break;

    case AST_BLOCK:
      (void)fprintf(output, "Block with %d statement(s):\n",
                    node->as.block.count);
      for (int i = 0; i < node->as.block.count; i++) {
        printAST(output, node->as.block.statements[i], indent + 1);
      }
      break;

    case AST_RETURN:
      (void)fprintf(output, "Return Statement:\n");
      printIndent(output, indent + 1);
      (void)fprintf(output, "Expression:\n");
      printAST(output, node->as._return.expression, indent + 2);
      break;

    default:
      (void)fprintf(output, "Unknown AST Node\n");
      break;
  }
}

/* -------------------------------------------------------------------------- */

void printASTOutput(ASTNode** nodes, int count, int outputToFile) {
  // DEBUG_PRINT("Entering printASTOutput (count=%d, outputToFile=%d)", count,
  //             outputToFile);

  FILE* output;
  if (outputToFile == 1) {
    output = fopen("ast.txt", "w");
    if (output == NULL) {
      perror("Error opening file 'ast'");
      exit(EXIT_FAILURE);
    }
  } else {
    output = stdout;
  }

  (void)fprintf(output, "Printing AST for the entire file:\n");
  for (int i = 0; i < count; i++) {
    if (nodes[i] != NULL) {
      (void)fprintf(output, "\n--- AST Node %d ---\n", i);
      printAST(output, nodes[i], 0);
    }
  }

  if (outputToFile) {
    fclose(output);
  }

  DEBUG_PRINT("Exiting printASTOutput");
}
