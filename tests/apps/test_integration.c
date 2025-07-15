#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Include the test framework
#include "../test_framework.h"

/**
 * Integration tests for C Monorepo applications
 * Tests real application behavior with input/output validation
 */

#define TEST(name) run_test(name, #name)

// Global variable to store the correct build path
static char build_path[256] = "build/debug";

/**
 * Find the correct build directory
 */
void find_build_path() {
  const char *paths[] = {"build/release", "build/debug", "build/profile", NULL};

  for (int i = 0; paths[i] != NULL; i++) {
    char test_path[512];
    snprintf(test_path, sizeof(test_path), "%s/bin/calculator", paths[i]);
    if (access(test_path, X_OK) == 0) {
      strncpy(build_path, paths[i], sizeof(build_path) - 1);
      build_path[sizeof(build_path) - 1] = '\0';
      return;
    }
  }
}

/**
 * Run application with input and capture output
 */
int run_app_with_input(const char *app_path, const char *input, char *output,
                       size_t output_size) {
  int pipe_in[2], pipe_out[2];
  pid_t pid;

  if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
    return -1;
  }

  pid = fork();
  if (pid == -1) {
    return -1;
  }

  if (pid == 0) {
    // Child process
    close(pipe_in[1]);
    close(pipe_out[0]);

    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);
    dup2(pipe_out[1], STDERR_FILENO);

    execl(app_path, app_path, (char *)NULL);
    exit(1);
  } else {
    // Parent process
    close(pipe_in[0]);
    close(pipe_out[1]);

    // Send input
    if (input) {
      write(pipe_in[1], input, strlen(input));
    }
    close(pipe_in[1]);

    // Read output
    ssize_t bytes_read = read(pipe_out[0], output, output_size - 1);
    if (bytes_read > 0) {
      output[bytes_read] = '\0';
    } else {
      output[0] = '\0';
    }

    close(pipe_out[0]);

    int status;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
  }
}

// Test calculator basic operations
static int test_calculator_basic(void) {
  char output[1024];
  const char *input = "+ 5 3\nquit\n";
  char app_path[512];

  snprintf(app_path, sizeof(app_path), "%s/bin/calculator", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  return (result == 0 && strstr(output, "8.00") != NULL);
}

// Test calculator security (format string protection)
static int test_calculator_security(void) {
  char output[1024];
  const char *input = "%s %p %n\nquit\n";
  char app_path[512];

  snprintf(app_path, sizeof(app_path), "%s/bin/calculator", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  // Should not crash and should show error message
  return (result == 0 && strstr(output, "Error") != NULL);
}

// Test calculator factorial with overflow protection
static int test_calculator_factorial_overflow(void) {
  char output[1024];
  const char *input = "fact 25\nquit\n"; // Should trigger overflow protection

  char app_path[512];
  snprintf(app_path, sizeof(app_path), "%s/bin/calculator", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  return (result == 0 && strstr(output, "Error") != NULL);
}

// Test file_utils security (path traversal protection)
static int test_file_utils_security(void) {
  char output[1024];
  const char *input = "info ../../../etc/passwd\nquit\n";

  char app_path[512];
  snprintf(app_path, sizeof(app_path), "%s/bin/file_utils", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  // Should reject dangerous path
  return (result == 0 && strstr(output, "Invalid or dangerous path") != NULL);
}

// Test file_utils basic functionality
static int test_file_utils_basic(void) {
  char output[1024];

  // Create a test file
  FILE *test_file = fopen("test_file.txt", "w");
  if (!test_file)
    return 0;
  fprintf(test_file, "Hello World\nTest file\n");
  fclose(test_file);

  const char *input = "info test_file.txt\nquit\n";
  char app_path[512];
  snprintf(app_path, sizeof(app_path), "%s/bin/file_utils", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  // Clean up
  unlink("test_file.txt");

  return (result == 0 && strstr(output, "File Information") != NULL);
}

// Test text_processor security (buffer overflow protection)
static int test_text_processor_security(void) {
  char output[1024];

  // Create very long input to test buffer overflow protection
  char long_input[2048];
  memset(long_input, 'A', sizeof(long_input) - 20);
  strcpy(long_input + sizeof(long_input) - 20, "\nquit\n");

  char app_path[512];
  snprintf(app_path, sizeof(app_path), "%s/bin/text_processor", build_path);
  int result = run_app_with_input(app_path, long_input, output, sizeof(output));

  // Should not crash
  return (result == 0);
}

// Test text_processor basic functionality
static int test_text_processor_basic(void) {
  char output[1024];
  const char *input = "upper hello world\nquit\n";

  char app_path[512];
  snprintf(app_path, sizeof(app_path), "%s/bin/text_processor", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  return (result == 0 && strstr(output, "HELLO WORLD") != NULL);
}

// Test tic_tac_toe basic functionality
static int test_tic_tac_toe_basic(void) {
  char output[1024];
  const char *input = "3\n"; // Exit option

  char app_path[512];
  snprintf(app_path, sizeof(app_path), "%s/bin/tic_tac_toe", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  return (result == 0);
}

// Test number_guessing basic functionality
static int test_number_guessing_basic(void) {
  char output[1024];
  const char *input = "3\n"; // Exit option

  char app_path[512];
  snprintf(app_path, sizeof(app_path), "%s/bin/number_guessing", build_path);
  int result = run_app_with_input(app_path, input, output, sizeof(output));

  return (result == 0);
}

int main(void) {
  printf("Integration Test Suite\n");
  printf("======================\n\n");

  // Find the correct build path
  find_build_path();

  // Check if calculator exists
  char calc_path[512];
  snprintf(calc_path, sizeof(calc_path), "%s/bin/calculator", build_path);
  if (access(calc_path, X_OK) != 0) {
    printf(
        "Calculator not found or not executable. Build the project first.\n");
    return 1;
  }

  TEST(test_calculator_basic);
  TEST(test_calculator_security);
  TEST(test_calculator_factorial_overflow);
  TEST(test_file_utils_security);
  TEST(test_file_utils_basic);
  TEST(test_text_processor_security);
  TEST(test_text_processor_basic);
  TEST(test_tic_tac_toe_basic);
  TEST(test_number_guessing_basic);

  return test_summary("Integration Test Suite");
}