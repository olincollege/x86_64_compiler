
#include "codegen.h"

#include <stdlib.h>  // for free()
#include <string.h>

#include "lexer.h"
#include "parser.h"

// ASTNode constructors with added debug prints.

// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
  fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

const size_t MAX_LINE_LENGTH = 64;
const int INITIAL_MEMORY_CAPACITY = 8;
const int BUFFER_SIZE = 32;
const map opConstants[] = {
    {TOKEN_PLUS, "add"},
    {TOKEN_MINUS, "sub"},
    {TOKEN_STAR, "imul"},
    {TOKEN_SLASH, "idiv"},  // ← added this line for division
    {0, NULL}};

const map lowLinuxRegisters[] = {
    {1, "edi"}, {2, "esi"}, {3, "edx"}, {4, "ecx"}, {5, "e8d"}, {6, "e9d"},
};

const char* get_op_name(TokenType operator) {
  for (int i = 0; opConstants[i].name != NULL; i++) {
    if (opConstants[i].symbol == operator) {
      return opConstants[i].name;
    }
  }
  return "UNKNOWN_OP";
}
const char* getLowLinuxRegistersName(int index) {
  return lowLinuxRegisters[index].name;
}

void initMemory(memory* mem) {
  mem->variableCapacity = INITIAL_MEMORY_CAPACITY;

  mem->variables = (variableInMemory**)malloc(
      sizeof(variableInMemory*) * (unsigned long)mem->variableCapacity);
  mem->numberOfVariables = 0;
  mem->nextStartingLocation = -4;  // Start at memory address 16 (2^4)
}

void addVariableToMemory(memory* mem, char* variableName) {
  variableInMemory* newVariable =
      (variableInMemory*)malloc(sizeof(variableInMemory));
  newVariable->variableName = variableName;
  newVariable->memory_difference = mem->nextStartingLocation;
  DEBUG_PRINT("Adding variable %s to memory at location %d\n", variableName,
              newVariable->memory_difference);
  mem->nextStartingLocation -= 4;

  if (mem->numberOfVariables + 1 > mem->variableCapacity) {
    DEBUG_PRINT("Memory full! Increasing capacity to %d\n",
                mem->variableCapacity * 2);
    mem->variableCapacity *= 2;
    variableInMemory** newLoc = (variableInMemory**)realloc(
        (void*)mem->variables,
        sizeof(variableInMemory*) * (long unsigned int)mem->variableCapacity);
    if (newLoc == NULL) {
      printf("Error reallocating memory\n");
    }
    mem->variables = newLoc;
  }
  mem->variables[mem->numberOfVariables] = newVariable;
  mem->numberOfVariables++;

  // free(old_variable_location); Realloc frees so don't need this
}

