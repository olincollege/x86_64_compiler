#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
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

  initLexer(&lexer, source);

  Token token = getNextToken(&lexer);

  Token* tokens = malloc(sizeof(Token) * 1000);  // Arbitrary size for demo
  int tokenIndex = 0;
  tokens[tokenIndex++] = token;

  while (token.type != TOKEN_EOF) {
    printToken(&token);
    token = getNextToken(&lexer);
    tokens[tokenIndex++] = token;
  }

  // free(source);
  fclose(file);
  parseFile(tokens, tokenIndex);

  return 0;
}
