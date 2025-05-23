// NOLINTBEGIN(misc-include-cleaner)
// we checked to make sure only criterion related warnings were left
// NOLINTBEGIN(cert-env33-c, concurrency-mt-unsafe)
// these are just for the system calls
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void copy_file(const char* src_path, const char* dst_path) {
  FILE* src = fopen(src_path, "re");
  cr_assert_not_null(src, "Could not open source file: %s", src_path);
  FILE* dst = fopen(dst_path, "we");
  cr_assert_not_null(dst, "Could not open destination file: %s", dst_path);

  int chrc = 0;
  while ((chrc = fgetc(src)) != EOF) {
    (void)fputc(chrc, dst);
  }

  (void)fclose(src);
  (void)fclose(dst);
}

static int files_equal(const char* path1, const char* path2) {
  FILE* file1 = fopen(path1, "re");
  FILE* file2 = fopen(path2, "re");
  if (!file1 || !file2) {
    if (file1) {
      (void)fclose(file1);
    }
    if (file2) {
      (void)fclose(file2);
    }
    (void)fprintf(stderr, "Could not open one of the files: %s or %s\n", path1,
                  path2);
    return 0;
  }

  enum { MAX_LINE_LENGTH = 1024 };

  char line1[MAX_LINE_LENGTH];
  char line2[MAX_LINE_LENGTH];
  int line_num = 1;

  while (fgets(line1, sizeof(line1), file1) &&
         fgets(line2, sizeof(line2), file2)) {
    // Trim trailing whitespace
    size_t len1 = strlen(line1);
    while (len1 > 0 && (line1[len1 - 1] == ' ' || line1[len1 - 1] == '\t' ||
                        line1[len1 - 1] == '\n')) {
      line1[--len1] = '\0';
    }

    size_t len2 = strlen(line2);
    while (len2 > 0 && (line2[len2 - 1] == ' ' || line2[len2 - 1] == '\t' ||
                        line2[len2 - 1] == '\n')) {
      line2[--len2] = '\0';
    }

    if (strcmp(line1, line2) != 0) {
      (void)fprintf(stderr,
                    "Difference at line %d:\n"
                    "  %s: \"%s\"\n"
                    "  %s: \"%s\"\n",
                    line_num, path1, line1, path2, line2);
      (void)fclose(file1);
      (void)fclose(file2);
      return 0;
    }

    line_num++;
  }

  // Check if either file still has lines
  int more1 = fgets(line1, sizeof(line1), file1) != NULL;
  int more2 = fgets(line2, sizeof(line2), file2) != NULL;

  if (more1 || more2) {
    (void)fprintf(stderr, "Files differ in number of lines at line %d\n",
                  line_num);
    (void)fclose(file1);
    (void)fclose(file2);
    return 0;
  }

  (void)fclose(file1);
  (void)fclose(file2);
  return 1;
}

static int run_and_get_exit(const char* command) {
  int status = system(command);
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status);
  }
  return -1;
}

enum { COMMAND_BUFFER_SIZE = 256, FILE_BUFFER_SIZE = 512 };

// Test 1: Return 3
Test(compiler, full_system_simple_return) {
  copy_file(CMAKE_SOURCE_DIR
            "/test/test_inputs/compiler_inputs/simple_return.c",
            "test.txt");

  char command[COMMAND_BUFFER_SIZE];
  (void)snprintf(command, sizeof(command), "gcc %s/src/*.c -o compiler_main",
                 CMAKE_SOURCE_DIR);
  cr_assert_eq(system(command), 0, "Failed to compile compiler");

  cr_assert_eq(system("./compiler_main"), 0, "Compiler failed to run");

  cr_assert(access("chat.s", F_OK) == 0, "chat.s was not generated");

  char expected[FILE_BUFFER_SIZE];
  (void)snprintf(expected, sizeof(expected),
                 "%s/test/test_expected_outputs/simple_return.s",
                 CMAKE_SOURCE_DIR);
  cr_assert(access(expected, F_OK) == 0, "Expected file not found");

  cr_assert(files_equal("chat.s", expected),
            "chat.s does not match expected output");

  cr_assert_eq(system("as -o abcd.o chat.s"), 0, "Assembly failed");
  cr_assert_eq(system("ld -o abcd abcd.o"), 0, "Linking failed");

  int ret_code = run_and_get_exit("./abcd");
  cr_expect_eq(ret_code, 3, "Expected return value of 3 from binary");
}

// Test 2: full system binary add (6 + 2 = 8)
Test(compiler, full_system_binary_add) {
  copy_file(CMAKE_SOURCE_DIR "/test/test_inputs/compiler_inputs/binary_add.c",
            "test.txt");

  // rebuild the compiler
  char cmd[COMMAND_BUFFER_SIZE];
  (void)snprintf(cmd, sizeof(cmd), "gcc %s/src/*.c -o compiler_main",
                 CMAKE_SOURCE_DIR);
  cr_assert_eq(system(cmd), 0, "Failed to compile compiler");

  // run it
  cr_assert_eq(system("./compiler_main"), 0, "Compiler failed to run");
  cr_assert(access("chat.s", F_OK) == 0, "chat.s was not generated");

  // compare to expected
  char expected[FILE_BUFFER_SIZE];
  (void)snprintf(expected, sizeof(expected),
                 "%s/test/test_expected_outputs/binary_add.s",
                 CMAKE_SOURCE_DIR);
  cr_assert(access(expected, F_OK) == 0, "Expected file not found at %s",
            expected);
  cr_assert(files_equal("chat.s", expected),
            "chat.s does not match expected output at %s", expected);

  // assemble, link, run
  cr_assert_eq(system("as -o abcd.o chat.s"), 0, "Assembly failed");
  cr_assert_eq(system("ld -o abcd abcd.o"), 0, "Linking failed");
  int result = run_and_get_exit("./abcd");
  cr_expect_eq(result, 8, "Expected return value of 8 from binary");
}

