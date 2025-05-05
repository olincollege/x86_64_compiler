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

/*
Exits a code with the error message

Args:
  error_msg: Error message to be displayed.
Returns:
  void
*/
void error_and_exit(const char* error_msg);

/*
Determines the token type of a given identifier string.

Used to distinguish keywords (like `if`, `return`) from general identifiers.

Args:
  text: Pointer to the identifier text.
  length: Length of the identifier string.

Returns:
  TokenType indicating the type of the identifier.
*/
TokenType identifierType(const char* text, int length);

/*
Scans the source and returns the next token.

Consumes characters from the source code and returns the next valid token,
advancing the lexer.

Args:
  lexer: Pointer to the Lexer object.

Returns:
  Token representing the next lexical unit in the input.
*/
Token getNextToken(Lexer* lexer);

/*
Initializes the lexer with source code.

Sets up internal pointers and line tracking for tokenization.

Args:
  lexer: Pointer to the Lexer to initialize.
  source: String containing the source code to tokenize.

Returns:
  void
*/
void initLexer(Lexer* lexer, const char* source);

/*
Converts a TokenType enum to its string name.

Useful for debugging or printing token types.

Args:
  type: The TokenType to convert.

Returns:
  A string representation of the token type.
*/
const char* tokenTypeToString(TokenType type);

/*
Prints information about the current lexer state.

Displays the lexer's source pointer, current pointer, line, and offset.

Args:
  lexer: Pointer to the Lexer object.

Returns:
  void
*/
void printLexer(const Lexer* lexer);

/*
Prints a token to stdout or appends it to a file.

If `file` is 0, prints to stdout. Otherwise, appends the token info to a file
named "tokens".

Args:
  token: Pointer to the token to print.
  file: 0 for stdout, nonzero for file output.

Returns:
  void
*/
void printTokenBoth(const Token* token, int file);

/*
Prints a token to stdout.

Convenience function to print a token to the terminal.

Args:
  token: Pointer to the token.

Returns:
  void
*/
void printToken(const Token* token);
