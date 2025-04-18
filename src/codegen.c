#include "codegen.h"

#include "lexer.h"
#include "parser.h"

const int MAX_LINE_LENGTH = 64;

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
  mem;

  mem->variableCapacity = 8;

  mem->variables = malloc(sizeof(variableInMemory*) * mem->variableCapacity);
  mem->numberOfVariables = 0;
  mem->nextStartingLocation = -4;  // Start at memory address 16 (2^4)
}

void addVariableToMemory(memory* mem, char* variableName) {
  variableInMemory* newVariable = malloc(sizeof(variableInMemory));
  newVariable->variableName = variableName;
  newVariable->memory_difference = mem->nextStartingLocation;
  printf("Adding variable %s to memory at location %d\n", variableName,
         newVariable->memory_difference);
  mem->nextStartingLocation -= 4;

  if (mem->numberOfVariables + 1 > mem->variableCapacity) {
    printf("Memory full! Increasing capacity to %d\n",
           mem->variableCapacity * 2);
    mem->variableCapacity *= 2;
    mem->variables = realloc(mem->variables,
                             sizeof(variableInMemory*) * mem->variableCapacity);
  }
  mem->variables[mem->numberOfVariables] = newVariable;
  mem->numberOfVariables++;

  // free(old_variable_location); Realloc frees so don't need this
}

int get_variable_memory_location(memory* mem, char* lexeme, int length) {
  printf("Searching for variable: '%.*s' (length = %d)\n", length, lexeme,
         length);

  for (int i = 0; i < mem->numberOfVariables; i++) {
    char* varName = mem->variables[i]->variableName;
    int varLen = strlen(varName);
    int cmpResult = strncmp(varName, lexeme, length);

    printf("  Checking [%d]: name = '%s', strlen = %d, strncmp = %d\n", i,
           varName, varLen, cmpResult);

    if (length == varLen && cmpResult == 0) {
      printf("  -> Match found! Returning memory offset: %d\n",
             mem->variables[i]->memory_difference);
      return mem->variables[i]->memory_difference;
    }
  }

  printf("  -> No match found.\n");
  return -1;  // NULL is a pointer; returning -1 is better for an int
}

char* get_variable_memory_location_with_pointer(memory* mem, char* lexeme,
                                                int length) {
  int offset = get_variable_memory_location(mem, lexeme, length);
  char* buffer = malloc(32);  // plenty of room for [rbp-<offset>]
  if (!buffer) {
    perror("malloc failed");
    exit(1);
  }
  if (offset > 0) {
    sprintf(buffer, "[rbp+%d]", offset);  // format the result into the buffer
  } else {
    sprintf(buffer, "[rbp%d]", offset);  // format the result into the buffer
  }
  return buffer;
}

void initListOfInstructions(listOfX86Instructions* list) {
  //   *list = malloc(sizeof(listOfX86Instructions));

  //   list = malloc(sizeof(listOfX86Instructions));
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

void ASTVariableLiteralOrBinaryToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem) {
  if (node->as.declaration.expression->type == AST_BINARY) {
    ASTBinaryNodeToX86(node->as.declaration.expression, list, mem);
  } else if (node->as.declaration.expression->type == AST_VARIABLE ||
             node->as.declaration.expression->type == AST_INT_LITERAL) {
    ASTVariableOrLiteralNodeToX86(node->as.declaration.expression, list, mem);
  }
}

void ASTVariableOrLiteralNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                   memory* mem) {
  if (node->type == AST_INT_LITERAL) {
    char* newInstruction = malloc(MAX_LINE_LENGTH);
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "mov     eax, %d", node->as.intLiteral.intLiteral);
    addInstruction(list, newInstruction);
  } else if (node->type == AST_VARIABLE) {
    char* operand = get_variable_memory_location_with_pointer(
        mem, node->as.variableName->lexeme, node->as.variableName->length);

    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "mov     eax, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  }
}

void ASTBinaryNodeToX86(ASTNode* node, listOfX86Instructions* list,
                        memory* mem) {
  int rightLiteralOrVariable = 1;
  if (node->as.binary.right->type == AST_INT_LITERAL) {
    ASTNode* rightNode = node->as.binary.right;
    char* newInstruction = malloc(MAX_LINE_LENGTH);
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

    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
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
    char* newInstruction = malloc(MAX_LINE_LENGTH);
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

    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "mov     eax, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  }

  if (rightLiteralOrVariable == 0) {
    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }

    sprintf(newInstruction, "%s     edx, eax",
            get_op_name(node->as.binary._operator));

    addInstruction(list, newInstruction);
  } else {
    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "%s     eax, edx",
            get_op_name(node->as.binary._operator));

    addInstruction(list, newInstruction);
  }
}

void ASTVariableDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem) {
  ;
  char* variableName = malloc(node->as.variable_declaration.name->length + 1);
  if (!variableName) {
    perror("malloc failed");
    exit(1);
  }
  strncpy(variableName, node->as.variable_declaration.name->lexeme,
          node->as.variable_declaration.name->length);
  variableName[node->as.variable_declaration.name->length] = '\0';
  addVariableToMemory(mem, variableName);
}

void ASTDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                             memory* mem) {
  printf("In ASTDeclarationNodeToX86 function\n");
  char* variableLocationString;
  if (node->as.declaration.variable->type == AST_VARIABLE_DECLARATION) {
    ASTVariableDeclarationNodeToX86(node->as.declaration.variable, list, mem);
    variableLocationString = get_variable_memory_location_with_pointer(
        mem,
        node->as.declaration.variable->as.variable_declaration.name->lexeme,
        node->as.declaration.variable->as.variable_declaration.name->length);
  } else if (node->as.declaration.variable->type == AST_VARIABLE) {
    variableLocationString = get_variable_memory_location_with_pointer(
        mem, node->as.declaration.variable->as.variableName->lexeme,
        node->as.declaration.variable->as.variableName->length);
  } else {
    printf("Error: Not a variable node\n");
    exit(1);
  }

  ASTVariableLiteralOrBinaryToX86(node, list, mem);

  printMemory(mem);
  char* newInstruction =
      malloc(MAX_LINE_LENGTH);  // enough for full instruction line
  if (!newInstruction) {
    perror("malloc failed");
    exit(1);
  }

  sprintf(newInstruction, "mov     DWORD PTR %s, eax", variableLocationString);
  free(variableLocationString);
  addInstruction(list, newInstruction);
}

void ASTStatementNodeToX86(ASTNode* node, listOfX86Instructions* list,
                           memory* mem) {
  switch (node->type) {
    case AST_DECLARATION:
      ASTDeclarationNodeToX86(node, list, mem);
      break;
    case AST_VARIABLE_DECLARATION:
      ASTVariableDeclarationNodeToX86(node, list, mem);
      break;
  }
}

void ASTBlockNodeToX86(ASTNode* node, listOfX86Instructions* list,
                       memory* mem) {
  printf("In blocknode%d\n");
  for (int i = 0; i < node->as.block.count; i++) {
    printf("Blocknode: %d\n", i);

    ASTStatementNodeToX86(node->as.block.statements[i], list, mem);
  }
}

void ASTFunctionNodeToX86(ASTNode* node, listOfX86Instructions* list) {
  if (node->type != AST_FUNCTION_DECLARATION) {
    printf("Error: Not a function node\n");
    exit(1);
  }
  memory* mem = malloc(sizeof(memory));
  initMemory(mem);
  if (strncmp(node->as.function.name, "main", strlen("main")) == 0) {
    char* newInstruction = "main:";
    addInstruction(list, newInstruction);
  } else {
    char* newInstruction = malloc(MAX_LINE_LENGTH);

    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }

    sprintf(newInstruction, "%.*s(", node->as.function.name->length,
            node->as.function.name->lexeme);

    for (int i = 0; i < node->as.function.paramCount; i++) {
      // TODO: (PRIORITY)  Deal with variables properly by putting them in the
      // stack using edi esi
      char* paramType =
          node->as.function.parameters[i]->as.variable_declaration.type->lexeme;
      int length =
          node->as.function.parameters[i]->as.variable_declaration.type->length;

      int currentNewInstructionLength = strlen(newInstruction);
      for (int j = 0; j < length; j++) {
        newInstruction[currentNewInstructionLength + j] = paramType[j];
      }
      if (i + 1 < node->as.function.paramCount) {
        newInstruction[currentNewInstructionLength + length] = ',';
        length++;
      }
      newInstruction[currentNewInstructionLength + length] = '\0';
    }
    int currentNewInstructionLength = strlen(newInstruction);

    newInstruction[currentNewInstructionLength] = ')';
    newInstruction[currentNewInstructionLength + 1] = ':';
    newInstruction[currentNewInstructionLength + 2] = '\0';
    // sprintf(newInstruction, "global %s\n%s:", node->as.function.name,
    //         node->as.function.name);

    addInstruction(list, newInstruction);
  }
  ASTBlockNodeToX86(node->as.function.statements, list, mem);

  return;
}

void printInstructions(listOfX86Instructions* list) {
  printf("X86 Instruction List (%d total):\n", list->instructionCount);
  for (int i = 0; i < list->instructionCount; i++) {
    printf("%s\n", list->instructions[i]);
  }
}

void printMemory(memory* mem) {
  printf("Memory Layout (%d variable(s)):\n", mem->numberOfVariables);
  for (int i = 0; i < mem->numberOfVariables; i++) {
    printf("  %s -> [rbp-%d]\n", mem->variables[i]->variableName,
           mem->variables[i]->memory_difference * -1);  // make offset positive
  }
}

/*


 char* variableName = malloc(node->as.variable_declaration.name->length + 1);
  if (!variableName) {
    perror("malloc failed");
    exit(1);
  }
  strncpy(variableName, node->as.variable_declaration.name->lexeme,
          node->as.variable_declaration.name->length);
  variableName[node->as.variable_declaration.name->length] = '\0';
  addVariableToMemory(mem, variableName);

*/
