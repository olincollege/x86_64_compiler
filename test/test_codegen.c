#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/codegen.h"
#include "../src/lexer.h"
#include "../src/parser.h"

// Read a file into a null-terminated buffer
static char* read_file(const char* path) {
  FILE* f = fopen(path, "r");
  cr_assert_not_null(f, "Could not open file: %s", path);
  (void)fseek(f, 0, SEEK_END);
  size_t len = (size_t)ftell(f);
  (void)rewind(f);
  char* buf = malloc(len + 1);
  cr_assert_not_null(buf, "Malloc failed");
  (void)fread(buf, 1, len, f);
  buf[len] = '\0';
  (void)fclose(f);
  return buf;
}

// Lex a source string into a token array
static Token* lex_all(const char* src, int* out_count) {
  Lexer lx;
  initLexer(&lx, src);
  int cap = 128, count = 0;
  Token* toks = malloc(sizeof(Token) * cap);
  cr_assert_not_null(toks);

  Token tok;
  do {
    tok = getNextToken(&lx);
    if (count >= cap) {
      cap *= 2;
      toks = realloc(toks, sizeof(Token) * cap);
      cr_assert_not_null(toks);
    }
    toks[count++] = tok;
  } while (tok.type != TOKEN_EOF);
  *out_count = count;
  return toks;
}

// Count number of non-null AST nodes
static int ast_count(ASTNode** ast) {
  int n = 0;
  while (ast[n] != NULL) n++;
  return n;
}

// Test 1: Return constant
Test(codegen, return_constant) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/codegen_inputs/simple_codegen.txt");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int foundMain = 0, foundMov = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    if (strstr(list.instructions[i], "main:")) {
      foundMain = 1;
    }
    if (strstr(list.instructions[i], "mov     eax, 42")) {
      foundMov = 1;
    }
  }

  cr_expect(foundMain, "Missing 'main:' label");
  cr_expect(foundMov, "Missing 'mov eax, 42' instruction");

  free(src);
  free(toks);
}

// Test 2: Return binary expression
Test(codegen, return_binary_expression) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/codegen_inputs/binary_return.txt");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int found6 = 0, found2 = 0, foundAdd = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    if (strstr(list.instructions[i], "mov     edx, 2")) {
      found2 = 1;
    }
    if (strstr(list.instructions[i], "mov     eax, 6")) {
      found6 = 1;
    }
    if (strstr(list.instructions[i], "add     eax, edx")) {
      foundAdd = 1;
    }
  }

  cr_expect(found6, "Expected: mov eax, 6");
  cr_expect(found2, "Expected: mov edx, 2");
  cr_expect(foundAdd, "Expected: add eax, edx");

  free(src);
  free(toks);
}

// Test 3: Function call
Test(codegen, function_call) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/codegen_inputs/func_call.txt");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int foundCall = 0, foundEdi = 0, foundEsi = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    if (strstr(list.instructions[i], "call    test")) {
      foundCall = 1;
    }
    if (strstr(list.instructions[i], "mov     edi, eax")) {
      foundEdi = 1;
    }
    if (strstr(list.instructions[i], "mov     esi, eax")) {
      foundEsi = 1;
    }
  }

  cr_expect(foundCall, "Expected: call test");
  cr_expect(foundEdi, "Expected: mov edi, <value>");
  cr_expect(foundEsi, "Expected: mov esi, <value>");

  free(src);
  free(toks);
}
