
#include "codegen.h"

#include <stdlib.h>  // for free()
#include <string.h>

#include "lexer.h"
#include "parser.h"

// ast_node constructors with added debug prints.

// #define DEBUG
#ifdef DEBUG
/**
 * debug_print – Helper to emit a debug message with file, function, and line.
 * @func:   __func__ of the caller.
 * @line:   __LINE__ of the caller.
 * @fmt:    printf-style format string.
 * @...:    Arguments for fmt.
 */
static inline void debug_print(const char* func, int line, const char* fmt,
                               ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[DEBUG] %s:%d: ", func, line);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}
#define DEBUG_PRINT(fmt, ...) \
  debug_print(__func__, __LINE__, (fmt), ##__VA_ARGS__)
#else
/* In release builds this becomes a no‑op with zero overhead */
static inline void debug_print(const char* func, int line, const char* fmt,
                               ...) {
  (void)func;
  (void)line;
  (void)fmt;
}
#define DEBUG_PRINT(fmt, ...) ((void)0)
#endif

const size_t MAX_LINE_LENGTH = 64;
const int INITIAL_MEMORY_CAPACITY = 8;
const int BUFFER_SIZE = 32;
const map op_constants[] = {{TOKEN_PLUS, "add"},
                            {TOKEN_MINUS, "sub"},
                            {TOKEN_STAR, "imul"},
                            {TOKEN_SLASH, "idiv"},
                            {0, NULL}};

const map low_linux_registers[] = {
    {1, "edi"}, {2, "esi"}, {3, "edx"}, {4, "ecx"}, {5, "e8d"}, {6, "e9d"},
};

const char* get_op_name(TokenType operator) {
  for (int i = 0; op_constants[i].name != NULL; i++) {
    if (op_constants[i].symbol == operator) {
      return op_constants[i].name;
    }
  }
  return "UNKNOWN_OP";
}
const char* get_low_linux_registers_name(int index) {
  return low_linux_registers[index].name;
}

void init_memory(memory* mem) {
  mem->variable_capacity = INITIAL_MEMORY_CAPACITY;

  mem->variables = (variable_in_memory**)malloc(
      sizeof(variable_in_memory*) * (unsigned long)mem->variable_capacity);
  mem->number_of_variables = 0;
  mem->next_starting_location = -4;  // Start at memory address 16 (2^4)
}

void add_variable_to_memory(memory* mem, char* variable_name) {
  variable_in_memory* new_variable =
      (variable_in_memory*)malloc(sizeof(variable_in_memory));
  new_variable->variable_name = variable_name;
  new_variable->memory_difference = mem->next_starting_location;
  DEBUG_PRINT("Adding variable %s to memory at location %d\n", variable_name,
              new_variable->memory_difference);
  mem->next_starting_location -= 4;

  if (mem->number_of_variables + 1 > mem->variable_capacity) {
    DEBUG_PRINT("Memory full! Increasing capacity to %d\n",
                mem->variable_capacity * 2);
    mem->variable_capacity *= 2;
    variable_in_memory** new_variable_in_memory_location =
        (variable_in_memory**)realloc(
            (void*)mem->variables,
            sizeof(variable_in_memory*) *
                (long unsigned int)mem->variable_capacity);
    if (new_variable_in_memory_location == NULL) {
      printf("Error reallocating memory\n");
    }
    mem->variables = new_variable_in_memory_location;
  }
  mem->variables[mem->number_of_variables] = new_variable;
  mem->number_of_variables++;

  // free(old_variable_location); Realloc frees so don't need this
}

int get_variable_memory_location(memory* mem, const char* lexeme, int length) {
  DEBUG_PRINT("Searching for variable: '%.*s' (length = %d)\n", length, lexeme,
              length);

  for (int i = 0; i < mem->number_of_variables; i++) {
    char* variable_name = mem->variables[i]->variable_name;
    int variable_length = (int)strlen(variable_name);
    int compare_result = strncmp(variable_name, lexeme, (unsigned long)length);

    DEBUG_PRINT("  Checking [%d]: name = '%s', strlen = %d, strncmp = %d\n", i,
                variable_name, variable_length, compare_result);

    if (length == variable_length && compare_result == 0) {
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
    (void)sprintf(buffer, "[rbp+%d]",
                  offset);  // format the result into the buffer
  } else {
    (void)sprintf(buffer, "[rbp%d]",
                  offset);  // format the result into the buffer
  }
  return buffer;
}

void init_list_of_instructions(list_of_x86_instructions* list) {
  //   *list = malloc(sizeof(list_of_x86_instructions));

  //   list = malloc(sizeof(list_of_x86_instructions));
  list->instruction_capacity = 2;
  list->instruction_count = 0;
  list->instructions = (char**)malloc(
      sizeof(char*) * (long unsigned int)list->instruction_capacity);
}

void add_instruction(list_of_x86_instructions* list, char* instruction) {
  if (list->instruction_count == list->instruction_capacity) {
    list->instruction_capacity *= 2;
    char** new_instruction_location = (char**)realloc(
        (void*)list->instructions,
        sizeof(char*) * (long unsigned int)list->instruction_capacity);
    if (new_instruction_location == NULL) {
      error_and_exit("realloc failed");
    }
    list->instructions = new_instruction_location;
  }
  if (list->instructions == NULL) {
    error_and_exit("list is NULL");
    return;
  }
  list->instructions[list->instruction_count] = instruction;
  list->instruction_count++;
}

void ast_variable_literal_or_binary_to_x86(ast_node* node,
                                           list_of_x86_instructions* list,
                                           memory* mem) {
  DEBUG_PRINT("In ast_variable_literal_or_binary_to_x86\n");
  if (node->type == AST_BINARY) {
    ast_binary_node_to_x86(node, list, mem, 1);
  } else if (node->type == AST_VARIABLE || node->type == AST_INT_LITERAL) {
    ast_variable_or_literal_node_to_x86(node, list, mem);
  } else if (node->type == AST_FUNCTION_CALL) {
    ast_function_call_node_to_x86(node, list, mem);
  } else {
    return;
  }
}

void ast_variable_or_literal_node_to_x86(ast_node* node,
                                         list_of_x86_instructions* list,
                                         memory* mem) {
  DEBUG_PRINT("In ast_variable_or_literal_node_to_x86\n");
  if (node->type == AST_INT_LITERAL) {
    DEBUG_PRINT("IS Int Literal");
    char* new_instruction = malloc(MAX_LINE_LENGTH);
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        mov     eax, %d",
                  node->as.int_literal.int_literal);
    add_instruction(list, new_instruction);
  } else if (node->type == AST_VARIABLE) {
    char* operand = get_variable_memory_location_with_pointer(
        mem, node->as.variable_name->lexeme, node->as.variable_name->length);

    char* new_instruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        mov     eax, DWORD PTR %s",
                  operand);
    free(operand);  // don't forget to free the operand string
    add_instruction(list, new_instruction);
  } else {
    fprintf(stderr, "ERROR: Unknown AST node type\n");
  }
}

void ast_binary_node_to_x86(ast_node* node, list_of_x86_instructions* list,
                            memory* mem, int first) {
  DEBUG_PRINT("ast_binary_node_to_x86");
  if (node->as.binary.right->type == AST_INT_LITERAL) {
    ast_node* right_node = node->as.binary.right;
    char* new_instruction = malloc(MAX_LINE_LENGTH);
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        mov     edx, %d",
                  right_node->as.int_literal.int_literal);
    add_instruction(list, new_instruction);
  } else if (node->as.binary.right->type == AST_VARIABLE) {
    ast_node* right_node = node->as.binary.right;
    char* operand = get_variable_memory_location_with_pointer(
        mem, right_node->as.variable_name->lexeme,
        right_node->as.variable_name->length);

    char* new_instruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        mov     edx, DWORD PTR %s",
                  operand);
    free(operand);  // don't forget to free the operand string
    add_instruction(list, new_instruction);
  } else {
    // Add in what else to do if not literal
    ast_binary_node_to_x86(node->as.binary.right, list, mem, 1);
  }
  if (node->as.binary.left->type == AST_INT_LITERAL) {
    ast_node* left_node = node->as.binary.left;
    char* new_instruction = malloc(MAX_LINE_LENGTH);
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        mov     eax, %d",
                  left_node->as.int_literal.int_literal);
    add_instruction(list, new_instruction);

  } else if (node->as.binary.left->type == AST_VARIABLE) {
    ast_node* left_node = node->as.binary.left;
    char* operand = get_variable_memory_location_with_pointer(
        mem, left_node->as.variable_name->lexeme,
        left_node->as.variable_name->length);

    char* new_instruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        mov     eax, DWORD PTR %s",
                  operand);
    free(operand);  // don't forget to free the operand string
    add_instruction(list, new_instruction);
  }

  if (first == 0) {
    char* new_instruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }

    (void)sprintf(new_instruction, "        %s     edx, eax",
                  get_op_name(node->as.binary._operator));

    add_instruction(list, new_instruction);
  } else {
    char* new_instruction =
        malloc(MAX_LINE_LENGTH);  // enough for full instruction line
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        %s     eax, edx",
                  get_op_name(node->as.binary._operator));

    add_instruction(list, new_instruction);
  }
}

