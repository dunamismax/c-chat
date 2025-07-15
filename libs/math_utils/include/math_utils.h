/**
 * @file math_utils.h
 * @brief Mathematical utility functions optimized for ARM64
 * @author dunamismax <dunamismax@tutamail.com>
 * @version 1.0.0
 * @date 2024
 *
 * High-performance mathematical algorithms and utilities
 * Optimized for Apple Silicon (ARM64) architecture
 */

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

/**
 * @brief Mathematical utility functions library
 *
 * Educational examples of numerical algorithms with ARM64 optimizations.
 * Includes number theory, sequences, and common mathematical operations.
 */

/**
 * Calculate the greatest common divisor using Euclidean algorithm
 * @param a First number
 * @param b Second number
 * @return GCD of a and b
 */
int math_gcd(int a, int b);

/**
 * Calculate the least common multiple
 * @param a First number
 * @param b Second number
 * @return LCM of a and b
 */
int math_lcm(int a, int b);

/**
 * Check if a number is prime
 * @param n Number to check
 * @return 1 if prime, 0 if not prime
 */
int math_is_prime(int n);

/**
 * Calculate factorial iteratively
 * @param n Number to calculate factorial for
 * @return n! or -1 if n < 0
 */
long long math_factorial(int n);

/**
 * Calculate nth Fibonacci number iteratively
 * @param n Position in Fibonacci sequence
 * @return nth Fibonacci number
 */
long long math_fibonacci(int n);

/**
 * Calculate power using fast exponentiation
 * @param base Base number
 * @param exp Exponent
 * @return base^exp
 */
long long math_power(int base, int exp);

/**
 * Find square root using Newton's method
 * @param n Number to find square root of
 * @return Approximate square root
 */
double math_sqrt(double n);

/**
 * Calculate absolute value
 * @param x Input number
 * @return Absolute value of x
 */
double math_abs(double x);

/**
 * Find minimum of two numbers
 * @param a First number
 * @param b Second number
 * @return Minimum of a and b
 */
int math_min(int a, int b);

/**
 * Find maximum of two numbers
 * @param a First number
 * @param b Second number
 * @return Maximum of a and b
 */
int math_max(int a, int b);

#endif /* MATH_UTILS_H */
