/*
 * Lexer
 * A simple lexer for tokenizing C source code.
 */

#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int atEnd(Lexer* lexer) { return *lexer->current == '\0'; }

static char advance(Lexer* lexer) {
  lexer->current++;
  return lexer->current[-1];
}

static char peek(Lexer* lexer) { return *lexer->current; }

static char peekNext(Lexer* lexer) {
  if (atEnd(lexer)) {
    return '\0';
  }
  return lexer->current[-1];
}

static void skipWhitespace(Lexer* lexer) {
  for (;;) {
    char chrc = peek(lexer);
    switch (chrc) {
      case ' ':
      case '\r':
      case '\t':
        advance(lexer);
        break;
      case '\n':
        lexer->line++;
        advance(lexer);
        break;
      // skipping comments
      case '/':
        if (peekNext(lexer) == '/') {
          while (peek(lexer) != '\n' && !atEnd(lexer)) {
            advance(lexer);
          }
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static Token makeToken(Lexer* lexer, TokenType type, const char* start,
                       int length) {
  Token token;
  token.type = type;
  token.lexeme = start;
  token.length = length;
  token.line = lexer->line;
  return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
  return makeToken(lexer, TOKEN_UNKNOWN, message, (int)strlen(message));
}

static int isAlphabetic(char chrc) {
  return (chrc >= 'a' && chrc <= 'z') || (chrc >= 'A' && chrc <= 'Z') ||
         chrc == '_';
}

static int isDigit(char chrc) { return chrc >= '0' && chrc <= '9'; }

static Token scanNumber(Lexer* lexer) {
  const char* start = lexer->current - 1;
  while (isDigit(peek(lexer))) {
    advance(lexer);
  }
  int length = (int)(lexer->current - start);
  return makeToken(lexer, TOKEN_INT, start, length);
}

TokenType identifierType(const char* text, int length) {
  if (length == 2 && strncmp(text, "if", 2) == 0) {
    return TOKEN_IF;
  }
  if (length == 4 && strncmp(text, "else", 4) == 0) {
    return TOKEN_ELSE;
  }
  if (length == LENGTH_5 && strncmp(text, "while", LENGTH_5) == 0) {
    return TOKEN_WHILE;
  }
  if (length == 3 && strncmp(text, "for", 3) == 0) {
    return TOKEN_FOR;
  }
  if (length == LENGTH_6 && strncmp(text, "return", LENGTH_6) == 0) {
    return TOKEN_RETURN;
  }
  if (length == 3 && strncmp(text, "int", 3) == 0) {
    return TOKEN_INT_TYPE;
  }
  return TOKEN_IDENTIFIER;
}

static Token scanIdentifier(Lexer* lexer) {
  const char* start = lexer->current - 1;
  while (isAlphabetic(peek(lexer)) || isDigit(peek(lexer))) {
    advance(lexer);
  }
  int length = (int)(lexer->current - start);
  return makeToken(lexer, identifierType(start, length), start, length);
}

Token getNextToken(Lexer* lexer) {
  skipWhitespace(lexer);
  lexer->start = lexer->current;
  if (atEnd(lexer)) {
    return makeToken(lexer, TOKEN_EOF, lexer->start, 0);
  }
  char chrc = advance(lexer);
  if (isAlphabetic(chrc)) {
    return scanIdentifier(lexer);
  }
  if (isDigit(chrc)) {
    return scanNumber(lexer);
  }
  switch (chrc) {
    case '(':
      return makeToken(lexer, TOKEN_LPAREN, lexer->start, 1);
    case ')':
      return makeToken(lexer, TOKEN_RPAREN, lexer->start, 1);
    case '{':
      return makeToken(lexer, TOKEN_LBRACE, lexer->start, 1);
    case '}':
      return makeToken(lexer, TOKEN_RBRACE, lexer->start, 1);
    case ';':
      return makeToken(lexer, TOKEN_SEMICOLON, lexer->start, 1);
    case ',':
      return makeToken(lexer, TOKEN_COMMA, lexer->start, 1);
    case '+':
      return makeToken(lexer, TOKEN_PLUS, lexer->start, 1);
    case '-':
      return makeToken(lexer, TOKEN_MINUS, lexer->start, 1);
    case '*':
      return makeToken(lexer, TOKEN_STAR, lexer->start, 1);
    case '/':
      return makeToken(lexer, TOKEN_SLASH, lexer->start, 1);
    case '%':
      return makeToken(lexer, TOKEN_PERCENT, lexer->start, 1);
    case '=': {
      // equal equals
      if (peek(lexer) == '=') {
        advance(lexer);
        return makeToken(lexer, TOKEN_EQ, lexer->start, 2);
      }
      return makeToken(lexer, TOKEN_ASSIGN, lexer->start, 1);
    }
    case '!': {
      // not equals
      if (peek(lexer) == '=') {
        advance(lexer);
        return makeToken(lexer, TOKEN_NEQ, lexer->start, 2);
      }
      return errorToken(lexer, "Unexpected '!'");
    }
    case '<': {
      if (peek(lexer) == '=') {
        advance(lexer);
        return makeToken(lexer, TOKEN_LEQ, lexer->start, 2);
      }
      return makeToken(lexer, TOKEN_LT, lexer->start, 1);
    }
    case '>': {
      if (peek(lexer) == '=') {
        advance(lexer);
        return makeToken(lexer, TOKEN_GEQ, lexer->start, 2);
      }
      return makeToken(lexer, TOKEN_GT, lexer->start, 1);
    }
    default:
      return errorToken(lexer, "Unexpected character.");
  }
}

void initLexer(Lexer* lexer, const char* source) {
  lexer->start = source;
  lexer->current = source;
  lexer->line = 1;
}
