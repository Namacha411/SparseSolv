#include "sparse-solv.h"
#include <cmath>
#include <complex>
#include <gtest/gtest.h>
#include <vector>

using dcomplex = std::complex<double>;

class SparseMatCTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Common setup for tests if needed
  }

  void TearDown() override {
    // Common cleanup for tests if needed
  }
};

TEST_F(SparseMatCTest, Creation) {
  auto matc = SparseSolv::createSparseMatC(5);
  ASSERT_NE(matc, nullptr);
}

TEST_F(SparseMatCTest, BasicOperations) {
  // Create 3x3 complex matrix
  auto matc = SparseSolv::createSparseMatC(3);

  // Add complex diagonal elements
  matc->add(0, 0, dcomplex(2.0, 1.0));
  matc->add(1, 1, dcomplex(3.0, -1.0));
  matc->add(2, 2, dcomplex(4.0, 0.5));

  // Add complex off-diagonal elements
  matc->add(0, 1, dcomplex(-1.0, 0.5));
  matc->add(1, 0, dcomplex(-1.0, -0.5));
  matc->add(1, 2, dcomplex(-1.5, 1.0));
  matc->add(2, 1, dcomplex(-1.5, -1.0));

  // Fix the matrix
  matc->fix();

  ASSERT_TRUE(matc->isFixed());
}

TEST_F(SparseMatCTest, ComplexIdentity) {
  // Create 3x3 complex identity matrix
  auto matc = SparseSolv::createSparseMatC(3);
  matc->add(0, 0, dcomplex(1.0, 0.0));
  matc->add(1, 1, dcomplex(1.0, 0.0));
  matc->add(2, 2, dcomplex(1.0, 0.0));
  matc->fix();

  // Test with complex vector
  std::vector<dcomplex> vec = {dcomplex(1.0, 2.0), dcomplex(3.0, -1.0),
                               dcomplex(-1.0, 1.5)};
  auto result = (*matc) * vec;

  ASSERT_EQ(dcomplex(1.0, 2.0), result[0]);
  ASSERT_EQ(dcomplex(3.0, -1.0), result[1]);
  ASSERT_EQ(dcomplex(-1.0, 1.5), result[2]);
}

TEST_F(SparseMatCTest, ImaginaryOperations) {
  // Create matrix with purely imaginary elements
  auto matc = SparseSolv::createSparseMatC(2);
  matc->add(0, 0, dcomplex(0.0, 2.0));  // 2i
  matc->add(0, 1, dcomplex(0.0, -1.0)); // -i
  matc->add(1, 0, dcomplex(0.0, 1.0));  // i
  matc->add(1, 1, dcomplex(0.0, 3.0));  // 3i
  matc->fix();

  // Test with real vector
  std::vector<dcomplex> vec = {dcomplex(1.0, 0.0), dcomplex(2.0, 0.0)};
  auto result = (*matc) * vec;

  // Expected: [2i*1 + (-i)*2, i*1 + 3i*2] = [0, 7i]
  ASSERT_EQ(dcomplex(0.0, 0.0), result[0]);
  ASSERT_EQ(dcomplex(0.0, 7.0), result[1]);
}

TEST_F(SparseMatCTest, ScalarMultiplication) {
  auto matc = SparseSolv::createSparseMatC(2);
  matc->add(0, 0, dcomplex(1.0, 1.0));
  matc->add(1, 1, dcomplex(2.0, -1.0));
  matc->fix();

  // Test complex scalar multiplication
  auto scaled_mat = (*matc) * dcomplex(2.0, 1.0);

  std::vector<dcomplex> vec = {dcomplex(1.0, 0.0), dcomplex(1.0, 0.0)};
  auto result = scaled_mat * vec;

  // (1+i)*(2+i) = 2+i+2i+i^2 = 2+3i-1 = 1+3i
  // (2-i)*(2+i) = 4+2i-2i-i^2 = 4+1 = 5
  ASSERT_EQ(dcomplex(1.0, 3.0), result[0]);
  ASSERT_EQ(dcomplex(5.0, 0.0), result[1]);
}

