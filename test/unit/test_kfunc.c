#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "unity.h"
#include "kfunc.h"

void setUp(void) { }
void tearDown(void) { }

// Test Fisher's exact test with known values
// Using classic "lady tasting tea" example:
// contingency table where lady correctly identifies 3 of 4 cups
void test_fisher_exact_basic(void)
{
    long double left, right, two;

    // Classic 2x2 table:
    //        Tea first | Milk first
    // Guessed Tea    3 |     1      = 4
    // Guessed Milk   1 |     3      = 4
    //               ---+---
    //                4 |     4      = 8

    long double p = _kt_fisher_exact(3, 1, 1, 3, &left, &right, &two);

    // Two-tailed p-value should be approximately 0.4857
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.4857, (float)two);
}

// Test with perfect association (all in diagonal)
void test_fisher_exact_perfect_association(void)
{
    long double left, right, two;

    // Perfect positive association:
    // 10  0
    //  0 10
    long double p = _kt_fisher_exact(10, 0, 0, 10, &left, &right, &two);

    // Should have very small p-value (highly significant)
    TEST_ASSERT_TRUE(two < 0.001);
}

// Test with no association (equal distribution)
void test_fisher_exact_no_association(void)
{
    long double left, right, two;

    // No association:
    // 5 5
    // 5 5
    long double p = _kt_fisher_exact(5, 5, 5, 5, &left, &right, &two);

    // Should have p-value close to 1 (not significant)
    TEST_ASSERT_FLOAT_WITHIN(0.1, 1.0, (float)two);
}

// Test with zeros in the table
void test_fisher_exact_with_zeros(void)
{
    long double left, right, two;

    // Table with a zero:
    // 10 0
    // 5  5
    long double p = _kt_fisher_exact(10, 0, 5, 5, &left, &right, &two);

    // Should compute without error
    TEST_ASSERT_TRUE(two >= 0.0 && two <= 1.0);
}

// Test lbinom (log binomial coefficient)
void test_lbinom(void)
{
    // C(5,0) = 1, log(1) = 0
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0.0, (float)_lbinom(5, 0));

    // C(5,5) = 1, log(1) = 0
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0.0, (float)_lbinom(5, 5));

    // C(5,2) = 10, log(10) ~= 2.303
    TEST_ASSERT_FLOAT_WITHIN(0.01, 2.303, (float)_lbinom(5, 2));

    // C(10,5) = 252, log(252) ~= 5.529
    TEST_ASSERT_FLOAT_WITHIN(0.01, 5.529, (float)_lbinom(10, 5));
}

// Test hypergeometric distribution
void test_hypergeo(void)
{
    // Simple case: drawing from an urn
    // n11=2, n1_=4, n_1=4, n=8
    // P(X=2) when drawing 4 from urn with 4 white, 4 black
    long double p = _hypergeo(2, 4, 4, 8);

    // This should be C(4,2)*C(4,2)/C(8,4) = 36/70 ~= 0.514
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.514, (float)p);
}

// Test edge case: single element
void test_fisher_exact_single(void)
{
    long double left, right, two;

    // Minimal table:
    // 1 0
    // 0 1
    long double p = _kt_fisher_exact(1, 0, 0, 1, &left, &right, &two);

    // Should be significant (p = 0.5 for this tiny table)
    TEST_ASSERT_TRUE(two >= 0.0 && two <= 1.0);
}

// Test larger numbers
void test_fisher_exact_larger_numbers(void)
{
    long double left, right, two;

    // Larger table:
    // 100  50
    //  50 100
    long double p = _kt_fisher_exact(100, 50, 50, 100, &left, &right, &two);

    // Should compute without overflow and return valid p-value
    TEST_ASSERT_TRUE(two >= 0.0 && two <= 1.0);
    // This shows positive association, should be significant
    TEST_ASSERT_TRUE(two < 0.05);
}
