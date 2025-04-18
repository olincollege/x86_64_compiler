#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef struct variableInMemory {
  char* variableName;
  int memory_difference;
} variableInMemory;

typedef struct memory {
  variableInMemory** variables;
  int variableCapacity;
  int numberOfVariables;
  int nextStartingLocation;
} memory;

typedef struct listOfX86Instructions {
  char** instructions;
  int instructionCount;
  int instructionCapacity;
} listOfX86Instructions;

typedef struct {
  char symbol;
  const char* name;
} OpMap;

OpMap constants[] = {
    {TOKEN_PLUS, "add"},
    {TOKEN_MINUS, "sub"},
    {TOKEN_STAR, "imul"},
    // {'/', "DIV"},
    {0, NULL}  // sentinel
};
