#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG

#include "codegen.h"
#include "lexer.h"
#include "parser.h"

/**
 * main – Program entry point for the compiler front‑end.
 *
 * Opens the input file "test.txt" for reading; on failure, prints an error
 * message to stderr and returns 1. Otherwise, it:
 *   1. Reads the entire file into a dynamically allocated buffer.
 *   2. Initializes the Lexer and tokenizes the source into an array.
 *   3. Prints all tokens to stdout.
 *   4. Parses the tokens into an AST and prints the AST.
 *   5. Converts each AST function node into x86 instructions.
 *   6. Prints the generated instructions.
 *   7. Frees all allocated memory.
 *
 * Parameters:
 *   None.
 *
 * Return:
 *   0 on successful execution.
 *   1 if the source file cannot be opened.
 * :contentReference[oaicite:0]{index=0}:contentReference[oaicite:1]{index=1}
 */
int main() {
  FILE* file = fopen("test.txt", "r");
  if (!file) {
    fprintf(stderr, "Error opening file.\n");
    return 1;
  }

  char line[512];
  Lexer lexer;

  // Read the entire source code into a string
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  char* source = malloc(size + 1);
  fread(source, 1, size, file);
  source[size] = '\0';

  init_lexer(&lexer, source);

  Token token = get_next_token(&lexer);

  Token* tokens = malloc(sizeof(Token) * 1000);  // Arbitrary size for demo
  int token_index = 0;
  tokens[token_index++] = token;

  while (token.type != TOKEN_EOF) {
    token = get_next_token(&lexer);
    tokens[token_index++] = token;
  }
  token_index--;

  // free(source);
  // fclose(file);

  for (int i = 0; i < token_index; ++i) {
    print_token_both(&tokens[i], 1);
  }

  printf("\nParsing tokens...\n\n");

  ast_node** astNodes;
  printf("Printing AST...\n\n");

  astNodes = parse_file(tokens, token_index);

  printf("AST Nodes:\n");

  print_ast_output(astNodes, token_index, 1);

  // ast_node* expressionNode =
  // astNodes[1] = astNodes[0]->as.function.statements->as.block.statements[0];

  // ast_node* expressionNode = astNodes[1];

  print_ast_output(astNodes, token_index, 1);

  list_of_x86_instructions list;
  init_list_of_instructions(&list);

  memory mem;
  init_memory(&mem);

  printf("Before\n");

  list_of_ast_function_nodes_to_x86(astNodes, &list, token_index);
  // ast_declaration_node_to_x86(expressionNode, &list, &mem);

  printf("After\n");
  print_instructions(&list);

  // Cleanup
  free(source);
  free(tokens);
  return 0;

  // printASTFile(astNodes, token_index);
  // void print_ast_output(ast_node** nodes, int count, int outputToFile);

  // print_ast(astNodes[0], 0);  // Print the AST starting from the root node

  return 0;
}
