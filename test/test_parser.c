// NOLINTBEGIN(misc-include-cleaner)
// we checked to make sure only criterian related warnings were left
// NOLINTBEGIN(*-magic-numbers)

#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer.h"
#include "../src/parser.h"

// helper to read a file into a string
static char* read_file(const char* path) {
  FILE* file = fopen(path, "re");
  cr_assert_neq(file, NULL, "Could not open %s", path);
  cr_assert_eq(fseek(file, 0, SEEK_END), 0, "Failed to seek to end of file: %s",
               path);
  long tmp = ftell(file);
  cr_assert(tmp >= 0, "ftell failed on %s", path);
  cr_assert_eq(fseek(file, 0, SEEK_SET), 0,
               "Failed to seek back to start of file: %s", path);
  size_t len = (size_t)tmp;
  char* buf = malloc(len + 1);
  cr_assert_neq(buf, NULL, "Alloc failed");
  cr_assert_eq(fread(buf, 1, len, file), len, "Failed to read full file: %s",
               path);
  buf[len] = '\0';
  cr_assert_eq(fclose(file), 0, "Failed to close file: %s", path);
  return buf;
}

// tokenize entire source into a dynamically sized array of Tokens
static Token* lex_all(const char* src, int* out_count) {
  Lexer lex;
  initLexer(&lex, src);

  int capacity = 128;
  int count = 0;
  Token* toks = malloc(sizeof(Token) * (size_t)capacity);
  cr_assert_not_null(toks);

  Token tok;
  do {
    tok = getNextToken(&lex);

    if (count >= capacity) {
      capacity *= 2;
      size_t new_size = sizeof(Token) * (size_t)capacity;
      Token* tmp = realloc(toks, new_size);
      cr_assert_not_null(tmp, "Could not realloc token buffer to %zu bytes",
                         new_size);
      toks = tmp;
    }

    toks[count++] = tok;
  } while (tok.type != TOKEN_EOF);

  *out_count = count;
  return toks;
}

// count how many non-NULL ASTNode* in the returned array
static int ast_count(ASTNode** ast) {
  int node = 0;
  while (ast[node] != NULL) {
    node++;
  }
  return node;
}

Test(parser, empty_function) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/empty_function.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);

  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast, "parseFile returned NULL");
  cr_expect_eq(ast_count(ast), 1, "should find exactly one function");

  ASTNode* func = ast[0];
  cr_expect_eq(func->type, AST_FUNCTION_DECLARATION);
  // check that the function body is an empty block
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->type, AST_BLOCK);
  cr_expect_eq(body->as.block.count, 0);

  free(src);
  free(toks);
}

Test(parser, simple_return) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/simple_return.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);

  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->type, AST_BLOCK);
  cr_expect_eq(body->as.block.count, 1);

  // the statement should be a return of literal 3
  ASTNode* retStmt = body->as.block.statements[0];
  cr_expect_eq(retStmt->type, AST_RETURN);
  int val = retStmt->as._return.expression->as.intLiteral.intLiteral;
  cr_expect_eq(val, 3);

  free(src);
  free(toks);
}

Test(parser, complex_main) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/complex_main.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);

  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->type, AST_BLOCK);
  // a, b, c decls + while + if + else-if + else + return
  cr_expect_eq(body->as.block.count, 8);

  // check presence and ordering of control structures
  cr_expect_eq(body->as.block.statements[3]->type, AST_WHILE_STATEMENT);
  cr_expect_eq(body->as.block.statements[4]->type, AST_IF_STATEMENT);
  cr_expect_eq(body->as.block.statements[5]->type, AST_ELSE_IF_STATEMENT);
  cr_expect_eq(body->as.block.statements[6]->type, AST_ELSE_STATEMENT);

  // final return 0
  ASTNode* last = body->as.block.statements[7];
  cr_expect_eq(last->type, AST_RETURN);
  cr_expect_eq(last->as._return.expression->as.intLiteral.intLiteral, 0);

  free(src);
  free(toks);
}

