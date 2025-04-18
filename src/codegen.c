#include "codegen.h"

#include "lexer.h"
#include "parser.h"

void initMemory(memory* mem) {
  mem->variableCapacity = 8;
  mem->variables = malloc(sizeof(variableInMemory*) * mem->variableCapacity);
  mem->numberOfVariables = 0;
  mem->nextStartingLocation = -4;  // Start at memory address 16 (2^4)
}

void addVariableToMemory(memory* mem, char* variableName) {
  variableInMemory* newVariable = malloc(sizeof(variableInMemory));
  newVariable->variableName = variableName;
  newVariable->memory_difference = mem->nextStartingLocation;

  if (mem->numberOfVariables + 1 > mem->variableCapacity) {
    mem->variableCapacity *= 2;
    variableInMemory** old_variable_location = mem->variables;
    mem->variables = realloc(mem->variables,
                             sizeof(variableInMemory*) * mem->variableCapacity);

    mem->variables[mem->numberOfVariables] = newVariable;
    mem->numberOfVariables++;

    // free(old_variable_location); Realloc frees so don't need this
  }
}

int get_variable_memory_location(memory* mem, char* variableName) {
  for (int i = 0; i < mem->numberOfVariables; i++) {
    if (len(variableName) == len(mem->variables[i]->variableName) ==
        strcmp(mem->variables[i]->variableName, variableName) == 0)
      return mem->variables[i]->memory_difference;
  }
  return NULL;
}

char** ASTFunctionNodeToX86(ASTNode* node) {
  if (node->type != AST_FUNCTION_DECLARATION) {
    printf("Error: Not a function node\n");
    exit(1);
  }
}
