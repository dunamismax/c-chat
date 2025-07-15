#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include our math utilities library
#include "../../../../libs/math_utils/include/math_utils.h"

/**
 * Simple command-line calculator
 * Demonstrates: basic I/O, string parsing, arithmetic operations
 * Uses the math_utils library for extended operations
 *
 * SECURITY: Implements safe input parsing and validation
 */

#define MAX_INPUT_LENGTH 256
#define MAX_OP_LENGTH 32

/**
 * Safe input parsing with bounds checking
 */
static int safe_parse_input(const char *input, char *op_str, size_t op_size,
                            double *num1, double *num2) {
  if (!input || !op_str || !num1 || !num2 || op_size == 0) {
    return -1;
  }

  // Clear output buffers
  memset(op_str, 0, op_size);
  *num1 = 0.0;
  *num2 = 0.0;

  // Create a local copy to avoid modifying input
  char input_copy[MAX_INPUT_LENGTH];
  if (strlen(input) >= sizeof(input_copy)) {
    return -1; // Input too long
  }

  strcpy(input_copy, input);

  // Parse components safely
  char *token = strtok(input_copy, " \t\n");
  if (!token) {
    return 0; // No tokens
  }

  // Copy operation string with bounds checking
  if (strlen(token) >= op_size) {
    return -1; // Operation string too long
  }
  strcpy(op_str, token);

  // Parse first number if present
  token = strtok(NULL, " \t\n");
  if (token) {
    char *endptr;
    errno = 0;
    *num1 = strtod(token, &endptr);
    if (errno != 0 || *endptr != '\0') {
      return -1; // Invalid number
    }

    // Parse second number if present
    token = strtok(NULL, " \t\n");
    if (token) {
      errno = 0;
      *num2 = strtod(token, &endptr);
      if (errno != 0 || *endptr != '\0') {
        return -1; // Invalid number
      }
      return 3; // Operation + two numbers
    }
    return 2; // Operation + one number
  }

  return 1; // Operation only
}

/**
 * Validate operation string to prevent format string attacks
 */
static int validate_operation_string(const char *op_str) {
  if (!op_str) {
    return -1;
  }

  // Check for format string specifiers
  if (strchr(op_str, '%') != NULL) {
    return -1;
  }

  // Check for null bytes
  if (strlen(op_str) != strcspn(op_str, "\0")) {
    return -1;
  }

  // Check for control characters
  for (const char *p = op_str; *p; p++) {
    if (*p < 32 && *p != '\t') {
      return -1;
    }
  }

  return 0;
}

typedef enum {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_POW,
  OP_GCD,
  OP_LCM,
  OP_FACTORIAL,
  OP_FIBONACCI,
  OP_ISPRIME,
  OP_SQRT,
  OP_QUIT,
  OP_HELP,
  OP_INVALID
} Operation;

/**
 * Parse operation from string
 */
Operation parse_operation(const char *op) {
  if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0)
    return OP_ADD;
  if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0)
    return OP_SUB;
  if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0)
    return OP_MUL;
  if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0)
    return OP_DIV;
  if (strcmp(op, "%") == 0 || strcmp(op, "mod") == 0)
    return OP_MOD;
  if (strcmp(op, "^") == 0 || strcmp(op, "pow") == 0)
    return OP_POW;
  if (strcmp(op, "gcd") == 0)
    return OP_GCD;
  if (strcmp(op, "lcm") == 0)
    return OP_LCM;
  if (strcmp(op, "!") == 0 || strcmp(op, "fact") == 0)
    return OP_FACTORIAL;
  if (strcmp(op, "fib") == 0)
    return OP_FIBONACCI;
  if (strcmp(op, "prime") == 0)
    return OP_ISPRIME;
  if (strcmp(op, "sqrt") == 0)
    return OP_SQRT;
  if (strcmp(op, "quit") == 0 || strcmp(op, "exit") == 0)
    return OP_QUIT;
  if (strcmp(op, "help") == 0 || strcmp(op, "?") == 0)
    return OP_HELP;
  return OP_INVALID;
}

/**
 * Display help information
 */
