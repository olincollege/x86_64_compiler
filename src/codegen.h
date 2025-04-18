#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef struct variableInMemory {
  char* variableName;
  int memory_difference;
  int variable_type;
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
} map;

// ───── Memory and Instruction Management ─────
void initMemory(memory* mem);
void addVariableToMemory(memory* mem, char* variableName);
int get_variable_memory_location(memory* mem, const char* lexeme, int length);
char* get_variable_memory_location_with_pointer(memory* mem, const char* lexeme,
                                                int length);
void initListOfInstructions(listOfX86Instructions* list);
void addInstruction(listOfX86Instructions* list, char* instruction);
void printMemory(memory* mem);

// ───── Instruction Generation ─────
void ASTVariableLiteralOrBinaryToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem);
void ASTVariableOrLiteralNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                   memory* mem);
void ASTBinaryNodeToX86(ASTNode* node, listOfX86Instructions* list, memory* mem,
                        int first);
void ASTVariableDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem);
void ASTDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                             memory* mem);

void ASTVariableDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem);
void printMemory(memory* mem);
void ASTVariableOrLiteralNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                   memory* mem);
void ListOfASTFunctionNodesToX86(ASTNode** nodes, listOfX86Instructions* list,
                                 int numberOfFunctions);