int get_variable_memory_location(memory* mem, const char* lexeme, int length) {
  DEBUG_PRINT("Searching for variable: '%.*s' (length = %d)\n", length, lexeme,
              length);

  for (int i = 0; i < mem->numberOfVariables; i++) {
    char* varName = mem->variables[i]->variableName;
    int varLen = (int)strlen(varName);
    int cmpResult = strncmp(varName, lexeme, (unsigned long)length);

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

char* get_variable_memory_location_with_pointer(memory* mem, const char* lexeme,
                                                int length) {
  int offset = get_variable_memory_location(mem, lexeme, length);
  char* buffer =
      malloc((size_t)BUFFER_SIZE);  // plenty of room for [rbp-<offset>]
  if (!buffer) {
    error_and_exit("malloc failed");
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
  list->instructions = (char**)malloc(
      sizeof(char*) * (long unsigned int)list->instructionCapacity);
}

void addInstruction(listOfX86Instructions* list, char* instruction) {
  if (list->instructionCount == list->instructionCapacity) {
    list->instructionCapacity *= 2;
    char** newInstructionLoc = (char**)realloc(
        (void*)list->instructions,
        sizeof(char*) * (long unsigned int)list->instructionCapacity);
    if (newInstructionLoc == NULL) {
      error_and_exit("realloc failed");
    }
    list->instructions = newInstructionLoc;
  }
  if (list->instructions == NULL) {
    error_and_exit("list is NULL");
  }
  list->instructions[list->instructionCount] = instruction;
  list->instructionCount++;
}

void ASTVariableLiteralOrBinaryToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem) {
  DEBUG_PRINT("In ASTVariableLiteralOrBinaryToX86\n");
  if (node->type == AST_BINARY) {
    ASTBinaryNodeToX86(node, list, mem, 1);
  } else if (node->type == AST_VARIABLE || node->type == AST_INT_LITERAL) {
    ASTVariableOrLiteralNodeToX86(node, list, mem);
  } else if (node->type == AST_FUNCTION_CALL) {
    ASTFunctionCallNodeToX86(node, list, mem);
  } else {
    return;
  }
}

void ASTVariableOrLiteralNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                   memory* mem) {
  DEBUG_PRINT("In ASTVariableOrLiteralNodeToX86\n");
  if (node->type == AST_INT_LITERAL) {
    DEBUG_PRINT("IS Int Literal");
    char* newInstruction = malloc(MAX_LINE_LENGTH);
    if (!newInstruction) {
      error_and_exit("malloc failed");
    }
    sprintf(newInstruction, "        mov     eax, %d",
            node->as.intLiteral.intLiteral);
    addInstruction(list, newInstruction);
  } else if (node->type == AST_VARIABLE) {
    char* operand = get_variable_memory_location_with_pointer(
        mem, node->as.variableName->lexeme, node->as.variableName->length);

    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      error_and_exit("malloc failed");
    }
    sprintf(newInstruction, "        mov     eax, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  } else {
    fprintf(stderr, "ERROR: Unknown AST node type\n");
  }
}

void ASTBinaryNodeToX86(ASTNode* node, listOfX86Instructions* list, memory* mem,
                        int first) {
  DEBUG_PRINT("ASTBinaryNodeToX86");
  if (node->as.binary.right->type == AST_INT_LITERAL) {
    ASTNode* rightNode = node->as.binary.right;
    char* newInstruction = malloc(MAX_LINE_LENGTH);
    if (!newInstruction) {
      error_and_exit("malloc failed");
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
      error_and_exit("malloc failed");
    }
    sprintf(newInstruction, "        mov     edx, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  } else {
    // Add in what else to do if not literal
    ASTBinaryNodeToX86(node->as.binary.right, list, mem, 1);
  }
  if (node->as.binary.left->type == AST_INT_LITERAL) {
    ASTNode* leftNode = node->as.binary.left;
    char* newInstruction = malloc(MAX_LINE_LENGTH);
    if (!newInstruction) {
      error_and_exit("malloc failed");
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
      error_and_exit("malloc failed");
    }
    sprintf(newInstruction, "        mov     eax, DWORD PTR %s", operand);
    free(operand);  // don't forget to free the operand string
    addInstruction(list, newInstruction);
  }

  if (first == 0) {
    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      error_and_exit("malloc failed");
    }

    sprintf(newInstruction, "        %s     edx, eax",
            get_op_name(node->as.binary._operator));

    addInstruction(list, newInstruction);
  } else {
    char* newInstruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!newInstruction) {
      error_and_exit("malloc failed");
    }
    sprintf(newInstruction, "        %s     eax, edx",
            get_op_name(node->as.binary._operator));

    addInstruction(list, newInstruction);
  }
}

void ASTVariableDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                                     memory* mem) {
  char* variableName =
      malloc((unsigned long)node->as.variable_declaration.name->length +
             (unsigned long)1);
  if (!variableName) {
    error_and_exit("malloc failed");
  }
  strncpy(variableName, node->as.variable_declaration.name->lexeme,
          (size_t)node->as.variable_declaration.name->length);
  variableName[node->as.variable_declaration.name->length] = '\0';
  addVariableToMemory(mem, variableName);
}

