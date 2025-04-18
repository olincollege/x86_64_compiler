#include "codegen.h"

#include "lexer.h"
#include "parser.h"

#define DEBUG
// ASTNode constructors with added debug prints.

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
  fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

const int MAX_LINE_LENGTH = 64;

map opConstants[] = {
    {TOKEN_PLUS, "add"}, {TOKEN_MINUS, "sub"}, {TOKEN_STAR, "imul"}, {0, NULL}};

map lowLinuxRegisters[] = {
    {1, "edi"}, {2, "esi"}, {3, "edx"}, {4, "ecx"}, {5, "e8d"}, {6, "e9d"},
};

const char* get_op_name(TokenType op) {
  for (int i = 0; opConstants[i].name != NULL; i++) {
    if (opConstants[i].symbol == op) return opConstants[i].name;
  }
  return "UNKNOWN_OP";
}
const char* getLowLinuxRegistersName(int i) {
  return lowLinuxRegisters[i].name;
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
  DEBUG_PRINT("Adding variable %s to memory at location %d\n", variableName,
              newVariable->memory_difference);
  mem->nextStartingLocation -= 4;

  if (mem->numberOfVariables + 1 > mem->variableCapacity) {
    DEBUG_PRINT("Memory full! Increasing capacity to %d\n",
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
  DEBUG_PRINT("Searching for variable: '%.*s' (length = %d)\n", length, lexeme,
              length);

  for (int i = 0; i < mem->numberOfVariables; i++) {
    char* varName = mem->variables[i]->variableName;
    int varLen = strlen(varName);
    int cmpResult = strncmp(varName, lexeme, length);

    DEBUG_PRINT("  Checking [%d]: name = '%s', strlen = %d, strncmp = %d\n", i,
                varName, varLen, cmpResult);

    if (length == varLen && cmpResult == 0) {
      DEBUG_PRINT("  -> Match found! Returning memory offset: %d\n",
                  mem->variables[i]->memory_difference);
      return mem->variables[i]->memory_difference;
    }
  }

  DEBUG_PRINT("  -> No match found.\n");
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
  DEBUG_PRINT("In ASTVariableLiteralOrBinaryToX86\n");
  if (node->type == AST_BINARY) {
    ASTBinaryNodeToX86(node, list, mem);
  } else if (node->type == AST_VARIABLE || node->type == AST_INT_LITERAL) {
    ASTVariableOrLiteralNodeToX86(node, list, mem);
  }
}

void ASTVariableOrLiteralNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                   memory* mem) {
  DEBUG_PRINT("In ASTVariableOrLiteralNodeToX86\n");
  if (node->type == AST_INT_LITERAL) {
    DEBUG_PRINT("IS Int Literal");
    char* newInstruction = malloc(MAX_LINE_LENGTH);
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "        mov     eax, %d",
            node->as.intLiteral.intLiteral);
    addInstruction(list, newInstruction);
  } else if (node->type == AST_VARIABLE) {
    DEBUG_PRINT("IS Int Literaasdfasdfl");
    char* operand = get_variable_memory_location_with_pointer(
        mem, node->as.variableName->lexeme, node->as.variableName->length);

    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    sprintf(newInstruction, "        mov     eax, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  } else {
    fprintf(stderr, "ERROR: Unknown AST node type\n");
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
    sprintf(newInstruction, "        mov     edx, %d",
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
    sprintf(newInstruction, "        mov     edx, DWORD PTR %s", operand);
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
    sprintf(newInstruction, "        mov     eax, %d",
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
    sprintf(newInstruction, "        mov     eax, DWORD PTR %s", operand);
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

    sprintf(newInstruction, "        %s     edx, eax",
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
  DEBUG_PRINT("In ASTDeclarationNodeToX86 function\n");
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

  ASTVariableLiteralOrBinaryToX86(node->as.declaration.expression, list, mem);

  printMemory(mem);
  char* newInstruction =
      malloc(MAX_LINE_LENGTH);  // enough for full instruction line
  if (!newInstruction) {
    perror("malloc failed");
    exit(1);
  }

  sprintf(newInstruction, "        mov     DWORD PTR %s, eax",
          variableLocationString);
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
    case AST_FUNCTION_CALL:
      DEBUG_PRINT("In Function Call\n");
      ASTFunctionCallNodeToX86(node, list, mem);
      break;
  }
}

void ASTBlockNodeToX86(ASTNode* node, listOfX86Instructions* list,
                       memory* mem) {
  DEBUG_PRINT("In blocknode%d\n", node->as.block.count);
  for (int i = 0; i < node->as.block.count; i++) {
    DEBUG_PRINT("Blocknode: %d\n", i);

    ASTStatementNodeToX86(node->as.block.statements[i], list, mem);
  }
}

void ASTFunctionCallNodeToX86(ASTNode* node, listOfX86Instructions* list,
                              memory* mem) {
  for (int i = 0; i < node->as.function_call.paramCount; i++) {
    DEBUG_PRINT("1\n");
    if (node->as.function_call.parameters[i]->type == AST_INT_LITERAL) {
      DEBUG_PRINT(
          "%d", node->as.function_call.parameters[i]->as.intLiteral.intLiteral);
    }
    ASTVariableLiteralOrBinaryToX86(node->as.function_call.parameters[i], list,
                                    mem);
    DEBUG_PRINT("1\n");
    char* newInstruction = malloc(MAX_LINE_LENGTH);
    if (!newInstruction) {
      perror("malloc failed");
      exit(1);
    }
    printf("2n\n");
    sprintf(newInstruction, "        mov     %s, eax",
            getLowLinuxRegistersName(i));
    printf("2n\n");
    addInstruction(list, newInstruction);
  }
  DEBUG_PRINT("1\n");

  // Below is the function call
  char* newInstruction = malloc(MAX_LINE_LENGTH);

  sprintf(newInstruction, "        call    %.*s(",
          node->as.function_call.name->length,
          node->as.function_call.name->lexeme);

  for (int i = 0; i < node->as.function_call.paramCount; i++) {
    // TODO: (PRIORITY)  Deal with variables properly by putting them in the
    // stack using edi esi
    char* paramType = "int";
    // node->as.function_call.parameters[i]->as.variable_declaration.type->lexeme;
    int length = 3;
    // node->as.function_call.parameters[i]->as.variable_declaration.type->length;

    int currentNewInstructionLength = strlen(newInstruction);
    for (int j = 0; j < length; j++) {
      newInstruction[currentNewInstructionLength + j] = paramType[j];
    }
    if (i + 1 < node->as.function_call.paramCount) {
      newInstruction[currentNewInstructionLength + length] = ',';
      length++;
    }
    newInstruction[currentNewInstructionLength + length] = '\0';
  }
  int currentNewInstructionLength = strlen(newInstruction);

  newInstruction[currentNewInstructionLength] = ')';
  newInstruction[currentNewInstructionLength + 2] = '\0';
  addInstruction(list, newInstruction);
}

void ASTFunctionNodeToX86(ASTNode* node, listOfX86Instructions* list) {
  DEBUG_PRINT("In function node %.*s\n", node->as.function.name->length,
              node->as.function.name->lexeme);
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
  char* newInstruction = "        push    rbp";
  addInstruction(list, newInstruction);
  newInstruction = "        mov     rbp, rsp";
  addInstruction(list, newInstruction);

  for (int i = 0; i < node->as.function.paramCount; i++) {
    char* variableName = malloc(
        node->as.function.parameters[i]->as.variable_declaration.name->length +
        1);
    if (!variableName) {
      perror("malloc failed");
      exit(1);
    }
    strncpy(
        variableName,
        node->as.function.parameters[i]->as.variable_declaration.name->lexeme,
        node->as.function.parameters[i]->as.variable_declaration.name->length);
    variableName[node->as.function.parameters[i]
                     ->as.variable_declaration.name->length] = '\0';
    addVariableToMemory(mem, variableName);
    char* var_loc_with_pointer = get_variable_memory_location_with_pointer(
        mem,
        node->as.function.parameters[i]->as.variable_declaration.name->lexeme,
        node->as.function.parameters[i]->as.variable_declaration.name->length);
    newInstruction = malloc(MAX_LINE_LENGTH);
    sprintf(newInstruction, "        mov     DWORD PTR %s, %s",
            var_loc_with_pointer, getLowLinuxRegistersName(i));
    addInstruction(list, newInstruction);
  }

  ASTBlockNodeToX86(node->as.function.statements, list, mem);

  newInstruction = "        pop     rbp";
  addInstruction(list, newInstruction);
  newInstruction = "        ret";
  addInstruction(list, newInstruction);
  return;
}

void ListOfASTFunctionNodesToX86(ASTNode** nodes, listOfX86Instructions* list,
                                 int numberOfFunctions) {
  DEBUG_PRINT("Going through %d functions.\n", numberOfFunctions);
  for (int i = 0; i < numberOfFunctions; ++i) {
    if (nodes[i] != NULL) {
      ASTFunctionNodeToX86(nodes[i], list);
    }
  }
}

void printInstructions(listOfX86Instructions* list) {
  DEBUG_PRINT("X86 Instruction List (%d total):\n", list->instructionCount);
  for (int i = 0; i < list->instructionCount; i++) {
    printf("%s\n", list->instructions[i]);
  }
}

void printMemory(memory* mem) {
  DEBUG_PRINT("Memory Layout (%d variable(s)):\n", mem->numberOfVariables);
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