// Test 3: full system multiplication (7 * 3 = 21)
Test(compiler, full_system_multiplication) {
  copy_file(CMAKE_SOURCE_DIR "/test/test_inputs/compiler_inputs/mul_return.c",
            "test.txt");

  char cmd[COMMAND_BUFFER_SIZE];
  (void)snprintf(cmd, sizeof(cmd), "gcc %s/src/*.c -o compiler_main",
                 CMAKE_SOURCE_DIR);
  cr_assert_eq(system(cmd), 0, "Failed to compile compiler");
  cr_assert_eq(system("./compiler_main"), 0, "Compiler failed to run");
  cr_assert(access("chat.s", F_OK) == 0, "chat.s was not generated");

  char expected[FILE_BUFFER_SIZE];
  (void)snprintf(expected, sizeof(expected),
                 "%s/test/test_expected_outputs/mul_return.s",
                 CMAKE_SOURCE_DIR);
  cr_assert(access(expected, F_OK) == 0, "Expected file not found at %s",
            expected);
  cr_assert(files_equal("chat.s", expected),
            "chat.s does not match expected output at %s", expected);

  cr_assert_eq(system("as -o abcd.o chat.s"), 0, "Assembly failed");
  cr_assert_eq(system("ld -o abcd abcd.o"), 0, "Linking failed");
  int result = run_and_get_exit("./abcd");
  cr_expect_eq(result, 21, "Expected return value of 21 from binary");
}

// Test 4: full system var-decl + return
Test(compiler, full_system_var_decl_return) {
  copy_file(CMAKE_SOURCE_DIR
            "/test/test_inputs/compiler_inputs/var_decl_return.c",
            "test.txt");

  char cmd[COMMAND_BUFFER_SIZE];
  (void)snprintf(cmd, sizeof(cmd), "gcc %s/src/*.c -o compiler_main",
                 CMAKE_SOURCE_DIR);
  cr_assert_eq(system(cmd), 0, "Failed to compile compiler");
  cr_assert_eq(system("./compiler_main"), 0, "Compiler failed to run");
  cr_assert(access("chat.s", F_OK) == 0, "chat.s was not generated");

  char expected[FILE_BUFFER_SIZE];
  (void)snprintf(expected, sizeof(expected),
                 "%s/test/test_expected_outputs/var_decl_return.s",
                 CMAKE_SOURCE_DIR);
  cr_assert(access(expected, F_OK) == 0, "Expected file not found at %s",
            expected);
  cr_assert(files_equal("chat.s", expected),
            "chat.s does not match expected output at %s", expected);

  cr_assert_eq(system("as -o abcd.o chat.s"), 0, "Assembly failed");
  cr_assert_eq(system("ld -o abcd abcd.o"), 0, "Linking failed");
  int result = run_and_get_exit("./abcd");
  cr_expect_eq(result, 5, "Expected return value of 5 from binary");
}

// Test 5: full system with one-parameter function call
Test(compiler, full_system_func_params) {
  // 1) copy source into test.txt
  copy_file(CMAKE_SOURCE_DIR
            "/test/test_inputs/compiler_inputs/func_params_call.c",
            "test.txt");

  // 2) rebuild & run compiler
  char cmd[COMMAND_BUFFER_SIZE];
  (void)snprintf(cmd, sizeof(cmd), "gcc %s/src/*.c -o compiler_main",
                 CMAKE_SOURCE_DIR);
  cr_assert_eq(system(cmd), 0, "Failed to compile compiler");
  cr_assert_eq(system("./compiler_main"), 0, "Compiler run failed");
  cr_assert(access("chat.s", F_OK) == 0, "chat.s not generated");

  // 3) compare chat.s against expected
  char expected[FILE_BUFFER_SIZE];
  (void)snprintf(expected, sizeof(expected),
                 "%s/test/test_expected_outputs/func_params_call.s",
                 CMAKE_SOURCE_DIR);
  cr_assert(access(expected, F_OK) == 0, "Missing expected at %s", expected);
  cr_assert(files_equal("chat.s", expected), "chat.s does not match %s",
            expected);

  // 4) assemble, link, run and check exit code == 5
  cr_assert_eq(system("as -o abcd.o chat.s"), 0, "as failed");
  cr_assert_eq(system("ld -o abcd abcd.o"), 0, "ld failed");
  int result = run_and_get_exit("./abcd");
  cr_expect_eq(result, 5, "Expected return 5 from binary");
}

// NOLINTEND(cert-env33-c, concurrency-mt-unsafe)
// NOLINTEND(misc-include-cleaner)