void ASTDeclarationNodeToX86(ASTNode* node, listOfX86Instructions* list,
                             memory* mem) {
  DEBUG_PRINT("In ASTDeclarationNodeToX86 function\n");
  char* variableLocationString = NULL;
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
    error_and_exit("Error: Not a variable node\n");
  }

  ASTVariableLiteralOrBinaryToX86(node->as.declaration.expression, list, mem);

  printMemory(mem);
  char* newInstruction =
      malloc(MAX_LINE_LENGTH);  // enough for full instruction line
  if (!newInstruction) {
    error_and_exit("malloc failed");
  }

  sprintf(newInstruction, "        mov     DWORD PTR %s, eax",
          variableLocationString);
  free(variableLocationString);
  addInstruction(list, newInstruction);
}

void ASTReturnNodeToX86(ASTNode* node, listOfX86Instructions* list,
                        memory* mem) {
  DEBUG_PRINT("In Return Node\n");

  ASTStatementNodeToX86(node->as._return.expression, list, mem);
  char* newInstruction = NULL;  //= malloc(MAX_LINE_LENGTH);

  newInstruction = "        pop     rbp";
  addInstruction(list, newInstruction);
  newInstruction = "        ret";
  addInstruction(list, newInstruction);
}

void ASTStatementNodeToX86(ASTNode* node, listOfX86Instructions* list,
                           memory* mem) {
  DEBUG_PRINT("In Statement Node\n");
  if (node == NULL) {
    DEBUG_PRINT("NULL NODE\n");
    return;
  }
  switch (node->type) {
    case AST_VARIABLE:
      DEBUG_PRINT("In Variable Node\n");
      ASTVariableOrLiteralNodeToX86(node, list, mem);
      break;
    case AST_INT_LITERAL:
      DEBUG_PRINT("In Int Literal Node\n");
      ASTVariableOrLiteralNodeToX86(node, list, mem);
      break;
    case AST_DECLARATION:
      DEBUG_PRINT("In Declaration Node\n");
      ASTDeclarationNodeToX86(node, list, mem);
      break;
    case AST_VARIABLE_DECLARATION:
      DEBUG_PRINT("In Variable Declaration Node\n");
      ASTVariableDeclarationNodeToX86(node, list, mem);
      break;
    case AST_FUNCTION_CALL:
      DEBUG_PRINT("In Function Call\n");
      ASTFunctionCallNodeToX86(node, list, mem);
      break;
    case AST_RETURN:
      DEBUG_PRINT("In Return Statement\n");
      ASTReturnNodeToX86(node, list, mem);
      break;
    default:
      DEBUG_PRINT("In default case\n");
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
  DEBUG_PRINT("In Function Call\n");
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
      error_and_exit("malloc failed");
    }
    sprintf(newInstruction, "        mov     %s, eax",
            getLowLinuxRegistersName(i));
    addInstruction(list, newInstruction);
  }
  DEBUG_PRINT("1\n");

  // Below is the function call
  char* newInstruction = malloc(MAX_LINE_LENGTH);

  sprintf(newInstruction, "        call    %.*s",
          node->as.function_call.name->length,
          node->as.function_call.name->lexeme);

  //   for (int i = 0; i < node->as.function_call.paramCount; i++) {
  //     // TODO: (PRIORITY)  Deal with variables properly by putting them in
  //     the
  //     // stack using edi esi
  //     char* paramType = "int";
  //     //
  //     node->as.function_call.parameters[i]->as.variable_declaration.type->lexeme;
  //     int length = 3;
  //     //
  //     node->as.function_call.parameters[i]->as.variable_declaration.type->length;

  //     int currentNewInstructionLength = strlen(newInstruction);
  //     for (int j = 0; j < length; j++) {
  //       newInstruction[currentNewInstructionLength + j] = paramType[j];
  //     }
  //     if (i + 1 < node->as.function_call.paramCount) {
  //       newInstruction[currentNewInstructionLength + length] = ',';
  //       length++;
  //     }
  //     newInstruction[currentNewInstructionLength + length] = '\0';
  //   }
  //   int currentNewInstructionLength = strlen(newInstruction);

  //   newInstruction[currentNewInstructionLength] = ')';
  //   newInstruction[currentNewInstructionLength + 2] = '\0';
  addInstruction(list, newInstruction);
}

