/*
 * Lexer
 * A simple lexer for tokenizing C source code.
 */

#include "lexer.h"

#include <stdio.h>
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
  return makeToken(lexer, TOKEN_INT_LITERAL, start, length);
}

TokenType identifierType(const char* text, int length) {
  if (length == strlen("if") && strncmp(text, "if", strlen("if")) == 0) {
    return TOKEN_IF;
  }
  if (length == strlen("else") && strncmp(text, "else", strlen("else")) == 0) {
    return TOKEN_ELSE;
  }
  if (length == strlen("while") &&
      strncmp(text, "while", strlen("while")) == 0) {
    return TOKEN_WHILE;
  }
  if (length == strlen("for") && strncmp(text, "for", strlen("for")) == 0) {
    return TOKEN_FOR;
  }
  if (length == strlen("return") &&
      strncmp(text, "return", strlen("return")) == 0) {
    return TOKEN_RETURN;
  }
  if (length == strlen("int") && strncmp(text, "int", strlen("int")) == 0) {
    return TOKEN_INT_TYPE;
  }
  if (length == strlen("void") && strncmp(text, "void", strlen("void")) == 0) {
    return TOKEN_VOID_TYPE;
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

// Code to print lexers and tokens

const char* tokenTypeToString(TokenType type) {
  switch (type) {
    case TOKEN_EOF:
      return "EOF";
    case TOKEN_INT_LITERAL:
      return "INT";
    case TOKEN_IDENTIFIER:
      return "IDENTIFIER";
    case TOKEN_IF:
      return "IF";
    case TOKEN_ELSE:
      return "ELSE";
    case TOKEN_WHILE:
      return "WHILE";
    case TOKEN_FOR:
      return "FOR";
    case TOKEN_RETURN:
      return "RETURN";
    case TOKEN_INT_TYPE:
      return "INT_TYPE";
    case TOKEN_PLUS:
      return "PLUS";
    case TOKEN_MINUS:
      return "MINUS";
    case TOKEN_STAR:
      return "STAR";
    case TOKEN_SLASH:
      return "SLASH";
    case TOKEN_PERCENT:
      return "PERCENT";
    case TOKEN_ASSIGN:
      return "ASSIGN";
    case TOKEN_EQ:
      return "EQ";
    case TOKEN_NEQ:
      return "NEQ";
    case TOKEN_LT:
      return "LT";
    case TOKEN_GT:
      return "GT";
    case TOKEN_LEQ:
      return "LEQ";
    case TOKEN_GEQ:
      return "GEQ";
    case TOKEN_SEMICOLON:
      return "SEMICOLON";
    case TOKEN_COMMA:
      return "COMMA";
    case TOKEN_LPAREN:
      return "LPAREN";
    case TOKEN_RPAREN:
      return "RPAREN";
    case TOKEN_LBRACE:
      return "LBRACE";
    case TOKEN_RBRACE:
      return "RBRACE";
    case TOKEN_UNKNOWN:
      return "UNKNOWN";
    default:
      return "INVALID";
  }
}

void printLexer(const Lexer* lexer) {
  printf("Lexer(start=\"%s\", current=\"%s\", offset=%ld, line=%d)\n",
         lexer->start, lexer->current, lexer->current - lexer->start,
         lexer->line);
}
void printTokenBoth(const Token* token, int toFile)
/* ────────────────────────────────────────────────────────── *
 *  toFile == 0 → print to stdout
 *  toFile != 0 → append to the file called "tokens"
 * ────────────────────────────────────────────────────────── */
{
  if (toFile) {
    /* 1.  no shadowing: parameter is now  `toFile`, variable is `filePointer`
     */
    /* 2.  use "ae" so the file is opened with O_CLOEXEC             */
    FILE* filePointer = fopen("tokens", "ae");
    if (filePointer == NULL) { /* safer than exit() in libs   */
      perror("fopen(\"tokens\")");
      return;
    }

    /* 3.  check fprintf ’s return value (CERT ERR33‑C)              */
    if (fprintf(filePointer, "Token(type=%s, lexeme=\"%.*s\", length=%d)\n",
                tokenTypeToString(token->type), token->length, token->lexeme,
                token->length) < 0) {
      perror("fprintf");
      /* fall through ‑ we still try to flush & close */
    }

    if (fclose(filePointer) != 0) { /* also check fclose           */
      perror("fclose");
    }
  } else {
    printf("Token(type=%s, lexeme=\"%.*s\", length=%d, line=%d)\n",
           tokenTypeToString(token->type), token->length, token->lexeme,
           token->length, token->line);
  }
}

void printToken(const Token* token) { printTokenBoth(token, 0); }
