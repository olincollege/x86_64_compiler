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
                        "/test/test_inputs/codegen_inputs/simple_codegen.c");
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
                        "/test/test_inputs/codegen_inputs/binary_return.c");
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
                        "/test/test_inputs/codegen_inputs/func_call.c");
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

// Test 4: Variable declaration with initialization
Test(codegen, var_decl_initialization) {
  char* src = read_file(
      CMAKE_SOURCE_DIR "/test/test_inputs/codegen_inputs/var_decl.c");
  int tokc;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int foundMov5 = 0;
  int foundStore = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    const char* ins = list.instructions[i];
    if (strstr(ins, "mov     eax, 5")) foundMov5 = 1;
    if (strstr(ins, "mov     DWORD PTR [rbp-4]")) foundStore = 1;
  }

  cr_expect(foundMov5, "Expected: mov     eax, 5");
  cr_expect(foundStore, "Expected: mov     DWORD PTR [rbp-4], eax");

  free(src);
  free(toks);
}

// Test 5: Multiplication operator
Test(codegen, multiplication) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/codegen_inputs/multiply.c");
  int tokc;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int foundMov3 = 0;
  int foundMov7 = 0;
  int foundImul = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    const char* ins = list.instructions[i];
    if (strstr(ins, "mov     edx, 3")) foundMov3 = 1;
    if (strstr(ins, "mov     eax, 7")) foundMov7 = 1;
    if (strstr(ins, "imul     eax, edx")) foundImul = 1;
  }

  cr_expect(foundMov7, "Expected: mov     eax, 7");
  cr_expect(foundMov3, "Expected: mov     edx, 3");
  cr_expect(foundImul, "Expected: imul    eax, edx");

  free(src);
  free(toks);
}

// Test 6: Division operator
Test(codegen, division) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/codegen_inputs/division.c");
  int tokc;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int foundMov2 = 0;
  int foundMov10 = 0;
  int foundIdiv = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    const char* ins = list.instructions[i];
    if (strstr(ins, "mov     edx, 2")) foundMov2 = 1;
    if (strstr(ins, "mov     eax, 10")) foundMov10 = 1;
    if (strstr(ins, "idiv     eax, edx")) foundIdiv = 1;
  }

  cr_expect(foundMov10, "Expected: mov     eax, 10");
  cr_expect(foundMov2, "Expected: mov     edx, 2");
  cr_expect(foundIdiv, "Expected: idiv    eax, edx");

  free(src);
  free(toks);
}

// Test 7: Function call with no arguments
Test(codegen, function_call_no_args) {
  char* src =
      read_file(CMAKE_SOURCE_DIR
                "/test/test_inputs/codegen_inputs/func_call_no_args.c");
  int tokc;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int foundFooLabel = 0;
  int foundCallFoo = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    const char* ins = list.instructions[i];
    if (strstr(ins, "foo:")) foundFooLabel = 1;
    if (strstr(ins, "call    foo")) foundCallFoo = 1;
  }

  cr_expect(foundFooLabel, "Expected: foo: label");
  cr_expect(foundCallFoo, "Expected: call    foo");

  free(src);
  free(toks);
}

// Test 8: Multiple function definitions
Test(codegen, multiple_functions) {
  char* src = read_file(
      CMAKE_SOURCE_DIR "/test/test_inputs/codegen_inputs/multiple_func.c");
  int tokc;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int countFoo = 0;
  int countMain = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    if (strstr(list.instructions[i], "foo:")) countFoo++;
    if (strstr(list.instructions[i], "main:")) countMain++;
  }

  cr_expect_eq(countFoo, 1, "Expected exactly one foo: label");
  cr_expect_eq(countMain, 1, "Expected exactly one main: label");

  free(src);
  free(toks);
}

// Test 9: Declaration + return of variable
Test(codegen, decl_and_return) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/codegen_inputs/decl_and_return.c");
  int tokc;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast);

  int numFns = ast_count(ast);
  listOfX86Instructions list;
  initListOfInstructions(&list);
  ListOfASTFunctionNodesToX86(ast, &list, numFns);
  printInstructions(&list);

  int foundMov9 = 0;
  int foundStoreX = 0;
  int foundLoadX = 0;
  for (int i = 0; i < list.instructionCount; i++) {
    const char* ins = list.instructions[i];
    if (strstr(ins, "mov     eax, 9")) foundMov9 = 1;
    if (strstr(ins, "mov     DWORD PTR [rbp-4]")) foundStoreX = 1;
    if (strstr(ins, "mov     eax, DWORD PTR [rbp-4]")) foundLoadX = 1;
  }

  cr_expect(foundMov9, "Expected: mov     eax, 9");
  cr_expect(foundStoreX, "Expected: mov     DWORD PTR [rbp-4], eax");
  cr_expect(foundLoadX, "Expected: mov     eax, DWORD PTR [rbp-4]");

  free(src);
  free(toks);
}