void show_help() {
  printf("\n=== Calculator Help ===\n");
  printf("Basic Operations:\n");
  printf("  +, add      - Addition (a + b)\n");
  printf("  -, sub      - Subtraction (a - b)\n");
  printf("  *, mul      - Multiplication (a * b)\n");
  printf("  /, div      - Division (a / b)\n");
  printf("  %%, mod      - Modulo (a %% b)\n");
  printf("  ^, pow      - Power (a ^ b)\n");
  printf("\nAdvanced Operations:\n");
  printf("  gcd         - Greatest Common Divisor\n");
  printf("  lcm         - Least Common Multiple\n");
  printf("  !, fact     - Factorial (single number)\n");
  printf("  fib         - Fibonacci (single number)\n");
  printf("  prime       - Check if prime (single number)\n");
  printf("  sqrt        - Square root (single number)\n");
  printf("\nCommands:\n");
  printf("  help, ?     - Show this help\n");
  printf("  quit, exit  - Exit calculator\n");
  printf("\nUsage: <operation> <number1> [number2]\n");
  printf("Example: + 5 3\n");
  printf("Example: fact 5\n\n");
}

/**
 * Process calculator operation
 */
void process_operation(Operation op, double a, double b) {
  switch (op) {
  case OP_ADD:
    printf("Result: %.2f\n", a + b);
    break;
  case OP_SUB:
    printf("Result: %.2f\n", a - b);
    break;
  case OP_MUL:
    printf("Result: %.2f\n", a * b);
    break;
  case OP_DIV:
    if (b == 0) {
      printf("Error: Division by zero!\n");
    } else {
      printf("Result: %.2f\n", a / b);
    }
    break;
  case OP_MOD:
    if (b == 0) {
      printf("Error: Modulo by zero!\n");
    } else {
      printf("Result: %d\n", (int)a % (int)b);
    }
    break;
  case OP_POW:
    printf("Result: %lld\n", math_power((int)a, (int)b));
    break;
  case OP_GCD:
    printf("Result: %d\n", math_gcd((int)a, (int)b));
    break;
  case OP_LCM:
    printf("Result: %d\n", math_lcm((int)a, (int)b));
    break;
  case OP_FACTORIAL: {
    long long result = math_factorial((int)a);
    if (result == -1) {
      printf("Error: Factorial undefined for negative numbers!\n");
    } else {
      printf("Result: %lld\n", result);
    }
  } break;
  case OP_FIBONACCI: {
    long long result = math_fibonacci((int)a);
    if (result == -1) {
      printf("Error: Invalid input for Fibonacci!\n");
    } else {
      printf("Result: %lld\n", result);
    }
  } break;
  case OP_ISPRIME:
    if (math_is_prime((int)a)) {
      printf("Result: %d is prime\n", (int)a);
    } else {
      printf("Result: %d is not prime\n", (int)a);
    }
    break;
  case OP_SQRT: {
    double result = math_sqrt(a);
    if (result == -1) {
      printf("Error: Square root of negative number!\n");
    } else {
      printf("Result: %.6f\n", result);
    }
  } break;
  default:
    printf("Error: Unknown operation!\n");
    break;
  }
}

int main() {
  char input[MAX_INPUT_LENGTH];
  char op_str[MAX_OP_LENGTH];
  double num1, num2;

  printf("=== C Programming Calculator ===\n");
  printf("Type 'help' for instructions or 'quit' to exit\n\n");

  while (1) {
    printf("calc> ");

    // Read input line
    if (!fgets(input, sizeof(input), stdin)) {
      break;
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    // Skip empty lines
    if (strlen(input) == 0) {
      continue;
    }

    // Parse input safely
    int parsed = safe_parse_input(input, op_str, sizeof(op_str), &num1, &num2);

    if (parsed < 0) {
      printf("Error: Invalid input format. Type 'help' for instructions.\n");
      continue;
    }

    if (parsed < 1) {
      printf("Error: Invalid input. Type 'help' for instructions.\n");
      continue;
    }

    Operation op = parse_operation(op_str);

    if (op == OP_QUIT) {
      printf("Goodbye!\n");
      break;
    }

    if (op == OP_HELP) {
      show_help();
      continue;
    }

    if (op == OP_INVALID) {
      // Validate operation string before printing to prevent format string
      // attacks
      if (validate_operation_string(op_str) == 0) {
        printf("Error: Unknown operation. Type 'help' for list.\n");
      } else {
        printf("Error: Invalid operation format. Type 'help' for list.\n");
      }
      continue;
    }

    // Check if we have enough operands
    if ((op == OP_FACTORIAL || op == OP_FIBONACCI || op == OP_ISPRIME ||
         op == OP_SQRT) &&
        parsed < 2) {
      printf("Error: Missing operand. Single-operand operations require one "
             "number.\n");
      continue;
    }

    if ((op != OP_FACTORIAL && op != OP_FIBONACCI && op != OP_ISPRIME &&
         op != OP_SQRT) &&
        parsed < 3) {
      printf("Error: Missing operand. Two-operand operations require two "
             "numbers.\n");
      continue;
    }

    // Process the operation
    process_operation(op, num1, num2);
    printf("\n");
  }

  return 0;
}