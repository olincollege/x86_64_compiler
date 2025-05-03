#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer.h"
#include "../src/parser.h"

// helper to read a file into a string
static char* read_file(const char* path) {
  FILE* f = fopen(path, "r");
  cr_assert_neq(f, NULL, "Could not open %s", path);
  fseek(f, 0, SEEK_END);
  size_t len = (size_t)ftell(f);
  rewind(f);
  char* buf = malloc(len + 1);
  cr_assert_neq(buf, NULL, "Alloc failed");
  fread(buf, 1, len, f);
  buf[len] = '\0';
  fclose(f);
  return buf;
}

// tokenize entire source into a dynamically sized array of Tokens
static Token* lex_all(const char* src, int* out_count) {
  Lexer lx;
  initLexer(&lx, src);
  int capacity = 128, count = 0;
  Token* toks = malloc(sizeof(Token) * capacity);
  cr_assert_not_null(toks);
  Token tok;
  do {
    tok = getNextToken(&lx);
    if (count >= capacity) {
      capacity *= 2;
      toks = realloc(toks, sizeof(Token) * capacity);
      cr_assert_not_null(toks);
    }
    toks[count++] = tok;
  } while (tok.type != TOKEN_EOF);
  *out_count = count;
  return toks;
}

// count how many non-NULL ASTNode* in the returned array
static int ast_count(ASTNode** ast) {
  int n = 0;
  while (ast[n] != NULL) n++;
  return n;
}

Test(parser, empty_function) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/empty_function.txt");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);

  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_not_null(ast, "parseFile returned NULL");
  cr_expect_eq(ast_count(ast), 1, "should find exactly one function");

  ASTNode* fn = ast[0];
  cr_expect_eq(fn->type, AST_FUNCTION_DECLARATION);
  // check that the function body is an empty block
  ASTNode* body = fn->as.function.statements;
  cr_expect_eq(body->type, AST_BLOCK);
  cr_expect_eq(body->as.block.count, 0);

  free(src);
  free(toks);
}

Test(parser, simple_return) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/simple_return.txt");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);

  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* fn = ast[0];
  ASTNode* body = fn->as.function.statements;
  cr_expect_eq(body->type, AST_BLOCK);
  cr_expect_eq(body->as.block.count, 1);

  // the statement should be a return of literal 42
  ASTNode* retStmt = body->as.block.statements[0];
  cr_expect_eq(retStmt->type, AST_RETURN);
  int val = retStmt->as._return.expression->as.intLiteral.intLiteral;
  cr_expect_eq(val, 42);

  free(src);
  free(toks);
}

Test(parser, complex_main) {
  char* src = read_file(CMAKE_SOURCE_DIR
                        "/test/test_inputs/parser_inputs/complex_main.txt");
  int tokc = 0;
  Token* toks = lex_all(src, &tokc);

  ASTNode** ast = parseFile(toks, tokc);
  cr_assert_eq(ast_count(ast), 1);

  ASTNode* fn = ast[0];
  ASTNode* body = fn->as.function.statements;
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
