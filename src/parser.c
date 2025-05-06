#include "parser.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

// NOLINTBEGIN(clang-diagnostic-gnu-zero-variadic-macro-arguments)

// #define DEBUG
#ifdef DEBUG
/**
 * debug_print – Helper to emit a debug message with file, function, and line.
 * @func:   __func__ of the caller.
 * @line:   __LINE__ of the caller.
 * @fmt:    printf-style format string.
 * @...:    Arguments for fmt.
 */
static inline void debug_print(const char* func, int line, const char* fmt,
                               ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[DEBUG] %s:%d: ", func, line);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}
#define DEBUG_PRINT(fmt, ...) \
  debug_print(__func__, __LINE__, (fmt), ##__VA_ARGS__)
#else
/* In release builds this becomes a no‑op with zero overhead */
// NOLINTNEXTLINE(clang-diagnostic-unused-function)
static inline void debug_print(const char* func, int line, const char* fmt,
                               ...) {
  (void)func;
  (void)line;
  (void)fmt;
}
#define DEBUG_PRINT(fmt, ...) ((void)0)
#endif

const int MAX_PARAMETER_SIZE = 100;
const int MAX_NUMBER_OF_FUNCTIONS = 100;
const int MAX_VALUE_SIZE = 10;
const int MAX_NUMBER_OF_STATEMENTS = 100;

ast_node* new_int_literal_node(int value, Token* token) {
  DEBUG_PRINT("Debug: Creating new IntLiteral node with value = %d\n", value);
  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    error_and_exit("Error: Out of memory in new_int_literal_node\n");
    return NULL;
  }
  node->type = AST_INT_LITERAL;
  node->as.int_literal.int_literal = value;
  node->as.int_literal.token = token;
  return node;
}

ast_node* new_variable_node(Token* name) {
  DEBUG_PRINT("Debug: Creating new Variable node. Name: %.*s\n", name->length,
              name->lexeme);

  ast_node* node = malloc(sizeof(ast_node));

  if (!node) {
    error_and_exit("Error: Out of memory in new_variable_node\n");
    return NULL;
  }
  node->type = AST_VARIABLE;  // Using the variable type for declarations.
  node->as.variable_name = name;
  return node;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ast_node* new_variable_declaration_node(Token* name, Token* type) {
  DEBUG_PRINT(
      "Debug: Creating new VariableDeclaration node. Name: %.*s, Type: %.*s\n",
      name->length, name->lexeme, type->length, type->lexeme);

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    error_and_exit("Error: Out of memory in new_variable_declaration_node\n");
    return NULL;
  }
  node->type =
      AST_VARIABLE_DECLARATION;  // Using the variable type for declarations.
  node->as.variable_declaration.name = name;
  node->as.variable_declaration.type = type;
  return node;
}

ast_node* new_binary_node(ast_node* left, TokenType operator, ast_node* right) {
  DEBUG_PRINT("Debug: Creating new Binary node with operator '%s'\n",
              token_type_to_string(operator));

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    error_and_exit("Error: Out of memory in new_binary_node\n");
    return NULL;
  }
  node->type = AST_BINARY;
  node->as.binary.left = left;
  node->as.binary._operator = operator;
  node->as.binary.right = right;
  return node;
}

ast_node* new_unary_node(char operator, ast_node* operand) {
  DEBUG_PRINT("Debug: Creating new Unary node with operator '%c'\n", operator);

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in new_unary_node\n");
    return NULL;
  }
  node->type = AST_UNARY;
  node->as.unary._operator = operator;
  node->as.unary.operand = operand;
  return node;
}

ast_node* new_block_node(ast_node** statements, int count) {
  DEBUG_PRINT("Debug: Creating new Block node with %d statement(s)\n", count);

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in new_block_node\n");
    return NULL;
  }
  node->type = AST_BLOCK;
  node->as.block.statements = statements;
  node->as.block.count = count;
  return node;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ast_node* new_function_node(Token* name, Token* return_type,
                            ast_node** parameters, int count,
                            ast_node* statements) {
  DEBUG_PRINT(
      "Debug: Creating new Function node. Name: %.*s, Return Type: %.*s\n",
      name->length, name->lexeme, return_type->length, return_type->lexeme);

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in new_function_node\n");
    return NULL;
  }
  node->type = AST_FUNCTION_DECLARATION;
  node->as.function.name = name;
  node->as.function.return_type = return_type;
  node->as.function.parameters = parameters;
  node->as.function.param_count = count;
  node->as.function.statements = statements;
  return node;
}

