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
