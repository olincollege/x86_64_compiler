#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer.h"

// NOLINTBEGIN(*-magic-numbers)

// helper function to read contents of file
static char* read_file(const char* filepath) {
  FILE* file = fopen(filepath, "r");
  cr_assert_neq(file, NULL, "Could not open file: %s", filepath);

  (void)fseek(file, 0, SEEK_END);
  long length = ftell(file);
  rewind(file);

  char* buffer = malloc((size_t)length + 1);
  cr_assert_neq(buffer, NULL, "Memory allocation failed for file: %s",
                filepath);

  size_t read_bytes = fread(buffer, 1, (size_t)length, file);
  cr_assert_eq(read_bytes, (size_t)length, "Failed to read complete file: %s",
               filepath);
  buffer[length] = '\0';

  (void)fclose(file);
  return buffer;
}

// Test that verifies the lexer skips leading whitespace and correctly parses
// "if"
Test(lexer, if_whitespace) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/if_whitespace.txt");
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_IF);
  cr_assert(memcmp(token.lexeme, "if", token.length) == 0,
            "Expected token lexeme to be \"if\"");
  cr_assert_eq(token.length, 2);
  free(source);
}

// Test that verifies the lexer recognizes the "!=" operator
Test(lexer, not_equals) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/not_equals.txt");
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_NEQ);
  cr_assert(memcmp(token.lexeme, "!=", (size_t)token.length) == 0,
            "Expected token lexeme to be \"!=\"");
  cr_assert_eq(token.length, 2);
  free(source);
}

// Test that verifies the lexer recognizes integer literals
Test(lexer, int_literal) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/int_literal.txt");
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_INT_LITERAL);
  cr_assert(memcmp(token.lexeme, "12345", (size_t)token.length) == 0,
            "Expected token lexeme to be \"12345\"");
  cr_assert_eq(token.length, 5);
  free(source);
}

// Test that verifies the lexer recognizes a semicolon
Test(lexer, semicolon) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/semicolon.txt");
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_SEMICOLON);
  cr_assert(memcmp(token.lexeme, ";", (size_t)token.length) == 0,
            "Expected token lexeme to be \";\"");
  cr_assert_eq(token.length, 1);
  free(source);
}

// Tests for keyword detection (these don’t use file input because they test the
// identifierType function directly)
Test(lexer, keyword_detection_while) {
  const char* text = "while";
  TokenType type = identifierType(text, 5);
  cr_assert_eq(type, TOKEN_WHILE);
}

Test(lexer, keyword_detection_return) {
  const char* text = "return";
  TokenType type = identifierType(text, 6);
  cr_assert_eq(type, TOKEN_RETURN);
}

Test(lexer, multiple_tokens_sequence) {
  const char* src = "int x = 42;";
  Lexer lexer;
  initLexer(&lexer, src);

  Token t1 = getNextToken(&lexer); // int
  cr_assert_eq(t1.type, TOKEN_INT_TYPE);

  Token t2 = getNextToken(&lexer); // x
  cr_assert_eq(t2.type, TOKEN_IDENTIFIER);

  Token t3 = getNextToken(&lexer); // =
  cr_assert_eq(t3.type, TOKEN_ASSIGN);

  Token t4 = getNextToken(&lexer); // 42
  cr_assert_eq(t4.type, TOKEN_INT_LITERAL);

  Token t5 = getNextToken(&lexer); // ;
  cr_assert_eq(t5.type, TOKEN_SEMICOLON);

  Token t6 = getNextToken(&lexer); // EOF
  cr_assert_eq(t6.type, TOKEN_EOF);
}

Test(lexer, compound_operators) {
  const char* src = "== != <= >=";
  Lexer lexer;
  initLexer(&lexer, src);

  cr_assert_eq(getNextToken(&lexer).type, TOKEN_EQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_NEQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_LEQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_GEQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_EOF);
}

Test(lexer, identifier_with_underscore_and_digits) {
  const char* src = "_myVar123";
  Lexer lexer;
  initLexer(&lexer, src);

  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_IDENTIFIER);
  cr_assert_eq(token.length, 9);
  cr_assert(strncmp(token.lexeme, "_myVar123", token.length) == 0);
}

Test(lexer, invalid_exclamation) {
  const char* src = "!";
  Lexer lexer;
  initLexer(&lexer, src);

  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_UNKNOWN);
  cr_assert(strncmp(token.lexeme, "Unexpected '!'", token.length) == 0);
}

Test(lexer, unexpected_character) {
  const char* src = "$";
  Lexer lexer;
  initLexer(&lexer, src);

  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_UNKNOWN);
}

Test(lexer, multiline_tokens_and_line_count) {
  const char* src = "int\nx\n=\n123;";
  Lexer lexer;
  initLexer(&lexer, src);

  cr_assert_eq(getNextToken(&lexer).line, 1); // int
  cr_assert_eq(getNextToken(&lexer).line, 2); // x
  cr_assert_eq(getNextToken(&lexer).line, 3); // =
  cr_assert_eq(getNextToken(&lexer).line, 4); // 123
  cr_assert_eq(getNextToken(&lexer).line, 4); // ;
}


Test(lexer, keyword_as_prefix_identifier) {
  const char* src = "intif iffy";
  Lexer lexer;
  initLexer(&lexer, src);

  Token t1 = getNextToken(&lexer);
  cr_assert_eq(t1.type, TOKEN_IDENTIFIER); // "intif"

  Token t2 = getNextToken(&lexer);
  cr_assert_eq(t2.type, TOKEN_IDENTIFIER); // "iffy"
}

// NOLINTEND(*-magic-numbers)