void ast_variable_declaration_node_to_x86(ast_node* node, memory* mem) {
  char* variable_name =
      malloc((unsigned long)node->as.variable_declaration.name->length +
             (unsigned long)1);
  if (!variable_name) {
    error_and_exit("malloc failed");
    return;
  }
  strncpy(variable_name, node->as.variable_declaration.name->lexeme,
          (size_t)node->as.variable_declaration.name->length);
  variable_name[node->as.variable_declaration.name->length] = '\0';
  add_variable_to_memory(mem, variable_name);
}

void ast_declaration_node_to_x86(ast_node* node, list_of_x86_instructions* list,
                                 memory* mem) {
  DEBUG_PRINT("In ast_declaration_node_to_x86 function\n");
  char* variable_location_string = NULL;
  if (node->as.declaration.variable->type == AST_VARIABLE_DECLARATION) {
    ast_variable_declaration_node_to_x86(node->as.declaration.variable, mem);
    variable_location_string = get_variable_memory_location_with_pointer(
        mem,
        node->as.declaration.variable->as.variable_declaration.name->lexeme,
        node->as.declaration.variable->as.variable_declaration.name->length);
  } else if (node->as.declaration.variable->type == AST_VARIABLE) {
    variable_location_string = get_variable_memory_location_with_pointer(
        mem, node->as.declaration.variable->as.variable_name->lexeme,
        node->as.declaration.variable->as.variable_name->length);
  } else {
    error_and_exit("Error: Not a variable node\n");
  }

  ast_variable_literal_or_binary_to_x86(node->as.declaration.expression, list,
                                        mem);

  print_memory(mem);
  char* new_instruction =
      malloc(MAX_LINE_LENGTH);  // enough for full instruction line
  if (!new_instruction) {
    error_and_exit("malloc failed");
  }

  (void)sprintf(new_instruction, "        mov     DWORD PTR %s, eax",
                variable_location_string);
  free(variable_location_string);
  add_instruction(list, new_instruction);
}

