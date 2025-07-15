#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Include the test framework
#include "../test_framework.h"

// Include the math utils library
#include "../../libs/math_utils/include/math_utils.h"

// Test GCD function
int test_gcd(void) {
  if (math_gcd(48, 18) != 6)
    return 0;
  if (math_gcd(17, 13) != 1)
    return 0;
  if (math_gcd(100, 25) != 25)
    return 0;
  if (math_gcd(0, 5) != 5)
    return 0;
  if (math_gcd(5, 0) != 5)
    return 0;
  return 1;
}

// Test LCM function
int test_lcm(void) {
  if (math_lcm(4, 6) != 12)
    return 0;
  if (math_lcm(15, 20) != 60)
    return 0;
  if (math_lcm(7, 13) != 91)
    return 0;
  if (math_lcm(1, 5) != 5)
    return 0;
  return 1;
}

// Test prime checking
int test_is_prime(void) {
  if (math_is_prime(2) != 1)
    return 0;
  if (math_is_prime(3) != 1)
    return 0;
  if (math_is_prime(17) != 1)
    return 0;
  if (math_is_prime(97) != 1)
    return 0;
  if (math_is_prime(4) != 0)
    return 0;
  if (math_is_prime(9) != 0)
    return 0;
  if (math_is_prime(15) != 0)
    return 0;
  if (math_is_prime(1) != 0)
    return 0;
  if (math_is_prime(0) != 0)
    return 0;
  if (math_is_prime(-5) != 0)
    return 0;
  return 1;
}

// Test factorial function
int test_factorial(void) {
  if (math_factorial(0) != 1)
    return 0;
  if (math_factorial(1) != 1)
    return 0;
  if (math_factorial(5) != 120)
    return 0;
  if (math_factorial(6) != 720)
    return 0;
  if (math_factorial(-1) != -1)
    return 0; // Error case
  return 1;
}

// Test Fibonacci function
int test_fibonacci(void) {
  if (math_fibonacci(0) != 0)
    return 0;
  if (math_fibonacci(1) != 1)
    return 0;
  if (math_fibonacci(2) != 1)
    return 0;
  if (math_fibonacci(5) != 5)
    return 0;
  if (math_fibonacci(10) != 55)
    return 0;
  return 1;
}

// Test power function
int test_power(void) {
  if (math_power(2, 3) != 8)
    return 0;
  if (math_power(5, 2) != 25)
    return 0;
  if (math_power(10, 0) != 1)
    return 0;
  if (math_power(7, 1) != 7)
    return 0;
  if (math_power(-2, 2) != 4)
    return 0;
  if (math_power(-2, 3) != -8)
    return 0;
  return 1;
}

// Test square root function
int test_sqrt(void) {
  double result = math_sqrt(9.0);
  if (fabs(result - 3.0) > 0.001)
    return 0;

  result = math_sqrt(16.0);
  if (fabs(result - 4.0) > 0.001)
    return 0;

  result = math_sqrt(2.0);
  if (fabs(result - 1.414) > 0.01)
    return 0;

  result = math_sqrt(0.0);
  if (fabs(result - 0.0) > 0.001)
    return 0;

  return 1;
}

// Test absolute value function
int test_abs(void) {
  if (fabs(math_abs(5.0) - 5.0) > 0.001)
    return 0;
  if (fabs(math_abs(-5.0) - 5.0) > 0.001)
    return 0;
  if (fabs(math_abs(0.0) - 0.0) > 0.001)
    return 0;
  if (fabs(math_abs(-3.14) - 3.14) > 0.001)
    return 0;
  return 1;
}

// Test min and max functions
int test_min_max(void) {
  if (math_min(5, 3) != 3)
    return 0;
  if (math_min(-1, 1) != -1)
    return 0;
  if (math_min(10, 10) != 10)
    return 0;

  if (math_max(5, 3) != 5)
    return 0;
  if (math_max(-1, 1) != 1)
    return 0;
  if (math_max(10, 10) != 10)
    return 0;

  return 1;
}

int main(void) {
  printf("Math Utils Library Test Suite\n");
  printf("============================= \n\n");

  run_test(test_gcd, "test_gcd");
  run_test(test_lcm, "test_lcm");
  run_test(test_is_prime, "test_is_prime");
  run_test(test_factorial, "test_factorial");
  run_test(test_fibonacci, "test_fibonacci");
  run_test(test_power, "test_power");
  run_test(test_sqrt, "test_sqrt");
  run_test(test_abs, "test_abs");
  run_test(test_min_max, "test_min_max");

  return test_summary("Math Utils Library Test Suite");
}