ast_node* new_function_call_node(Token* name, ast_node** parameters,
                                 int param_count) {
  DEBUG_PRINT("Debug: Creating new function call");
  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in new_function_call_node\n");
    return NULL;
  }
  node->type = AST_FUNCTION_CALL;
  node->as.function_call.name = name;
  node->as.function_call.parameters = parameters;
  node->as.function_call.param_count = param_count;
  return node;
}

ast_node* new_return_node(ast_node* expression) {
  DEBUG_PRINT("Debug: Creating new Return node.\n");

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in new_return_node\n");
    return NULL;
  }
  node->type = AST_RETURN;
  node->as._return.expression = expression;
  return node;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ast_node* new_declaration_node(ast_node* variable_declaration,
                               ast_node* expression) {
  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    error_and_exit("Error: Out of memory in new_declaration_node\n");
    return NULL;
  }
  node->type = AST_DECLARATION;
  node->as.declaration.variable = variable_declaration;
  node->as.declaration.expression = expression;
  return node;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ast_node* new_if_elif_else_node(ast_node_type type, ast_node* condition,
                                ast_node* body) {
  DEBUG_PRINT("Debug: Creating new If/Elif/Else node.\n");

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in new_if_elif_else_node\n");
    return NULL;
  }
  node->type = type;
  node->as.if_elif_else_statement.condition = condition;
  node->as.if_elif_else_statement.body = body;
  return node;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ast_node* new_while_node(ast_node* condition, ast_node* body) {
  DEBUG_PRINT("Debug: Creating new while node\n");

  ast_node* node = malloc(sizeof(ast_node));
  if (!node) {
    (void)fprintf(stderr, "Error: Out of memory in new_while_node\n");
    return NULL;
  }
  node->type = AST_WHILE_STATEMENT;
  node->as.while_statement.condition = condition;
  node->as.while_statement.body = body;
  return node;
}

//////////////////
// Helper functions for the parser

int is_token_data_type(Token* token) {
  DEBUG_PRINT("Debug: Checking if token is data type: ");
  print_token(token);

  if (token->type == TOKEN_INT_TYPE || token->type == TOKEN_VOID_TYPE) {
    return 1;
  }
  return 0;
}

Token* peek_token(Token* tokens, const int* index) {
  // DEBUG_PRINT("Debug: peek_token at index %d\n", *index);

  return &tokens[(*index)];
}

Token* peek_ahead_token(Token* tokens, const int* index, int forward,
                        int token_count) {
  DEBUG_PRINT("Debug: peek_ahead_token at index %d and forward = %d\n", *index,
              forward);
  print_token(&tokens[(*index) + forward]);

  if ((*index) + forward >= token_count) {
    return NULL;
  }
  return &tokens[(*index) + forward];
}
//////////////////
// Parser functions

ast_node* parse_variable_declaration(Token* tokens, int* token_index,
                                     int token_count) {
  DEBUG_PRINT(
      "Debug: Entering parse_variable_declaration at token_index = %d\n",
      *token_index);

  // Expect a data type token first.
  if (!is_token_data_type(peek_token(tokens, token_index))) {
    error_and_exit("Error: Expected a data type\n");
  }

  DEBUG_PRINT("Debug: Data type token: ");
  print_token(peek_token(tokens, token_index));

  Token* type = peek_ahead_token(tokens, token_index, 0, token_count);
  (*token_index)++;

  // Check for identifier token.
  if (peek_token(tokens, token_index)->type != TOKEN_IDENTIFIER) {
    error_and_exit("Error: Expected an identifier\n");
  }

  DEBUG_PRINT("Debug: Identifier token: ");
  print_token(peek_token(tokens, token_index));

  Token* name = peek_token(tokens, token_index);
  (*token_index)++;

  return new_variable_declaration_node(name, type);
}