void ast_return_node_to_x86(ast_node* node, list_of_x86_instructions* list,
                            memory* mem) {
  DEBUG_PRINT("In Return Node\n");

  ast_statement_node_to_x86(node->as._return.expression, list, mem);
  char* new_instruction = NULL;  //= malloc(MAX_LINE_LENGTH);

  new_instruction = "        pop     rbp";
  add_instruction(list, new_instruction);
  new_instruction = "        ret";
  add_instruction(list, new_instruction);
}

void ast_statement_node_to_x86(ast_node* node, list_of_x86_instructions* list,
                               memory* mem) {
  DEBUG_PRINT("In Statement Node\n");
  if (node == NULL) {
    DEBUG_PRINT("NULL NODE\n");
    return;
  }
  switch (node->type) {
    case AST_VARIABLE:
      DEBUG_PRINT("In Variable Node\n");
      ast_variable_or_literal_node_to_x86(node, list, mem);
      break;
    case AST_INT_LITERAL:
      DEBUG_PRINT("In Int Literal Node\n");
      ast_variable_or_literal_node_to_x86(node, list, mem);
      break;
    case AST_DECLARATION:
      DEBUG_PRINT("In Declaration Node\n");
      ast_declaration_node_to_x86(node, list, mem);
      break;
    case AST_VARIABLE_DECLARATION:
      DEBUG_PRINT("In Variable Declaration Node\n");
      ast_variable_declaration_node_to_x86(node, mem);
      break;
    case AST_FUNCTION_CALL:
      DEBUG_PRINT("In Function Call\n");
      ast_function_call_node_to_x86(node, list, mem);
      break;
    case AST_RETURN:
      DEBUG_PRINT("In Return Statement\n");
      ast_return_node_to_x86(node, list, mem);
      break;
    default:
      DEBUG_PRINT("In default case\n");
      break;
  }
}

