#include "codegen.h"

#include "lexer.h"
#include "parser.h"

OpMap opConstants[] = {
    {TOKEN_PLUS, "add"},
    {TOKEN_MINUS, "sub"},
    {TOKEN_STAR, "imul"},
    // {'/', "DIV"},
    {0, NULL}  // sentinel
};

const char* get_op_name(TokenType op) {
  for (int i = 0; opConstants[i].name != NULL; i++) {
    if (opConstants[i].symbol == op) return opConstants[i].name;
  }
  return "UNKNOWN_OP";
}

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

int get_variable_memory_location(memory* mem, char* lexeme, int length) {
  for (int i = 0; i < mem->numberOfVariables; i++) {
    if (length == strlen(mem->variables[i]->variableName) ==
        strncmp(mem->variables[i]->variableName, lexeme, length) == 0)
      return mem->variables[i]->memory_difference;
  }
  return NULL;
}

char* get_variable_memory_location_with_pointer(memory* mem, char* lexeme,
                                                int length) {
  int offset = get_variable_memory_location(mem, lexeme, length);
  char* buffer = malloc(32);  // plenty of room for [rbp-<offset>]
  if (!buffer) {
    perror("malloc failed");
    exit(1);
  }
  sprintf(buffer, "[rbp-%d]", offset);  // format the result into the buffer
  return buffer;
}

void initListOfInstructions(listOfX86Instructions* list) {
  list->instructionCapacity = 2;
  list->instructionCount = 0;
  list->instructions = malloc(sizeof(char*) * list->instructionCapacity);
}

void addInstruction(listOfX86Instructions* list, char* instruction) {
  if (list->instructionCount == list->instructionCapacity) {
    list->instructionCapacity *= 2;
    list->instructions =
        realloc(list->instructions, sizeof(char*) * list->instructionCapacity);
  }
  list->instructions[list->instructionCount] = instruction;
  list->instructionCount++;
}

void ASTBinaryNodeToX86(ASTNode* node, listOfX86Instructions* list,
                        memory* mem) {
  //   DEBUG_PRINT("ASTBinaryNodeToX86");
  printf("Here");
  int rightLiteralOrVariable = 1;
  if (node->as.binary.right->type == AST_INT_LITERAL) {
    ASTNode* rightNode = node->as.binary.right;
    char* newInstruction = malloc(64);
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "mov     edx, %d",
            rightNode->as.intLiteral.intLiteral);
    addInstruction(list, newInstruction);
  } else if (node->as.binary.right->type == AST_VARIABLE) {
    ASTNode* rightNode = node->as.binary.right;
    char* operand = get_variable_memory_location_with_pointer(
        mem, rightNode->as.variableName->lexeme,
        rightNode->as.variableName->length);

    char* newInstruction = malloc(64);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "mov     edx, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  } else {
    rightLiteralOrVariable = 0;
    // Add in what else to do if not literal
    ASTBinaryNodeToX86(node->as.binary.right, list, mem);
  }
  if (node->as.binary.left->type == AST_INT_LITERAL) {
    ASTNode* leftNode = node->as.binary.left;
    char* newInstruction = malloc(64);
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "mov     eax, %d",
            leftNode->as.intLiteral.intLiteral);
    addInstruction(list, newInstruction);

  } else if (node->as.binary.left->type == AST_VARIABLE) {
    ASTNode* leftNode = node->as.binary.left;
    char* operand = get_variable_memory_location_with_pointer(
        mem, leftNode->as.variableName->lexeme,
        leftNode->as.variableName->length);

    char* newInstruction = malloc(64);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "mov     eax, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  }

  if (rightLiteralOrVariable == 0) {
    char* newInstruction = malloc(64);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }

    sprintf(newInstruction, "%s     edx, eax",
            get_op_name(node->as.binary._operator));

    addInstruction(list, newInstruction);
  } else {
    char* newInstruction = malloc(64);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "%s     eax, edx",
            get_op_name(node->as.binary._operator));

    addInstruction(list, newInstruction);
  }
}

char** ASTFunctionNodeToX86(ASTNode* node) {
  if (node->type != AST_FUNCTION_DECLARATION) {
    printf("Error: Not a function node\n");
    exit(1);
  }
}

void printInstructions(listOfX86Instructions* list) {
  printf("X86 Instruction List (%d total):\n", list->instructionCount);
  for (int i = 0; i < list->instructionCount; i++) {
    printf("%s\n", list->instructions[i]);
  }
}
