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
  FILE *f1 = fopen(path1, "r"), *f2 = fopen(path2, "r");
  if (!f1 || !f2) return 0;

  int ch1, ch2;
  while ((ch1 = fgetc(f1)) != EOF && (ch2 = fgetc(f2)) != EOF)
    if (ch1 != ch2) {
      fclose(f1);
      fclose(f2);
      return 0;
    }

  int result = feof(f1) && feof(f2);
  fclose(f1);
  fclose(f2);
  return result;
}

static int run_and_get_exit(const char* command) {
  int status = system(command);
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  return -1;
}

Test(compiler, full_system_simple_return) {
  // 1. Copy test C file to test.txt (input to compiler)
  copy_file(CMAKE_SOURCE_DIR "/test/test_inputs/compiler_inputs/simple_return.c",
            "test.txt");

  // 2. Compile main.c as compiler, generate chat.s
  char command[256];
  snprintf(command, sizeof(command), "gcc %s/src/*.c -o compiler_main", CMAKE_SOURCE_DIR);
  int compile_compiler = system(command);
  cr_assert_eq(compile_compiler, 0, "Failed to compile compiler with: %s", command);
  
  int run_compiler = system("./compiler_main");
  cr_assert_eq(run_compiler, 0, "Compiler failed to run");

  cr_assert(access("chat.s", F_OK) == 0, "chat.s was not generated");

  // 3. Compare generated .s to expected
  cr_assert(files_equal("chat.s", "test/test_expected_outputs/simple_return.s"),
            "chat.s does not match expected output");

  // 4. Assemble, link, and run program
  cr_assert_eq(system("as -o abcd.o chat.s"), 0, "Assembly failed");
  cr_assert_eq(system("ld -o abcd abcd.o"), 0, "Linking failed");

  int ret_code = run_and_get_exit("./abcd");
  cr_expect_eq(ret_code, 42, "Expected return value of 42 from binary");
}