void ast_block_node_to_x86(ast_node* node, list_of_x86_instructions* list,
                           memory* mem) {
  DEBUG_PRINT("In blocknode%d\n", node->as.block.count);
  for (int i = 0; i < node->as.block.count; i++) {
    DEBUG_PRINT("Blocknode: %d\n", i);

    ast_statement_node_to_x86(node->as.block.statements[i], list, mem);
  }
}

void ast_function_call_node_to_x86(ast_node* node,
                                   list_of_x86_instructions* list,
                                   memory* mem) {
  DEBUG_PRINT("In Function Call\n");
  for (int i = 0; i < node->as.function_call.param_count; i++) {
    DEBUG_PRINT("1\n");
    if (node->as.function_call.parameters[i]->type == AST_INT_LITERAL) {
      DEBUG_PRINT(
          "%d",
          node->as.function_call.parameters[i]->as.int_literal.int_literal);
    }
    ast_variable_literal_or_binary_to_x86(node->as.function_call.parameters[i],
                                          list, mem);
    DEBUG_PRINT("1\n");
    char* new_instruction = malloc(MAX_LINE_LENGTH);
    if (!new_instruction) {
      error_and_exit("malloc failed");
    }
    (void)sprintf(new_instruction, "        mov     %s, eax",
                  get_low_linux_registers_name(i));
    add_instruction(list, new_instruction);
  }
  DEBUG_PRINT("1\n");

  // Below is the function call
  char* new_instruction = malloc(MAX_LINE_LENGTH);

  (void)sprintf(new_instruction, "        call    %.*s",
                node->as.function_call.name->length,
                node->as.function_call.name->lexeme);

  //   for (int i = 0; i < node->as.function_call.param_count; i++) {
  //     // TODO: (PRIORITY)  Deal with variables properly by putting them in
  //     the
  //     // stack using edi esi
  //     char* paramType = "int";
  //     //
  //     node->as.function_call.parameters[i]->as.variable_declaration.type->lexeme;
  //     int length = 3;
  //     //
  //     node->as.function_call.parameters[i]->as.variable_declaration.type->length;

  //     int currentNewInstructionLength = strlen(new_instruction);
  //     for (int j = 0; j < length; j++) {
  //       new_instruction[currentNewInstructionLength + j] = paramType[j];
  //     }
  //     if (i + 1 < node->as.function_call.param_count) {
  //       new_instruction[currentNewInstructionLength + length] = ',';
  //       length++;
  //     }
  //     new_instruction[currentNewInstructionLength + length] = '\0';
  //   }
  //   int currentNewInstructionLength = strlen(new_instruction);

  //   new_instruction[currentNewInstructionLength] = ')';
  //   new_instruction[currentNewInstructionLength + 2] = '\0';
  add_instruction(list, new_instruction);
}

