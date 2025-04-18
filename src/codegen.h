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

void initListOfInstructions(listOfX86Instructions* list);

void initMemory(memory* mem);
void ASTBinaryNodeToX86(ASTNode* node, listOfX86Instructions* list,
                        memory* mem);
void printInstructions(listOfX86Instructions* list);
const char* get_op_name(TokenType op);
