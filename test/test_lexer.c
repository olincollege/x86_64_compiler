#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer.h"

// NOLINTBEGIN(*-magic-numbers)

// helper function to read contents of file
static char* read_file(const char* filepath) {
  FILE* file = fopen(filepath, "re");
  cr_assert_neq(file, NULL, "Could not open file: %s", filepath);
  int ret = fseek(file, 0, SEEK_END);
  cr_assert_eq(ret, 0, "Failed to seek to end of file: %s", filepath);
  long length = ftell(file);
  cr_assert_neq(length, -1L, "ftell failed for file: %s", filepath);
  ret = fseek(file, 0, SEEK_SET);
  cr_assert_eq(ret, 0, "Failed to seek to start of file: %s", filepath);
  char* buffer = malloc((size_t)length + 1);
  cr_assert_neq(buffer, NULL, "Memory allocation failed for file: %s",
                filepath);
  size_t read_bytes = fread(buffer, 1, (size_t)length, file);
  cr_assert_eq(read_bytes, (size_t)length, "Failed to read complete file: %s",
               filepath);
  buffer[length] = '\0';
  ret = fclose(file);
  cr_assert_eq(ret, 0, "Failed to close file: %s", filepath);
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

// Test that verifies a sequence of tokens via file input
Test(lexer, multiple_tokens_sequence_file) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/multi_tok_seq.txt");
  Lexer lexer;
  initLexer(&lexer, source);

  Token token1 = getNextToken(&lexer);  // int
  cr_assert_eq(token1.type, TOKEN_INT_TYPE);

  Token token2 = getNextToken(&lexer);  // x
  cr_assert_eq(token2.type, TOKEN_IDENTIFIER);

  Token token3 = getNextToken(&lexer);  // =
  cr_assert_eq(token3.type, TOKEN_ASSIGN);

  Token token4 = getNextToken(&lexer);  // 42
  cr_assert_eq(token4.type, TOKEN_INT_LITERAL);

  Token token5 = getNextToken(&lexer);  // ;
  cr_assert_eq(token5.type, TOKEN_SEMICOLON);

  Token token6 = getNextToken(&lexer);  // EOF
  cr_assert_eq(token6.type, TOKEN_EOF);

  free(source);
}

// Test that verifies compound operators via file input
Test(lexer, compound_operators_file) {
  char* source = read_file(
      CMAKE_SOURCE_DIR "/test/test_inputs/lexer_inputs/compound_operators.txt");
  Lexer lexer;
  initLexer(&lexer, source);

  cr_assert_eq(getNextToken(&lexer).type, TOKEN_EQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_NEQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_LEQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_GEQ);
  cr_assert_eq(getNextToken(&lexer).type, TOKEN_EOF);

  free(source);
}

// Test that verifies identifiers with underscores & digits via file input
Test(lexer, identifier_with_underscore_and_digits_file) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/"
                           "identifier.txt");
  Lexer lexer;
  initLexer(&lexer, source);

  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_IDENTIFIER);
  cr_assert_eq(token.length, 9);
  cr_assert(strncmp(token.lexeme, "_myVar123", token.length) == 0);

  free(source);
}

// // Test that verifies skipping a single-line comment via file input
// Test(lexer, comment_skipping_file) {
//   char* source = read_file(
//       CMAKE_SOURCE_DIR "/test/test_inputs/lexer_inputs/comment_skipping.txt");
//   Lexer lexer;
//   initLexer(&lexer, source);

//   Token token = getNextToken(&lexer);
//   cr_assert_eq(token.type, TOKEN_INT_LITERAL);
//   cr_assert(strncmp(token.lexeme, "42", token.length) == 0);

//   free(source);
// }

// Test that verifies the lone '!' produces an error token via file input
Test(lexer, invalid_exclamation_file) {
  char* source =
      read_file(CMAKE_SOURCE_DIR
                "/test/test_inputs/lexer_inputs/invalid_exclamation.txt");
  Lexer lexer;
  initLexer(&lexer, source);

  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_UNKNOWN);
  cr_assert(strncmp(token.lexeme, "Unexpected '!'", token.length) == 0);

  free(source);
}

// Test that verifies an unexpected character via file input
Test(lexer, unexpected_character_file) {
  char* source =
      read_file(CMAKE_SOURCE_DIR
                "/test/test_inputs/lexer_inputs/unexpected_character.txt");
  Lexer lexer;
  initLexer(&lexer, source);

  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_UNKNOWN);

  free(source);
}

// Test that verifies line counting across newlines via file input
Test(lexer, multiline_tokens_and_line_count_file) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/multiline.txt");
  Lexer lexer;
  initLexer(&lexer, source);

  cr_assert_eq(getNextToken(&lexer).line, 1);  // int
  cr_assert_eq(getNextToken(&lexer).line, 2);  // x
  cr_assert_eq(getNextToken(&lexer).line, 3);  // =
  cr_assert_eq(getNextToken(&lexer).line, 4);  // 123
  cr_assert_eq(getNextToken(&lexer).line, 4);  // ;

  free(source);
}

// // Test that verifies mixed comments + tokens via file input
// Test(lexer, multiple_comments_and_tokens_file) {
//   char* source = read_file(
//       CMAKE_SOURCE_DIR "/test/test_inputs/lexer_inputs/multiple_com_tok.txt");
//   Lexer lexer;
//   initLexer(&lexer, source);

//   cr_assert_eq(getNextToken(&lexer).type, TOKEN_INT_TYPE);
//   cr_assert_eq(getNextToken(&lexer).type, TOKEN_IDENTIFIER);
//   cr_assert_eq(getNextToken(&lexer).type, TOKEN_ASSIGN);
//   cr_assert_eq(getNextToken(&lexer).type, TOKEN_INT_LITERAL);
//   cr_assert_eq(getNextToken(&lexer).type, TOKEN_SEMICOLON);
//   cr_assert_eq(getNextToken(&lexer).type, TOKEN_EOF);

//   free(source);
// }

// Test that verifies keywords embedded in identifiers via file input
Test(lexer, keyword_as_prefix_identifier_file) {
  char* source = read_file(CMAKE_SOURCE_DIR
                           "/test/test_inputs/lexer_inputs/keyword_pre_id.txt");
  Lexer lexer;
  initLexer(&lexer, source);

  Token token1 = getNextToken(&lexer);
  cr_assert_eq(token1.type, TOKEN_IDENTIFIER);  // "intif"

  Token token2 = getNextToken(&lexer);
  cr_assert_eq(token2.type, TOKEN_IDENTIFIER);  // "iffy"

  free(source);
}

// NOLINTEND(*-magic-numbers)