void ast_function_node_to_x86(ast_node* node, list_of_x86_instructions* list) {
  DEBUG_PRINT("In function node %.*s\n", node->as.function.name->length,
              node->as.function.name->lexeme);
  if (node->type != AST_FUNCTION_DECLARATION) {
    error_and_exit("Error: Not a function node\n");
  }
  memory* mem = malloc(sizeof(memory));
  init_memory(mem);
  if (strncmp(node->as.function.name->lexeme, "main", strlen("main")) == 0) {
    char* new_instruction = "main:";
    add_instruction(list, new_instruction);
  } else {
    char* new_instruction = malloc(MAX_LINE_LENGTH);

    if (!new_instruction) {
      error_and_exit("malloc failed");
    }

    (void)sprintf(new_instruction, "%.*s:", node->as.function.name->length,
                  node->as.function.name->lexeme);

    // for (int i = 0; i < node->as.function.param_count; i++) {
    //   // TODO: (PRIORITY)  Deal with variables properly by putting them in
    //   the
    //   // stack using edi esi
    //   char* paramType =
    //       node->as.function.parameters[i]->as.variable_declaration.type->lexeme;
    //   int length =
    //       node->as.function.parameters[i]->as.variable_declaration.type->length;

    //   int currentNewInstructionLength = strlen(new_instruction);
    //   for (int j = 0; j < length; j++) {
    //     new_instruction[currentNewInstructionLength + j] = paramType[j];
    //   }
    //   if (i + 1 < node->as.function.param_count) {
    //     new_instruction[currentNewInstructionLength + length] = ',';
    //     length++;
    //   }
    //   new_instruction[currentNewInstructionLength + length] = '\0';
    // }
    // int currentNewInstructionLength = strlen(new_instruction);

    // new_instruction[currentNewInstructionLength] = ')';
    // new_instruction[currentNewInstructionLength + 1] = ':';
    // new_instruction[currentNewInstructionLength + 2] = '\0';
    // // (void)sprintf(new_instruction, "global %s\n%s:",
    // node->as.function.name,
    // //         node->as.function.name);

    add_instruction(list, new_instruction);
  }
  char* new_instruction = "        push    rbp";
  add_instruction(list, new_instruction);
  new_instruction = "        mov     rbp, rsp";
  add_instruction(list, new_instruction);

  for (int i = 0; i < node->as.function.param_count; i++) {
    char* variable_name = malloc(
        node->as.function.parameters[i]->as.variable_declaration.name->length +
        1);
    if (!variable_name) {
      error_and_exit("malloc failed");
    }
    strncpy(
        variable_name,
        node->as.function.parameters[i]->as.variable_declaration.name->lexeme,
        node->as.function.parameters[i]->as.variable_declaration.name->length);
    variable_name[node->as.function.parameters[i]
                      ->as.variable_declaration.name->length] = '\0';
    add_variable_to_memory(mem, variable_name);
    char* var_loc_with_pointer = get_variable_memory_location_with_pointer(
        mem,
        node->as.function.parameters[i]->as.variable_declaration.name->lexeme,
        node->as.function.parameters[i]->as.variable_declaration.name->length);
    new_instruction = malloc(MAX_LINE_LENGTH);
    (void)sprintf(new_instruction, "        mov     DWORD PTR %s, %s",
                  var_loc_with_pointer, get_low_linux_registers_name(i));
    add_instruction(list, new_instruction);
  }

  ast_block_node_to_x86(node->as.function.statements, list, mem);
}

void list_of_ast_function_nodes_to_x86(ast_node** nodes,
                                       list_of_x86_instructions* list,
                                       int numberOfFunctions) {
  DEBUG_PRINT("Going through %d functions.\n", numberOfFunctions);
  char* new_instruction = ".intel_syntax noprefix";
  add_instruction(list, new_instruction);
  new_instruction = ".global _start";
  add_instruction(list, new_instruction);
  new_instruction = ".text";
  add_instruction(list, new_instruction);
  new_instruction = "_start:";
  add_instruction(list, new_instruction);
  new_instruction = "    call main";
  add_instruction(list, new_instruction);
  new_instruction = "    mov rdi, rax       # syscall: exit";
  add_instruction(list, new_instruction);
  new_instruction = "    mov rax, 60        # exit code 0";
  add_instruction(list, new_instruction);
  new_instruction = "    syscall";
  add_instruction(list, new_instruction);

  for (int i = 0; i < numberOfFunctions; ++i) {
    if (nodes[i] != NULL) {
      ast_function_node_to_x86(nodes[i], list);
    }
  }
}

#include <stdio.h>

void print_instructions(list_of_x86_instructions* list) {
  FILE* file = fopen("chat.s", "we");  // Overwrite/clear the file
  if (!file) {
    perror("Failed to open chat.s for writing");
    return;
  }

  // Add the instructions
  for (int i = 0; i < list->instruction_count; i++) {
    fprintf(file, "%s\n", list->instructions[i]);
  }

  (void)fclose(file);
}

void print_memory(memory* mem) {
  DEBUG_PRINT("Memory Layout (%d variable(s)):\n", mem->number_of_variables);
  for (int i = 0; i < mem->number_of_variables; i++) {
    printf("  %s -> [rbp-%d]\n", mem->variables[i]->variable_name,
           mem->variables[i]->memory_difference * -1);  // make offset positive
  }
}

/*


 char* variable_name = malloc(node->as.variable_declaration.name->length + 1);
  if (!variable_name) {
    error_and_exit("malloc failed");
  }
  strncpy(variable_name, node->as.variable_declaration.name->lexeme,
          node->as.variable_declaration.name->length);
  variable_name[node->as.variable_declaration.name->length] = '\0';
  add_variable_to_memory(mem, variable_name);

*/
