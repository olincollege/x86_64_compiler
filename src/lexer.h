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

/* Initializes the lexer with the given source code */
void initLexer(Lexer* lexer, const char* source);

/* Returns the next token from the source code */
Token getNextToken(Lexer* lexer);

/* Determines if an identifier is a keyword; if so, returns the appropriate
 * TokenType */
TokenType identifierType(const char* text, int length);

// set of assembly that wer'e gong to rno on device that's been compiled

// lots of testing

// set up in cmake

// Helper functions to print tokens and lexer state
const char* tokenTypeToString(TokenType type);

void printLexer(const Lexer* lexer);
void printToken(const Token* token);
void printTokenBoth(const Token* token, int file);
