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

// NOLINTEND(*-magic-numbers)