int convert_token_to_int(Token* token) {
  DEBUG_PRINT("convert_token_to_int");

  // Allocate memory for a null-terminated string copy of the substring.
  char* buf = malloc((size_t)token->length + (size_t)1);
  if (!buf) {
    error_and_exit("Error: Out of memory\n");
    return -1;
  }

  // Copy the substring and add a null terminator.
  memcpy(buf, token->lexeme, (size_t)token->length);
  buf[token->length] = '\0';

  // Convert to integer
  char* endptr = NULL;
  long value = strtol(buf, &endptr, MAX_VALUE_SIZE);
  // Clean up the temporary buffer.
  free(buf);

  // Check if any characters were converted
  if (endptr == buf) {
    error_and_exit("Error: No digits found in substring\n");
  }

  // // Check if there are any non-digit trailing characters
  // while (*endptr != '\0') {
  //   if (!isspace((unsigned char)*endptr)) {
  //     error_and_exit("Error: Invalid characters after number%s\n",
  //     (*endptr)); return 0;
  //   }
  //   endptr++;
  // }

  // Manual range check (since we're avoiding errno)
  if (value < INT_MIN || value > INT_MAX) {
    error_and_exit("Error: Number out of range for int\n");
  }

  return (int)value;
}

ast_node* parse_variable_or_literal(Token* tokens, int* token_index,
                                    int token_count) {
  DEBUG_PRINT("Debug: Entering parse_variable_or_literal at token_index = %d\n",
              *token_index);

  if (peek_token(tokens, token_index)->type == TOKEN_IDENTIFIER) {
    ast_node* node = new_variable_node(peek_token(tokens, token_index));
    (*token_index)++;
    return node;
  }
  if (peek_ahead_token(tokens, token_index, 0, token_count)->type ==
      TOKEN_INT_LITERAL) {
    ast_node* node = new_int_literal_node(
        convert_token_to_int(peek_token(tokens, token_index)),
        &(tokens[*token_index]));
    (*token_index)++;
    return node;
  }
  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
ast_node* parse_expression(Token* tokens, int* token_index, int token_count) {
  DEBUG_PRINT("Debug: Entering parse_expression at token_index = %d\n",
              *token_index);

  ast_node* node = NULL;

  if (peek_ahead_token(tokens, token_index, 1, token_count)->type ==
      TOKEN_LPAREN) {
    DEBUG_PRINT("Debug: Next Left Parenthis in parseexpression");
    node = parse_function_call(tokens, token_index, token_count);
  } else {
    DEBUG_PRINT("Debug: Next Left Parenthis not in parseexpression");
    node = parse_variable_or_literal(tokens, token_index, token_count);
  }

  // For now, this is a placeholder.
  // A complete implementation would parse an expression, possibly using
  // recursive descent.
  if (peek_ahead_token(tokens, token_index, 0, token_count)->type ==
          TOKEN_RPAREN ||
      peek_ahead_token(tokens, token_index, 0, token_count)->type ==
          TOKEN_SEMICOLON) {
    DEBUG_PRINT("Debug: No expression, just value\n");
    return node;
  }

  if (peek_token(tokens, token_index)->type == TOKEN_LPAREN) {
    // Deal with this later
  }

  DEBUG_PRINT("Debug: Variable or literal with second part\n");
  ast_node* leftSide = node;
  // parse_variable_or_literal(tokens, token_index, token_count);
  TokenType _operator = peek_token(tokens, token_index)->type;
  (*token_index)++;

  return new_binary_node(leftSide, _operator,
                         parse_expression(tokens, token_index, token_count));
}

// NOLINTNEXTLINE(misc-no-recursion)
ast_node* parse_while_statement(Token* tokens, int* token_index,
                                int token_count) {
  DEBUG_PRINT("Debug: Entering parse_while_statement at token_index = %d\n",
              *token_index);

  ast_node* condition = NULL;
  ast_node* body = NULL;

  if (peek_token(tokens, token_index)->type != TOKEN_WHILE) {
    return NULL;
  }
  (*token_index)++;

  if (peek_token(tokens, token_index)->type != TOKEN_LPAREN) {
    (void)fprintf(stderr, "Error: Expected '(' at token_index = %d\n",
                  *token_index);
    error_and_exit("");
  }

  (*token_index)++;  // Skip left parenthis

  condition = parse_expression(tokens, token_index, token_count);

  if (peek_token(tokens, token_index)->type != TOKEN_RPAREN) {
    (void)fprintf(stderr, "Error: Expected '(' at token_index = %d\n",
                  *token_index);
    error_and_exit("");
  }

  (*token_index)++;  // Skip right parenthis

  body = parse_block(tokens, token_index, token_count);

  return new_while_node(condition, body);
}

// NOLINTNEXTLINE(misc-no-recursion)
ast_node* parse_if_elif_else_statement(Token* tokens, int* token_index,
                                       int token_count) {
  DEBUG_PRINT("Debug: Entering parseIfStatement at token_index = %d\n",
              *token_index);

  ast_node* condition = NULL;
  ast_node* body = NULL;

  ast_node_type node_type = AST_INVALID;

  if (peek_token(tokens, token_index)->type == TOKEN_IF) {
    node_type = AST_IF_STATEMENT;
    (*token_index)++;
  } else if (peek_token(tokens, token_index)->type == TOKEN_ELSE) {
    DEBUG_PRINT("Else\n\n");
    if (peek_ahead_token(tokens, token_index, 1, token_count)->type ==
        TOKEN_IF) {
      node_type = AST_ELSE_IF_STATEMENT;

      DEBUG_PRINT("Elseif\n\n");

      (*token_index)++;
      (*token_index)++;
    } else if (peek_ahead_token(tokens, token_index, 1, token_count)->type ==
               TOKEN_LBRACE) {
      DEBUG_PRINT("Elseelse\n\n");

      node_type = AST_ELSE_STATEMENT;
      (*token_index)++;
    } else {
      DEBUG_PRINT("asdfasdfsada\n");
      (void)fprintf(stderr,
                    "Error1: Expected 'if', 'else' or 'else if' at "
                    "token_index = %d\n",
                    *token_index);
      return NULL;
    }
  } else {
    (void)fprintf(
        stderr,
        "Error2: Expected 'if', 'else if' or 'else' at token_index = %d\n",
        *token_index);
    return NULL;
  }

  // Condition only for if or else if, not else
  if (node_type != AST_ELSE_STATEMENT) {
    if (peek_token(tokens, token_index)->type != TOKEN_LPAREN) {
      (void)fprintf(stderr, "Error: Expected '(' at token_index = %d\n",
                    *token_index);
      return NULL;
    }
    (*token_index)++;  // Skip left parenthis

    condition = parse_expression(tokens, token_index, token_count);

    if (peek_token(tokens, token_index)->type != TOKEN_RPAREN) {
      (void)fprintf(stderr, "Error: Expected '(' at token_index = %d\n",
                    *token_index);
      return NULL;
    }

    (*token_index)++;  // Skip right parenthis
  }

  body = parse_block(tokens, token_index, token_count);

  return new_if_elif_else_node(node_type, condition, body);
}

ast_node* parse_function_call(Token* tokens, int* token_index,
                              int token_count) {
  DEBUG_PRINT("Debug: Entering parse_function_call at token_index = %d\n",
              *token_index);
  Token* name = peek_token(tokens, token_index);
  (*token_index)++;
  ast_node** parameters = (ast_node**)malloc(
      sizeof(ast_node*) * ((long unsigned int)MAX_PARAMETER_SIZE));
  int parameter_count = 0;
  if (peek_token(tokens, token_index)->type != TOKEN_LPAREN) {
    (void)fprintf(stderr, "Error: Expected '(' at token_index = %d\n",
                  *token_index);
    error_and_exit("");
  }
  (*token_index)++;  // Skip left parenthis
  while (peek_token(tokens, token_index)->type != TOKEN_RPAREN) {
    parameters[parameter_count++] =
        parse_variable_or_literal(tokens, token_index, token_count);
    if (peek_token(tokens, token_index)->type == TOKEN_RPAREN) {
      break;
    }
    if (peek_token(tokens, token_index)->type == TOKEN_COMMA) {
      (*token_index)++;
    } else {
      error_and_exit("Error something else expected\n");
    }
  }
  (*token_index)++;  // Skip right parenthis
  ast_node* new_node =
      new_function_call_node(name, parameters, parameter_count);
  if (!new_node) {
    free((void*)parameters);
  }
  return new_node;
}

// NOLINTNEXTLINE(misc-no-recursion)
ast_node* parse_statement(Token* tokens, int* token_index, int token_count) {
  DEBUG_PRINT("Debug: Entering parse_statement at token_index = %d\n",
              *token_index);

  // If the token represents the start of a variable declaration:
  if (is_token_data_type(peek_token(tokens, token_index))) {
    ast_node* variable_declaration_node =
        parse_variable_declaration(tokens, token_index, token_count);
    if (variable_declaration_node == NULL) {
      error_and_exit("Error: Failed to parse variable declaration\n");
    }
    // Check if there is an assignment following the declaration.
    if (peek_token(tokens, token_index) != NULL &&
        peek_token(tokens, token_index)->type != TOKEN_ASSIGN) {
      (*token_index)++;

      DEBUG_PRINT(
          "Debug: No assignment operator found after variable declaration");

      return variable_declaration_node;
    }

    // Otherwise, if an assignment operator is present, parse the expression.
    (*token_index)++;
    ast_node* expression = parse_expression(tokens, token_index, token_count);
    if (expression == NULL) {
      error_and_exit("Error: Failed to parse expression\n");
    }
    (*token_index)++;  // Skip semicolon

    ast_node* new_node =
        new_declaration_node(variable_declaration_node, expression);
    if (!new_node) {
      free((void*)expression);
      free((void*)variable_declaration_node);
    }
    return new_node;
  }

  // Case below
  switch (peek_token(tokens, token_index)->type) {
    case TOKEN_RETURN:

      DEBUG_PRINT("Debug: Found 'return' keyword\n");

      (*token_index)++;  // Skip "Return"
      ast_node* return_expression =
          parse_expression(tokens, token_index, token_count);
      if (peek_token(tokens, token_index)->type != TOKEN_SEMICOLON) {
        error_and_exit("Error: Expected semicolon after return\n");
      }
      (*token_index)++;  // skip semicolon

      ast_node* new_node = new_return_node(return_expression);
      if (!new_node) {
        free((void*)return_expression);
      }
      return new_node;

    case TOKEN_SEMICOLON:

      DEBUG_PRINT("Debug: Found semicolon after statement\n");

      (*token_index)++;
      return NULL;
    case TOKEN_IF:
    case TOKEN_ELSE:
      return parse_if_elif_else_statement(tokens, token_index, token_count);
    case TOKEN_WHILE:
      return parse_while_statement(tokens, token_index, token_count);
    case TOKEN_IDENTIFIER:
      // TODO (Nividh): Maybe switch case for the next part
      if (peek_ahead_token(tokens, token_index, 1, token_count)->type ==
          TOKEN_ASSIGN) {
        ast_node* varaible_name =
            new_variable_node(peek_token(tokens, token_index));
        (*token_index)++;
        (*token_index)++;  // consume the assign operator
        ast_node* expression_node =
            parse_expression(tokens, token_index, token_count);

        ast_node* temp_declaration_node =
            new_declaration_node(varaible_name, expression_node);
        if (!temp_declaration_node) {
          free((void*)varaible_name);
          free((void*)expression_node);
        }
        return temp_declaration_node;

        // new_declaration_node()
      } else if (peek_ahead_token(tokens, token_index, 1, token_count)->type ==
                 TOKEN_LPAREN) {
        return parse_function_call(tokens, token_index, token_count);
      }
      break;
    default:
      // Placeholder for other types of statements.
      (*token_index)++;  // consume the token to avoid infinite loop

      return NULL;
  }
  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
ast_node* parse_block(Token* tokens, int* token_index, int token_count) {
  DEBUG_PRINT("Debug: Entering parse_block at token_index = %d\n",
              *token_index);

  ast_node** statements = (ast_node**)malloc(
      (size_t)MAX_NUMBER_OF_STATEMENTS *
      sizeof *statements);  // NOLINT(bugprone-sizeof-expression)
  int statement_count = 0;

  if (peek_token(tokens, token_index)->type != TOKEN_LBRACE) {
    // There isn't a left brace so only parse next statement
    ast_node* new_node = parse_statement(tokens, token_index, token_count);
    if (new_node != NULL) {
      statements[statement_count++] = new_node;
    }
    new_node = new_block_node(statements, statement_count);
    if (!new_node) {
      free((void*)statements);
      error_and_exit(
          "Error parsing function body. No statements found after left brace.");
      return NULL;
    }
    return new_node;
  }

  // There is a left brace
  (*token_index)++;  // Move to the next token after '{'

  // Parse function body statements until a '}' is encountered.
  while (peek_token(tokens, token_index)->type != TOKEN_RBRACE) {
    DEBUG_PRINT("Debug: Parsing statement %d at token_index = %d: ",
                statement_count + 1, *token_index);
    print_token(peek_token(tokens, token_index));

    ast_node* new_node = parse_statement(tokens, token_index, token_count);
    if (new_node != NULL) {
      statements[statement_count++] = new_node;
    }
  }

  (*token_index)++;  // Move to the next token after '}'
  ast_node* new_node = new_block_node(statements, statement_count);
  if (!new_node) {
    free((void*)statements);
    error_and_exit("Failed to parse function body");
  }
  return new_node;
}

ast_node* parse_function(Token* tokens, int* token_index, int token_count) {
  DEBUG_PRINT("Debug: Entering parse_function at token_index = %d\n",
              *token_index);

  Token* name = NULL;
  Token* return_type = NULL;
  ast_node** parameters = (ast_node**)malloc(
      (long unsigned int)MAX_PARAMETER_SIZE * sizeof(ast_node*));
  ast_node* statements = NULL;

  int parameter_count = 0;

  // Parse return type.
  DEBUG_PRINT("Debug: Parsing function return type token at index %d: ",
              *token_index);
  print_token(peek_token(tokens, token_index));

  return_type = peek_token(tokens, token_index);
  (*token_index)++;

  // Parse function name.
  DEBUG_PRINT("Debug: Parsing function name token at index %d: ", *token_index);
  print_token(peek_token(tokens, token_index));

  name = peek_token(tokens, token_index);
  (*token_index)++;

  // Ensure a '(' token follows.
  if (peek_token(tokens, token_index)->type != TOKEN_LPAREN) {
    free((void*)parameters);
    error_and_exit("Error: Expected '(' after function name\n");
    return NULL;
  }

  DEBUG_PRINT("Debug: Found '(' token: ");
  print_token(peek_token(tokens, token_index));

  (*token_index)++;

  // Parse parameters until a ')' token is found.
  while (peek_token(tokens, token_index)->type != TOKEN_RPAREN) {
    DEBUG_PRINT("Debug: Parsing parameter %d at token_index = %d: ",
                parameter_count + 1, *token_index);
    print_token(peek_token(tokens, token_index));

    parameters[parameter_count++] =
        parse_variable_declaration(tokens, token_index, token_count);
    if (peek_token(tokens, token_index)->type == TOKEN_COMMA) {
      DEBUG_PRINT("Debug: Found comma token between parameters: ");
      print_token(peek_token(tokens, token_index));

      (*token_index)++;
    }
  }

  // Skip the closing ')'
  DEBUG_PRINT("Debug: Found ')' token for parameter list: ");
  print_token(peek_token(tokens, token_index));

  (*token_index)++;

  // Check for '{' to begin the function body.
  if (peek_token(tokens, token_index)->type != TOKEN_LBRACE) {
    error_and_exit("Error: Expected '{' after function parameters\n");
  }

  DEBUG_PRINT("Debug: Found '{' token for function body: ");
  print_token(peek_token(tokens, token_index));

  statements = parse_block(tokens, token_index, token_count);

  DEBUG_PRINT("Debug: Found '}' token ending function body: ");
  print_token(peek_token(tokens, token_index));

  //   (*token_index)++;  // Skip the closing brace

  DEBUG_PRINT("Debug: Finished parsing function '%.*s'\n", name->length,
              name->lexeme);

  ast_node* new_node = new_function_node(name, return_type, parameters,
                                         parameter_count, statements);
  if (!new_node) {
    free((void*)statements);
    free((void*)parameters);
    error_and_exit("malloc failed");
  }
  return new_node;
}

ast_node** parse_file(Token* tokens, int token_count) {
  DEBUG_PRINT("Debug: Entering parse_file. Total tokens: %d\n", token_count);

  int token_index = 0;
  ast_node** ast_nodes =
      (ast_node**)malloc((size_t)MAX_NUMBER_OF_FUNCTIONS * sizeof(ast_node*));
  for (int i = 0; i < MAX_NUMBER_OF_FUNCTIONS; i++) {
    ast_nodes[i] = NULL;
  }
  int ast_nodes_index = 0;

  // Loop until end-of-file token is reached.
  while (token_index < token_count) {
    if (peek_token(tokens, &token_index)->type == TOKEN_EOF) {
      break;
    }
    if (is_token_data_type(peek_token(tokens, &token_index)) == 1) {
      DEBUG_PRINT("Is data type\n");

      if (peek_ahead_token(tokens, &token_index, 1, token_count)->type ==
          TOKEN_IDENTIFIER) {
        DEBUG_PRINT("Is identifier\n");

        if (peek_ahead_token(tokens, &token_index, 2, token_count)->type ==
            TOKEN_LPAREN) {
          DEBUG_PRINT("Debug: Parsing function starting at token_index %d\n",
                      token_index);

          ast_nodes[ast_nodes_index++] =
              parse_function(tokens, &token_index, token_count);
          continue;
        }
      }
    }
    token_index++;  // Avoid infinite loop if no matching constructs are found.
  }
  return ast_nodes;
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
static void print_indent(FILE* output, int indent) {
  // DEBUG_PRINT("print_indent with indent = %d", indent);
  for (int i = 0; i < indent; i++) {
    (void)fprintf(output, "  ");
  }
}

/* (Other helper / parser functions remain unchanged, but each already contains
 * DEBUG_PRINT calls or had them added earlier.) */

/* -------------------------------------------------------------------------- */
/*                                AST Printer                                 */
/* -------------------------------------------------------------------------- */

// NOLINTNEXTLINE(misc-no-recursion)
void print_ast(FILE* output, ast_node* node, int indent) {
  // DEBUG_PRINT("Entering print_ast (indent=%d)", indent);

  if (node == NULL) {
    print_indent(output, indent);
    (void)fprintf(output, "NULL\n");
    return;
  }

  print_indent(output, indent);
  switch (node->type) {
    case AST_INT_LITERAL:
      (void)fprintf(output, "IntLiteral: %d\n",
                    node->as.int_literal.int_literal);
      break;

    case AST_VARIABLE_DECLARATION:
      if (node->as.variable_declaration.type != NULL) {
        (void)fprintf(output, "Variable Declaration: %.*s of type %.*s\n",
                      node->as.variable_declaration.name->length,
                      node->as.variable_declaration.name->lexeme,
                      node->as.variable_declaration.type->length,
                      node->as.variable_declaration.type->lexeme);
      } else {
        (void)fprintf(output, "Variable: %.*s\n",
                      node->as.variable_name->length,
                      node->as.variable_name->lexeme);
      }
      break;

    case AST_VARIABLE:
      (void)fprintf(output, "Variable: %.*s\n", node->as.variable_name->length,
                    node->as.variable_name->lexeme);
      break;

    case AST_BINARY:
      (void)fprintf(output, "Binary Expression: '%s'\n",
                    token_type_to_string(node->as.binary._operator));
      print_indent(output, indent + 1);
      (void)fprintf(output, "Left:\n");
      print_ast(output, node->as.binary.left, indent + 2);
      print_indent(output, indent + 1);
      (void)fprintf(output, "Right:\n");
      print_ast(output, node->as.binary.right, indent + 2);
      break;

    case AST_UNARY:
      (void)fprintf(output, "Unary Expression: '%c'\n",
                    node->as.unary._operator);
      print_indent(output, indent + 1);
      (void)fprintf(output, "Operand:\n");
      print_ast(output, node->as.unary.operand, indent + 2);
      break;

    case AST_ASSIGNMENT:
      (void)fprintf(output, "Assignment -- details not implemented.\n");
      break;

    case AST_DECLARATION:
      (void)fprintf(output, "Declaration:\n");
      print_indent(output, indent + 1);
      (void)fprintf(output, "Variable Declaration:\n");
      print_ast(output, node->as.declaration.variable, indent + 2);
      print_indent(output, indent + 1);
      (void)fprintf(output, "Expression:\n");
      print_ast(output, node->as.declaration.expression, indent + 2);
      break;

    case AST_FUNCTION_DECLARATION:
      (void)fprintf(output, "Function Declaration: %.*s returns %.*s\n",
                    node->as.function.name->length,
                    node->as.function.name->lexeme,
                    node->as.function.return_type->length,
                    node->as.function.return_type->lexeme);
      print_indent(output, indent + 1);
      (void)fprintf(output, "Parameters (%d):\n",
                    node->as.function.param_count);
      for (int i = 0; i < node->as.function.param_count; i++) {
        print_ast(output, node->as.function.parameters[i], indent + 2);
      }
      print_indent(output, indent + 1);
      (void)fprintf(output, "Body Statements:\n");
      print_ast(output, node->as.function.statements, indent + 2);
      break;

    case AST_FUNCTION_CALL:
      (void)fprintf(output, "Function Call: %.*s with %d argument(s)\n",
                    node->as.function_call.name->length,
                    node->as.function_call.name->lexeme,
                    node->as.function_call.param_count);
      for (int i = 0; i < node->as.function_call.param_count; i++) {
        print_ast(output, node->as.function_call.parameters[i], indent + 1);
      }
      break;

    case AST_IF_STATEMENT:
      (void)fprintf(output, "If Statement:\n");
      print_indent(output, indent + 1);
      (void)fprintf(output, "Condition:\n");
      print_ast(output, node->as.if_elif_else_statement.condition, indent + 2);
      print_indent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      print_ast(output, node->as.if_elif_else_statement.body, indent + 2);
      break;

    case AST_ELSE_IF_STATEMENT:
      (void)fprintf(output, "Else If Statement:\n");
      print_indent(output, indent + 1);
      (void)fprintf(output, "Condition:\n");
      print_ast(output, node->as.if_elif_else_statement.condition, indent + 2);
      print_indent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      print_ast(output, node->as.if_elif_else_statement.body, indent + 2);
      break;

    case AST_ELSE_STATEMENT:
      (void)fprintf(output, "Else Statement:\n");
      print_indent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      print_ast(output, node->as.if_elif_else_statement.body, indent + 2);
      break;

    case AST_WHILE_STATEMENT:
      (void)fprintf(output, "While Statement:\n");
      print_indent(output, indent + 1);
      (void)fprintf(output, "Condition:\n");
      print_ast(output, node->as.while_statement.condition, indent + 2);
      print_indent(output, indent + 1);
      (void)fprintf(output, "Body:\n");
      print_ast(output, node->as.while_statement.body, indent + 2);
      break;

    case AST_BLOCK:
      (void)fprintf(output, "Block with %d statement(s):\n",
                    node->as.block.count);
      for (int i = 0; i < node->as.block.count; i++) {
        print_ast(output, node->as.block.statements[i], indent + 1);
      }
      break;

    case AST_RETURN:
      (void)fprintf(output, "Return Statement:\n");
      print_indent(output, indent + 1);
      (void)fprintf(output, "Expression:\n");
      print_ast(output, node->as._return.expression, indent + 2);
      break;

    default:
      (void)fprintf(output, "Unknown AST Node\n");
      break;
  }
}

/* -------------------------------------------------------------------------- */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void print_ast_output(ast_node** nodes, int count, int output_to_file) {
  // DEBUG_PRINT("Entering print_ast_output (count=%d, output_to_file=%d)",
  // count,
  //             output_to_file);

  FILE* output = NULL;
  if (output_to_file == 1) {
    output = fopen("ast.txt", "we");
    if (output == NULL) {
      error_and_exit("Error opening file 'ast'");
    }
  } else {
    output = stdout;
  }
  if (!output) {
    error_and_exit("Error opening file 'ast'");
    return;
  }
  (void)fprintf(output, "Printing AST for the entire file:\n");
  for (int i = 0; i < count; i++) {
    if (nodes[i] != NULL) {
      (void)fprintf(output, "\n--- AST Node %d ---\n", i);
      print_ast(output, nodes[i], 0);
    }
  }

  if (output_to_file) {
    (void)fclose(output);
  }

  DEBUG_PRINT("Exiting print_ast_output");
}

// NOLINTEND(clang-diagnostic-gnu-zero-variadic-macro-arguments)