void ASTFunctionNodeToX86(ASTNode* node, listOfX86Instructions* list) {
  DEBUG_PRINT("In function node %.*s\n", node->as.function.name->length,
              node->as.function.name->lexeme);
  if (node->type != AST_FUNCTION_DECLARATION) {
    error_and_exit("Error: Not a function node\n");
  }
  memory* mem = malloc(sizeof(memory));
  initMemory(mem);
  if (strncmp(node->as.function.name->lexeme, "main", strlen("main")) == 0) {
    char* newInstruction = "main:";
    addInstruction(list, newInstruction);
  } else {
    char* newInstruction = malloc(MAX_LINE_LENGTH);

    if (!newInstruction) {
      error_and_exit("malloc failed");
    }

    sprintf(newInstruction, "%.*s:", node->as.function.name->length,
            node->as.function.name->lexeme);

    // for (int i = 0; i < node->as.function.paramCount; i++) {
    //   // TODO: (PRIORITY)  Deal with variables properly by putting them in
    //   the
    //   // stack using edi esi
    //   char* paramType =
    //       node->as.function.parameters[i]->as.variable_declaration.type->lexeme;
    //   int length =
    //       node->as.function.parameters[i]->as.variable_declaration.type->length;

    //   int currentNewInstructionLength = strlen(newInstruction);
    //   for (int j = 0; j < length; j++) {
    //     newInstruction[currentNewInstructionLength + j] = paramType[j];
    //   }
    //   if (i + 1 < node->as.function.paramCount) {
    //     newInstruction[currentNewInstructionLength + length] = ',';
    //     length++;
    //   }
    //   newInstruction[currentNewInstructionLength + length] = '\0';
    // }
    // int currentNewInstructionLength = strlen(newInstruction);

    // newInstruction[currentNewInstructionLength] = ')';
    // newInstruction[currentNewInstructionLength + 1] = ':';
    // newInstruction[currentNewInstructionLength + 2] = '\0';
    // // sprintf(newInstruction, "global %s\n%s:", node->as.function.name,
    // //         node->as.function.name);

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
      error_and_exit("malloc failed");
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
}

void ListOfASTFunctionNodesToX86(ASTNode** nodes, listOfX86Instructions* list,
                                 int numberOfFunctions) {
  DEBUG_PRINT("Going through %d functions.\n", numberOfFunctions);
  char* newInstruction = ".intel_syntax noprefix";
  addInstruction(list, newInstruction);
  newInstruction = ".global _start";
  addInstruction(list, newInstruction);
  newInstruction = ".text";
  addInstruction(list, newInstruction);
  newInstruction = "_start:";
  addInstruction(list, newInstruction);
  newInstruction = "    call main";
  addInstruction(list, newInstruction);
  newInstruction = "    mov rdi, rax       # syscall: exit";
  addInstruction(list, newInstruction);
  newInstruction = "    mov rax, 60        # exit code 0";
  addInstruction(list, newInstruction);
  newInstruction = "    syscall";
  addInstruction(list, newInstruction);

  for (int i = 0; i < numberOfFunctions; ++i) {
    if (nodes[i] != NULL) {
      ASTFunctionNodeToX86(nodes[i], list);
    }
  }
}

#include <stdio.h>

void printInstructions(listOfX86Instructions* list) {
  FILE* file = fopen("chat.s", "we");  // Overwrite/clear the file
  if (!file) {
    perror("Failed to open chat.s for writing");
    return;
  }

  // Add the instructions
  for (int i = 0; i < list->instructionCount; i++) {
    fprintf(file, "%s\n", list->instructions[i]);
  }

  (void)fclose(file);
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
    error_and_exit("malloc failed");
  }
  strncpy(variableName, node->as.variable_declaration.name->lexeme,
          node->as.variable_declaration.name->length);
  variableName[node->as.variable_declaration.name->length] = '\0';
  addVariableToMemory(mem, variableName);

*/