TEST_F(SparseMatCTest, Addition) {
  // Create first complex matrix
  auto matc1 = SparseSolv::createSparseMatC(2);
  matc1->add(0, 0, dcomplex(1.0, 1.0));
  matc1->add(1, 1, dcomplex(2.0, -1.0));
  matc1->fix();

  // Create second complex matrix
  auto matc2 = SparseSolv::createSparseMatC(2);
  matc2->add(0, 0, dcomplex(2.0, -1.0));
  matc2->add(1, 1, dcomplex(1.0, 1.0));
  matc2->fix();

  // Test addition
  auto sum_mat = (*matc1) + (*matc2);

  std::vector<dcomplex> vec = {dcomplex(1.0, 0.0), dcomplex(1.0, 0.0)};
  auto result = sum_mat * vec;

  // (1+i) + (2-i) = 3+0i = 3
  // (2-i) + (1+i) = 3+0i = 3
  ASSERT_EQ(dcomplex(3.0, 0.0), result[0]);
  ASSERT_EQ(dcomplex(3.0, 0.0), result[1]);
}

TEST_F(SparseMatCTest, MixedRealComplex) {
  // Create complex matrix
  auto matc = SparseSolv::createSparseMatC(3);
  matc->add(0, 0, dcomplex(2.0, 0.0)); // Real
  matc->add(1, 1, dcomplex(0.0, 3.0)); // Purely imaginary
  matc->add(2, 2, dcomplex(1.0, 1.0)); // Complex
  matc->fix();

  // Test with mixed vector
  std::vector<dcomplex> vec = {
      dcomplex(1.0, 0.0), // Real
      dcomplex(0.0, 1.0), // Purely imaginary
      dcomplex(1.0, 1.0)  // Complex
  };
  auto result = (*matc) * vec;

  ASSERT_EQ(dcomplex(2.0, 0.0), result[0]);  // 2*1 = 2
  ASSERT_EQ(dcomplex(-3.0, 0.0), result[1]); // 3i*i = -3
  ASSERT_EQ(dcomplex(0.0, 2.0), result[2]);  // (1+i)*(1+i) = 1+2i+i^2 = 2i
}

TEST_F(SparseMatCTest, HermitianMatrix) {
  // Create a simple 2x2 Hermitian matrix
  // [2    1+i]
  // [1-i  3  ]
  auto matc = SparseSolv::createSparseMatC(2);
  matc->add(0, 0, dcomplex(2.0, 0.0));
  matc->add(0, 1, dcomplex(1.0, 1.0));
  matc->add(1, 0, dcomplex(1.0, -1.0));
  matc->add(1, 1, dcomplex(3.0, 0.0));
  matc->fix();

  // Test with unit vector [1, 0]
  std::vector<dcomplex> vec1 = {dcomplex(1.0, 0.0), dcomplex(0.0, 0.0)};
  auto result1 = (*matc) * vec1;

  ASSERT_EQ(dcomplex(2.0, 0.0), result1[0]);
  ASSERT_EQ(dcomplex(1.0, -1.0), result1[1]);

  // Test with unit vector [0, 1]
  std::vector<dcomplex> vec2 = {dcomplex(0.0, 0.0), dcomplex(1.0, 0.0)};
  auto result2 = (*matc) * vec2;

  ASSERT_EQ(dcomplex(1.0, 1.0), result2[0]);
  ASSERT_EQ(dcomplex(3.0, 0.0), result2[1]);
}

TEST_F(SparseMatCTest, EmptyOperations) {
  auto matc = SparseSolv::createSparseMatC(5);
  ASSERT_FALSE(matc->isFixed());

  // Fix empty matrix
  matc->fix();
  ASSERT_TRUE(matc->isFixed());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}