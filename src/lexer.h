#pragma once

typedef enum {
  TOKEN_EOF,          // end of file
  TOKEN_INT_LITERAL,  // Integer literal
  TOKEN_IDENTIFIER,   // Identifier names
  // Keywords:
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FOR,
  TOKEN_RETURN,
  TOKEN_INT_TYPE,
  TOKEN_VOID_TYPE,
  // Operators:
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_PERCENT,
  TOKEN_ASSIGN,
  TOKEN_EQ,
  TOKEN_NEQ,
  TOKEN_LT,
  TOKEN_GT,
  TOKEN_LEQ,
  TOKEN_GEQ,
  // Delimiters:
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_UNKNOWN  // errors
} TokenType;

typedef struct {
  TokenType type;
  const char* lexeme;  // pointer to start of token
  int length;
  int line;
} Token;

typedef struct {
  const char* start;
  const char* current;
  int line;
} Lexer;

TokenType identifierType(const char* text, int length);
Token getNextToken(Lexer* lexer);
void initLexer(Lexer* lexer, const char* source);
const char* tokenTypeToString(TokenType type);
void printLexer(const Lexer* lexer);
void printTokenBoth(const Token* token, int file);
void printToken(const Token* token);