// Test 4: Variable declaration inside function
Test(parser, variable_declaration) {
  char* src =
      read_file(CMAKE_SOURCE_DIR "/test/test_inputs/parser_inputs/var_decl.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->type, AST_BLOCK);
  cr_expect_eq(body->as.block.count, 2);

  // int x;
  ASTNode* decl = body->as.block.statements[0];
  cr_expect_eq(decl->type, AST_VARIABLE_DECLARATION);

  // name == "x"
  {
    int len = decl->as.variable_declaration.name->length;
    cr_expect_eq(len, 1, "variable name length");
    char buf[2];
    memcpy(buf, decl->as.variable_declaration.name->lexeme, len);
    buf[len] = '\0';
    cr_expect_str_eq(buf, "x");
  }

  // type == "int"
  {
    int len = decl->as.variable_declaration.type->length;
    cr_expect_eq(len, 3, "variable type length");
    char buf[4];
    memcpy(buf, decl->as.variable_declaration.type->lexeme, (size_t)len);
    buf[len] = '\0';
    cr_expect_str_eq(buf, "int");
  }

  free(src);
  free(toks);
}

// Test 5: Function with parameters
Test(parser, function_with_parameters) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/func_params.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  cr_expect_eq(func->type, AST_FUNCTION_DECLARATION);
  cr_expect_eq(func->as.function.paramCount, 2);

  // param 0: name "a", type "int"
  {
    ASTNode* param0 = func->as.function.parameters[0];
    cr_expect_eq(param0->type, AST_VARIABLE_DECLARATION);

    int nlen = param0->as.variable_declaration.name->length;
    cr_expect_eq(nlen, 1, "param0 name length");
    char nbuf[2];
    memcpy(nbuf, param0->as.variable_declaration.name->lexeme, nlen);
    nbuf[nlen] = '\0';
    cr_expect_str_eq(nbuf, "a");

    int tlen = param0->as.variable_declaration.type->length;
    cr_expect_eq(tlen, 3, "param0 type length");
    char tbuf[4];
    memcpy(tbuf, param0->as.variable_declaration.type->lexeme, tlen);
    tbuf[tlen] = '\0';
    cr_expect_str_eq(tbuf, "int");
  }

  // param 1: name "b", type "int"
  {
    ASTNode* param1 = func->as.function.parameters[1];
    cr_expect_eq(param1->type, AST_VARIABLE_DECLARATION);

    int nlen = param1->as.variable_declaration.name->length;
    cr_expect_eq(nlen, 1, "param1 name length");
    char nbuf[2];
    memcpy(nbuf, param1->as.variable_declaration.name->lexeme, nlen);
    nbuf[nlen] = '\0';
    cr_expect_str_eq(nbuf, "b");

    int tlen = param1->as.variable_declaration.type->length;
    cr_expect_eq(tlen, 3, "param1 type length");
    char tbuf[4];
    memcpy(tbuf, param1->as.variable_declaration.type->lexeme, tlen);
    tbuf[tlen] = '\0';
    cr_expect_str_eq(tbuf, "int");
  }

  free(src);
  free(toks);
}

// Test 6: Assignment and return of variable
Test(parser, assignment_and_return) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/assign_and_return.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->as.block.count, 3);

  // check declaration int a;
  cr_expect_eq(body->as.block.statements[0]->type, AST_VARIABLE_DECLARATION);
  // check assignment a = 4;
  cr_expect_eq(body->as.block.statements[1]->type, AST_DECLARATION);
  // check return a;
  cr_expect_eq(body->as.block.statements[2]->type, AST_RETURN);

  free(src);
  free(toks);
}

// Test 7: nested if without else
Test(parser, nested_if_without_else) {
  char* src =
      read_file(CMAKE_SOURCE_DIR "/test/test_inputs/parser_inputs/nested_if.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->as.block.count, 2);
  cr_expect_eq(body->as.block.statements[0]->type, AST_IF_STATEMENT);
  cr_expect_eq(body->as.block.statements[1]->type, AST_RETURN);

  free(src);
  free(toks);
}

// Test 8: while loop parsing
Test(parser, while_loop) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/while_loop.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->as.block.count, 2);
  cr_expect_eq(body->as.block.statements[0]->type, AST_WHILE_STATEMENT);
  cr_expect_eq(body->as.block.statements[1]->type, AST_RETURN);

  free(src);
  free(toks);
}

// Test 9: void function with no params
Test(parser, void_function) {
  char* src =
      read_file(CMAKE_SOURCE_DIR "/test/test_inputs/parser_inputs/void_func.c");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);
  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* func = ast[0];
  cr_expect_eq(func->type, AST_FUNCTION_DECLARATION);
  cr_expect_eq(func->as.function.paramCount, 0);
  ASTNode* body = func->as.function.statements;
  cr_expect_eq(body->as.block.count, 0);

  free(src);
  free(toks);
}

// NOLINTEND(*-magic-numbers)
// NOLINTEND(misc-include-cleaner)
