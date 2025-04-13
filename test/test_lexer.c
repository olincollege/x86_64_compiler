#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <string.h>

#include "../src/lexer.h"

// NOLINTBEGIN(*-magic-numbers)

Test(lexer, if_whitespace) {
  const char* source = "   if ";
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_IF);
  cr_assert(memcmp(token.lexeme, "if", token.length) == 0);
  cr_assert_eq(token.length, 2);
}

Test(lexer, not_equals) {
  const char* source = "    !=  ";
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_NEQ);
  cr_assert(memcmp(token.lexeme, "!=", token.length) == 0,
            "Expected token lexeme to be \"!=\"");
  cr_assert_eq(token.length, 2);
}

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

Test(lexer, int_literal) {
  const char* source = "12345";
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_INT_LITERAL);
  cr_assert(memcmp(token.lexeme, "12345", token.length) == 0,
            "Expected token lexeme to be \"12345\"");
  cr_assert_eq(token.length, 5);
}

Test(lexer, semicolon) {
  const char* source = "  ;";
  Lexer lexer;
  initLexer(&lexer, source);
  Token token = getNextToken(&lexer);
  cr_assert_eq(token.type, TOKEN_SEMICOLON);
  cr_assert(memcmp(token.lexeme, ";", token.length) == 0,
            "Expected token lexeme to be \";\"");
  cr_assert_eq(token.length, 1);
}

// NOLINTEND(*-magic-numbers)
