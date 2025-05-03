#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void copy_file(const char* src_path, const char* dst_path) {
  FILE* src = fopen(src_path, "r");
  cr_assert_not_null(src, "Could not open source file: %s", src_path);
  FILE* dst = fopen(dst_path, "w");
  cr_assert_not_null(dst, "Could not open destination file: %s", dst_path);

  int c;
  while ((c = fgetc(src)) != EOF) fputc(c, dst);

  fclose(src);
  fclose(dst);
}

static int files_equal(const char* path1, const char* path2) {
  FILE* f1 = fopen(path1, "r");
  FILE* f2 = fopen(path2, "r");
  if (!f1 || !f2) {
    if (f1) fclose(f1);
    if (f2) fclose(f2);
    fprintf(stderr, "Could not open one of the files: %s or %s\n", path1,
            path2);
    return 0;
  }

  char line1[1024], line2[1024];
  int line_num = 1;

  while (fgets(line1, sizeof(line1), f1) && fgets(line2, sizeof(line2), f2)) {
    // Trim trailing whitespace
    int len1 = strlen(line1);
    while (len1 > 0 && (line1[len1 - 1] == ' ' || line1[len1 - 1] == '\t' ||
                        line1[len1 - 1] == '\n'))
      line1[--len1] = '\0';

    int len2 = strlen(line2);
    while (len2 > 0 && (line2[len2 - 1] == ' ' || line2[len2 - 1] == '\t' ||
                        line2[len2 - 1] == '\n'))
      line2[--len2] = '\0';

    if (strcmp(line1, line2) != 0) {
      fprintf(stderr,
              "Difference at line %d:\n"
              "  %s: \"%s\"\n"
              "  %s: \"%s\"\n",
              line_num, path1, line1, path2, line2);
      fclose(f1);
      fclose(f2);
      return 0;
    }

    line_num++;
  }

  // Check if either file still has lines
  int more1 = fgets(line1, sizeof(line1), f1) != NULL;
  int more2 = fgets(line2, sizeof(line2), f2) != NULL;

  if (more1 || more2) {
    fprintf(stderr, "Files differ in number of lines at line %d\n",
            line_num);
    fclose(f1);
    fclose(f2);
    return 0;
  }

  fclose(f1);
  fclose(f2);
  return 1;
}

static int run_and_get_exit(const char* command) {
  int status = system(command);
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  return -1;
}

Test(compiler, full_system_simple_return) {
  copy_file(CMAKE_SOURCE_DIR
            "/test/test_inputs/compiler_inputs/simple_return.c",
            "test.txt");

  char command[256];
  snprintf(command, sizeof(command), "gcc %s/src/*.c -o compiler_main",
           CMAKE_SOURCE_DIR);
  cr_assert_eq(system(command), 0, "Failed to compile compiler");

  cr_assert_eq(system("./compiler_main"), 0, "Compiler failed to run");

  cr_assert(access("chat.s", F_OK) == 0, "chat.s was not generated");

  char expected[512];
  snprintf(expected, sizeof(expected),
           "%s/test/test_expected_outputs/simple_return.s", CMAKE_SOURCE_DIR);
  cr_assert(access(expected, F_OK) == 0, "Expected file not found");

  cr_assert(files_equal("chat.s", expected),
            "chat.s does not match expected output");

  cr_assert_eq(system("as -o abcd.o chat.s"), 0, "Assembly failed");
  cr_assert_eq(system("ld -o abcd abcd.o"), 0, "Linking failed");

  int ret_code = run_and_get_exit("./abcd");
  cr_expect_eq(ret_code, 42, "Expected return value of 42 from binary");
}
